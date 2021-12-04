// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sqflite_plugin.h"

#include <app_common.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <filesystem>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "constants.h"
#include "database_manager.h"
#include "errors.h"
#include "log.h"
#include "log_level.h"

template <typename T>
bool GetValueFromEncodableMap(flutter::EncodableMap &map, std::string key,
                              T &out) {
  auto iter = map.find(flutter::EncodableValue(key));
  if (iter != map.end() && !iter->second.IsNull()) {
    if (auto pval = std::get_if<T>(&iter->second)) {
      out = *pval;
      return true;
    }
  }
  return false;
}

struct DBResultVisitor {
  flutter::EncodableValue operator()(int64_t value) {
    return flutter::EncodableValue(value);
  };
  flutter::EncodableValue operator()(std::string value) {
    return flutter::EncodableValue(value);
  };
  flutter::EncodableValue operator()(double value) {
    return flutter::EncodableValue(value);
  };
  flutter::EncodableValue operator()(std::vector<uint8_t> value) {
    return flutter::EncodableValue(value);
  };
  flutter::EncodableValue operator()(std::nullptr_t value) {
    return flutter::EncodableValue();
  };
};

class SqflitePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), sqflite_constants::kPluginKey,
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<SqflitePlugin>(registrar);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }
  SqflitePlugin(flutter::PluginRegistrar *registrar) : registrar_(registrar) {}

  virtual ~SqflitePlugin() {}

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("HandleMethodCall: %s", method_call.method_name().c_str());
    const std::string method_name = method_call.method_name();
    if (method_name == sqflite_constants::kMethodOpenDatabase) {
      OnOpenDatabaseCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodCloseDatabase) {
      OnCloseDatabaseCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodDeleteDatabase) {
      OnDeleteDatabase(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodGetDatabasesPath) {
      OnGetDatabasesPathCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodOptions) {
      OnOptionsCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodExecute) {
      OnExecuteCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodQuery) {
      OnQueryCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodInsert) {
      OnInsertCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodUpdate) {
      OnUpdateCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodBatch) {
      OnBatchCall(method_call, std::move(result));
    } else if (method_name == sqflite_constants::kMethodDebug) {
      OnDebugCall(method_call, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

 private:
  static bool IsInMemoryPath(std::string path) {
    return (path.empty() || path == sqflite_constants::kMemoryDatabasePath);
  }

  static int *GetDatabaseId(std::string path) {
    int *result = nullptr;
    auto itr = single_instances_by_path_.find(path);
    if (itr != single_instances_by_path_.end()) {
      result = &itr->second;
    }
    return result;
  }

  static std::shared_ptr<sqflite_database::DatabaseManager> GetDatabase(
      int database_id) {
    std::shared_ptr<sqflite_database::DatabaseManager> result = nullptr;
    auto itr = database_map_.find(database_id);
    if (itr != database_map_.end()) {
      result = itr->second;
    }
    return result;
  }

  bool IsDatabaseOpened(int database_id) {
    auto itr = database_map_.find(database_id);
    if (itr != database_map_.end()) {
      return itr->second->database() != nullptr;
    }
    return false;
  }

  static void HandleQueryException(
      sqflite_errors::DatabaseError exception, std::string sql,
      sqflite_database::SQLParameters sql_parameters,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap exception_map;
    exception_map.insert(
        std::pair<flutter::EncodableValue, flutter::EncodableValue>(
            flutter::EncodableValue(sqflite_constants::kParamSql),
            flutter::EncodableValue(sql)));
    exception_map.insert(
        std::pair<flutter::EncodableValue, flutter::EncodableList>(
            flutter::EncodableValue(sqflite_constants::kParamSqlArguments),
            sql_parameters));
    result->Error(sqflite_constants::kErrorDatabase, exception.what(),
                  flutter::EncodableValue(exception_map));
  }

  void OnDebugCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    std::string command;
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamCmd, command);

    flutter::EncodableMap map;

    std::lock_guard<std::mutex> lock(mutex_);
    if (command == sqflite_constants::kCmdGet) {
      if (log_level_ > sqflite_log_level::kNone) {
        map.insert(std::make_pair(
            flutter::EncodableValue(sqflite_constants::kParamLogLevel),
            flutter::EncodableValue(log_level_)));
      }
      if (database_map_.size() > 0) {
        flutter::EncodableMap databases_info;
        for (const auto &entry : database_map_) {
          auto [id, database] = entry;
          flutter::EncodableMap info;
          info.insert(std::make_pair(
              flutter::EncodableValue(sqflite_constants::kParamPath),
              flutter::EncodableValue(database->path())));
          info.insert(std::make_pair(
              flutter::EncodableValue(sqflite_constants::kParamSingleInstance),
              flutter::EncodableValue(database->single_instance())));
          if (database->log_level() > sqflite_log_level::kNone) {
            info.insert(std::make_pair(
                flutter::EncodableValue(sqflite_constants::kParamLogLevel),
                flutter::EncodableValue(database->log_level())));
          }
          databases_info.insert(
              std::make_pair(flutter::EncodableValue(id), info));
        }
        map.insert(std::make_pair(
            flutter::EncodableValue(sqflite_constants::kParamDatabases),
            databases_info));
      }
    }
    result->Success(flutter::EncodableValue(map));
  }

  void OnExecuteCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int database_id;
    std::string sql;
    sqflite_database::SQLParameters parameters;

    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSqlArguments,
                             parameters);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSql, sql);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamId,
                             database_id);

    std::lock_guard<std::mutex> lock(mutex_);
    auto database = GetDatabase(database_id);
    if (database == nullptr) {
      result->Error(sqflite_constants::kErrorDatabase,
                    sqflite_constants::kErrorDatabaseClosed + " " +
                        std::to_string(database_id));
      return;
    }
    try {
      Execute(database, sql, parameters);
    } catch (const sqflite_errors::DatabaseError &exception) {
      result->Error(sqflite_constants::kErrorDatabase, exception.what());
      return;
    }
    result->Success();
  }

  void Execute(std::shared_ptr<sqflite_database::DatabaseManager> database,
               std::string sql, sqflite_database::SQLParameters parameters) {
    database->Execute(sql, parameters);
  }

  int64_t QueryUpdateChanges(
      std::shared_ptr<sqflite_database::DatabaseManager> database) {
    std::string changes_sql = "SELECT changes();";
    auto [_, resultset] = database->Query(changes_sql);

    auto first_result = resultset[0];
    return std::get<int64_t>(first_result[0]);
  }

  std::pair<int64_t, int64_t> QueryInsertChanges(
      std::shared_ptr<sqflite_database::DatabaseManager> database) {
    std::string changes_sql = "SELECT changes(), last_insert_rowid();";

    auto [_, resultset] = database->Query(changes_sql);
    auto first_result = resultset[0];
    auto changes = std::get<int64_t>(first_result[0]);
    int last_id = 0;
    if (changes > 0) {
      last_id = std::get<int64_t>(first_result[1]);
    }
    return std::make_pair(changes, last_id);
  }

  flutter::EncodableValue Update(
      std::shared_ptr<sqflite_database::DatabaseManager> database,
      std::string sql, sqflite_database::SQLParameters parameters,
      bool no_result) {
    database->Execute(sql, parameters);
    if (no_result) {
      return flutter::EncodableValue();
    }

    auto changes = QueryUpdateChanges(database);
    if (changes > 0 && sqflite_log_level::HasSqlLevel(database->log_level())) {
      LOG_DEBUG("Number of rows changed: %d", changes);
    }
    return flutter::EncodableValue(changes);
  }

  flutter::EncodableValue Insert(
      std::shared_ptr<sqflite_database::DatabaseManager> database,
      std::string sql, sqflite_database::SQLParameters parameters,
      bool no_result) {
    database->Execute(sql, parameters);
    if (no_result) {
      return flutter::EncodableValue();
    }

    auto [changes, last_id] = QueryInsertChanges(database);

    if (changes == 0) {
      if (sqflite_log_level::HasSqlLevel(database->log_level())) {
        LOG_DEBUG("No changes (id was %d)", last_id);
      }
      return flutter::EncodableValue();
    }
    if (sqflite_log_level::HasSqlLevel(database->log_level())) {
      LOG_DEBUG("Inserted id: %d", last_id);
    }
    return flutter::EncodableValue(last_id);
  }

  flutter::EncodableValue Query(
      std::shared_ptr<sqflite_database::DatabaseManager> database,
      std::string sql, sqflite_database::SQLParameters parameters) {
    auto db_result_visitor = DBResultVisitor{};
    auto [columns, resultset] = database->Query(sql, parameters);
    if (query_as_map_list_) {
      flutter::EncodableList response;
      if (resultset.size() == 0) {
        return flutter::EncodableValue(response);
      }
      for (const auto &row : resultset) {
        flutter::EncodableMap row_map;
        for (size_t i = 0; i < row.size(); i++) {
          auto row_value = std::visit(db_result_visitor, row[i]);
          row_map.insert(
              std::pair<flutter::EncodableValue, flutter::EncodableValue>(
                  flutter::EncodableValue(columns[i]), row_value));
        }
        response.push_back(flutter::EncodableValue(row_map));
      }
      return flutter::EncodableValue(response);
    } else {
      flutter::EncodableMap response;
      if (resultset.size() == 0) {
        return flutter::EncodableValue(response);
      }
      flutter::EncodableList columns_response;
      flutter::EncodableList rows_response;
      for (const auto &column : columns) {
        columns_response.push_back(flutter::EncodableValue(column));
      }
      for (const auto &row : resultset) {
        flutter::EncodableList row_list;
        for (const auto &column : row) {
          auto row_value = std::visit(db_result_visitor, column);
          row_list.push_back(row_value);
        }
        rows_response.push_back(flutter::EncodableValue(row_list));
      }
      response.insert(
          std::pair<flutter::EncodableValue, flutter::EncodableValue>(
              flutter::EncodableValue(sqflite_constants::kParamColumns),
              flutter::EncodableValue(columns_response)));
      response.insert(
          std::pair<flutter::EncodableValue, flutter::EncodableValue>(
              flutter::EncodableValue(sqflite_constants::kParamRows),
              flutter::EncodableValue(rows_response)));
      return flutter::EncodableValue(response);
    }
  }

  void OnInsertCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int database_id;
    std::string sql;
    sqflite_database::SQLParameters parameters;
    bool no_result = false;

    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSqlArguments,
                             parameters);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSql, sql);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamId,
                             database_id);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamNoResult,
                             no_result);

    std::lock_guard<std::mutex> lock(mutex_);
    auto database = GetDatabase(database_id);
    if (database == nullptr) {
      result->Error(sqflite_constants::kErrorDatabase,
                    sqflite_constants::kErrorDatabaseClosed + " " +
                        std::to_string(database_id));
      return;
    }
    flutter::EncodableValue response;
    try {
      response = Insert(database, sql, parameters, no_result);
    } catch (const sqflite_errors::DatabaseError &exception) {
      HandleQueryException(exception, sql, parameters, std::move(result));
      return;
    }
    result->Success(response);
  }

  void OnUpdateCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int database_id;
    std::string sql;
    sqflite_database::SQLParameters parameters;
    bool no_result = false;

    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSqlArguments,
                             parameters);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSql, sql);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamId,
                             database_id);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamNoResult,
                             no_result);

    std::lock_guard<std::mutex> lock(mutex_);
    auto database = GetDatabase(database_id);
    if (database == nullptr) {
      result->Error(sqflite_constants::kErrorDatabase,
                    sqflite_constants::kErrorDatabaseClosed + " " +
                        std::to_string(database_id));
      return;
    }
    flutter::EncodableValue response;
    try {
      response = Update(database, sql, parameters, no_result);
    } catch (const sqflite_errors::DatabaseError &exception) {
      HandleQueryException(exception, sql, parameters, std::move(result));
      return;
    }
    result->Success(response);
  }

  void OnOptionsCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    bool parameters_as_list = false;
    int log_level = log_level_;

    GetValueFromEncodableMap(arguments, sqflite_constants::kParamQueryAsMapList,
                             parameters_as_list);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamLogLevel,
                             log_level);

    query_as_map_list_ = parameters_as_list;
    log_level_ = log_level;
    // TODO: Implement Thread Priority usage
    result->Success();
  }

  void OnQueryCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int database_id;
    std::string sql;
    sqflite_database::SQLParameters parameters;
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSqlArguments,
                             parameters);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSql, sql);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamId,
                             database_id);

    std::lock_guard<std::mutex> lock(mutex_);
    auto database = GetDatabase(database_id);
    if (database == nullptr) {
      result->Error(sqflite_constants::kErrorDatabase,
                    sqflite_constants::kErrorDatabaseClosed + " " +
                        std::to_string(database_id));
      return;
    }
    flutter::EncodableValue response;
    try {
      response = Query(database, sql, parameters);
    } catch (const sqflite_errors::DatabaseError &exception) {
      HandleQueryException(exception, sql, parameters, std::move(result));
      return;
    }
    result->Success(response);
  }

  void OnGetDatabasesPathCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    auto path = app_get_data_path();
    if (path == nullptr) {
      result->Error("storage_error", "not enough space to get data directory");
      return;
    }
    databases_path_ = path;
    free(path);

    result->Success(flutter::EncodableValue(databases_path_));
  }

  void OnDeleteDatabase(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    std::string path;
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamPath, path);

    std::lock_guard<std::mutex> lock(mutex_);
    auto existing_database_id = GetDatabaseId(path);
    if (existing_database_id) {
      if (IsDatabaseOpened(*existing_database_id)) {
        database_map_.erase(*existing_database_id);
        single_instances_by_path_.erase(path);
        if (sqflite_log_level::HasVerboseLevel(log_level_)) {
          LOG_DEBUG("Deleting database in path %s", path.c_str());
        }
      }
    }
    // TODO: Safe check before delete.
    std::filesystem::remove(path);
    result->Success();
  }

  flutter::EncodableValue MakeOpenResult(int database_id, bool recovered,
                                         bool recovered_in_transaction) {
    flutter::EncodableMap response;
    response.insert(
        std::make_pair(flutter::EncodableValue(sqflite_constants::kParamId),
                       flutter::EncodableValue(database_id)));
    if (recovered) {
      response.insert(std::make_pair(
          flutter::EncodableValue(sqflite_constants::kParamRecovered),
          flutter::EncodableValue(true)));
    }
    if (recovered_in_transaction) {
      response.insert(
          std::make_pair(flutter::EncodableValue(
                             sqflite_constants::kParamRecoveredInTransaction),
                         flutter::EncodableValue(true)));
    }
    return flutter::EncodableValue(response);
  }

  void OnOpenDatabaseCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    std::string path;
    bool read_only = false;
    bool single_instance = false;

    GetValueFromEncodableMap(arguments, sqflite_constants::kParamPath, path);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamReadOnly,
                             read_only);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamSingleInstance,
                             single_instance);

    const bool in_memory = IsInMemoryPath(path);
    single_instance = single_instance && !in_memory;

    std::lock_guard<std::mutex> lock(mutex_);
    if (single_instance) {
      if (sqflite_log_level::HasVerboseLevel(log_level_)) {
        std::string paths_in_map = "";
        for (const auto &pair : single_instances_by_path_) {
          if (paths_in_map.empty()) {
            paths_in_map = pair.first;
          } else {
            paths_in_map += "," + pair.first;
          }
        }
        LOG_DEBUG("Look for path %s in %s", path.c_str(), paths_in_map.c_str());
      }
      auto found_database_id = GetDatabaseId(path);
      if (found_database_id) {
        auto database_manager = GetDatabase(*found_database_id);
        if (database_manager->database()) {
          if (sqflite_log_level::HasVerboseLevel(log_level_)) {
            LOG_DEBUG("Re-opened single instance %d %s", *found_database_id,
                      path.c_str());
          }
          auto response = MakeOpenResult(*found_database_id, true, false);
          result->Success(response);
          return;
        }
      }
    }
    const int new_database_id = ++database_id_;
    try {
      std::shared_ptr<sqflite_database::DatabaseManager> database_manager =
          std::make_shared<sqflite_database::DatabaseManager>(
              path, new_database_id, single_instance, log_level_);
      if (!read_only) {
        database_manager->Open();
      } else {
        database_manager->OpenReadOnly();
      }

      // Store dbid in internal map
      if (single_instance) {
        single_instances_by_path_.insert(std::make_pair(path, new_database_id));
      }
      database_map_.insert(std::make_pair(new_database_id, database_manager));
    } catch (const sqflite_errors::DatabaseError &exception) {
      result->Error(sqflite_constants::kErrorDatabase,
                    sqflite_constants::kErrorOpenFailed + " " + path);
      return;
    }

    if (sqflite_log_level::HasSqlLevel(log_level_)) {
      LOG_DEBUG("Database opened %d in path %s", new_database_id, path.c_str());
    }

    auto response = MakeOpenResult(new_database_id, false, false);
    result->Success(response);
  }

  void OnCloseDatabaseCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int database_id;
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamId,
                             database_id);

    std::lock_guard<std::mutex> lock(mutex_);
    auto database = GetDatabase(database_id);
    if (database == nullptr) {
      result->Error(sqflite_constants::kErrorDatabase,
                    sqflite_constants::kErrorDatabaseClosed + " " +
                        std::to_string(database_id));
      return;
    }

    auto path = database->path();

    try {
      // By erasing the entry from databaseMap, the destructor of
      // database::DatabaseManager is called, which finalizes all open
      // statements and closes the database.
      if (sqflite_log_level::HasSqlLevel(database->log_level())) {
        LOG_DEBUG("Closing database %d %s", database->database_id(),
                  database->path().c_str());
      }
      database_map_.erase(database_id);

      if (database->single_instance()) {
        single_instances_by_path_.erase(path);
      }
    } catch (const sqflite_errors::DatabaseError &exception) {
      LOG_ERROR("Error while closing database %d: %s", database_id,
                exception.what());
      result->Error(sqflite_constants::kErrorDatabase, exception.what());
      return;
    }

    result->Success();
  };

  flutter::EncodableValue BuildSuccessBatchOperationResult(
      flutter::EncodableValue result) {
    flutter::EncodableMap operation_result;
    operation_result.insert(std::make_pair(
        flutter::EncodableValue(sqflite_constants::kParamResult), result));
    return flutter::EncodableValue(operation_result);
  }

  flutter::EncodableValue BuildErrorBatchOperationResult(
      const sqflite_errors::DatabaseError &exception, std::string sql,
      sqflite_database::SQLParameters parameters) {
    flutter::EncodableMap operation_result;
    flutter::EncodableMap operation_error_detail_result;
    flutter::EncodableMap operation_error_detail_data;
    operation_error_detail_result.insert(std::make_pair(
        flutter::EncodableValue(sqflite_constants::kParamErrorCode),
        flutter::EncodableValue(sqflite_constants::kErrorDatabase)));
    operation_error_detail_result.insert(std::make_pair(
        flutter::EncodableValue(sqflite_constants::kParamErrorMessage),
        flutter::EncodableValue(exception.what())));
    operation_error_detail_data.insert(
        std::make_pair(flutter::EncodableValue(sqflite_constants::kParamSql),
                       flutter::EncodableValue(sql)));
    operation_error_detail_data.insert(std::make_pair(
        flutter::EncodableValue(sqflite_constants::kParamSqlArguments),
        flutter::EncodableValue(parameters)));
    operation_error_detail_result.insert(std::make_pair(
        flutter::EncodableValue(sqflite_constants::kParamErrorData),
        flutter::EncodableValue(operation_error_detail_data)));
    operation_result.insert(
        std::make_pair(flutter::EncodableValue(sqflite_constants::kParamError),
                       operation_error_detail_result));
    return flutter::EncodableValue(operation_result);
  }

  void OnBatchCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int database_id;
    bool continue_on_error = false;
    bool no_result = false;
    flutter::EncodableList operations;
    flutter::EncodableList results;
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamId,
                             database_id);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamOperations,
                             operations);
    GetValueFromEncodableMap(
        arguments, sqflite_constants::kParamContinueOnError, continue_on_error);
    GetValueFromEncodableMap(arguments, sqflite_constants::kParamNoResult,
                             no_result);

    std::lock_guard<std::mutex> lock(mutex_);
    auto database = GetDatabase(database_id);
    if (database == nullptr) {
      result->Error(sqflite_constants::kErrorDatabase,
                    sqflite_constants::kErrorDatabaseClosed + " " +
                        std::to_string(database_id));
      return;
    }

    for (const auto &item : operations) {
      auto item_map = std::get<flutter::EncodableMap>(item);
      std::string method;
      std::string sql;
      sqflite_database::SQLParameters parameters;
      GetValueFromEncodableMap(item_map, sqflite_constants::kParamMethod,
                               method);
      GetValueFromEncodableMap(item_map, sqflite_constants::kParamSqlArguments,
                               parameters);
      GetValueFromEncodableMap(item_map, sqflite_constants::kParamSql, sql);

      if (method == sqflite_constants::kMethodExecute) {
        try {
          Execute(database, sql, parameters);
          if (!no_result) {
            auto operation_result =
                BuildSuccessBatchOperationResult(flutter::EncodableValue());
            results.push_back(operation_result);
          }
        } catch (const sqflite_errors::DatabaseError &exception) {
          if (!continue_on_error) {
            HandleQueryException(exception, sql, parameters, std::move(result));
            return;
          } else {
            if (!no_result) {
              auto operation_result =
                  BuildErrorBatchOperationResult(exception, sql, parameters);
              results.push_back(operation_result);
            }
          }
        }
      } else if (method == sqflite_constants::kMethodInsert) {
        try {
          auto response = Insert(database, sql, parameters, no_result);
          if (!no_result) {
            auto operation_result = BuildSuccessBatchOperationResult(response);
            results.push_back(operation_result);
          }
        } catch (const sqflite_errors::DatabaseError &exception) {
          if (!continue_on_error) {
            HandleQueryException(exception, sql, parameters, std::move(result));
            return;
          } else {
            if (!no_result) {
              auto operation_result =
                  BuildErrorBatchOperationResult(exception, sql, parameters);
              results.push_back(operation_result);
            }
          }
        }
      } else if (method == sqflite_constants::kMethodQuery) {
        try {
          auto response = Query(database, sql, parameters);
          if (!no_result) {
            auto operation_result = BuildSuccessBatchOperationResult(response);
            results.push_back(operation_result);
          }
        } catch (const sqflite_errors::DatabaseError &exception) {
          if (!continue_on_error) {
            HandleQueryException(exception, sql, parameters, std::move(result));
            return;
          } else {
            if (!no_result) {
              auto operation_result =
                  BuildErrorBatchOperationResult(exception, sql, parameters);
              results.push_back(operation_result);
            }
          }
        }
      } else if (method == sqflite_constants::kMethodUpdate) {
        try {
          auto response = Update(database, sql, parameters, no_result);
          if (!no_result) {
            auto operation_result = BuildSuccessBatchOperationResult(response);
            results.push_back(operation_result);
          }
        } catch (const sqflite_errors::DatabaseError &exception) {
          if (!continue_on_error) {
            HandleQueryException(exception, sql, parameters, std::move(result));
            return;
          } else {
            if (!no_result) {
              auto operation_result =
                  BuildErrorBatchOperationResult(exception, sql, parameters);
              results.push_back(operation_result);
            }
          }
        }
      } else {
        result->NotImplemented();
        return;
      }
    }
    if (no_result) {
      result->Success();
    } else {
      result->Success(flutter::EncodableValue(results));
    }
  }

  flutter::PluginRegistrar *registrar_;
  inline static std::mutex mutex_;
  inline static std::map<std::string, int> single_instances_by_path_;
  inline static std::map<int,
                         std::shared_ptr<sqflite_database::DatabaseManager>>
      database_map_;
  inline static std::string databases_path_;
  inline static bool query_as_map_list_ = false;
  inline static int database_id_ = 0;  // incremental database id
  inline static int log_level_ = sqflite_log_level::kNone;
};

void SqflitePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SqflitePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
