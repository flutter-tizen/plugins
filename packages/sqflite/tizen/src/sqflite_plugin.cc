// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <storage.h>
#ifndef TV_PROFILE
#include <privacy_privilege_manager.h>
#endif

#include <app_common.h>
#include <dlog.h>

#include <filesystem>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "database_manager.h"
#include "list"
#include "log.h"
#include "permission_manager.h"
#include "setting.h"
#include "sqflite_plugin.h"

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

class SqflitePlugin : public flutter::Plugin {
  inline static std::map<std::string, int> singleInstancesByPath;
  inline static std::map<int, std::shared_ptr<DatabaseManager>> databaseMap;
  inline static std::string databasesPath;
  inline static int internalStorageId;
  inline static bool queryAsMapList = false;
  inline static int databaseId = 0;  // incremental database id
  inline static int logLevel = DLOG_UNKNOWN;

 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "com.tekartik.sqflite",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<SqflitePlugin>(registrar);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }
  SqflitePlugin(flutter::PluginRegistrar *registrar)
      : registrar_(registrar), pmm_(std::make_unique<PermissionManager>()) {}

  virtual ~SqflitePlugin() {}

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("HandleMethodCall: %s", method_call.method_name().c_str());
    // CheckPermissions();
    const std::string methodName = method_call.method_name();
    if (methodName == "openDatabase") {
      OnOpenDatabaseCall(method_call, std::move(result));
    } else if (methodName == "closeDatabase") {
      OnCloseDatabaseCall(method_call, std::move(result));
    } else if (methodName == "deleteDatabase") {
      OnDeleteDatabase(method_call, std::move(result));
    } else if (methodName == "getDatabasesPath") {
      OnGetDatabasesPathCall(method_call, std::move(result));
    } else if (methodName == "options") {
      OnOptionsCall(method_call, std::move(result));
    } else if (methodName == "execute") {
      OnExecuteCall(method_call, std::move(result));
    } else if (methodName == "query") {
      OnQueryCall(method_call, std::move(result));
    } else if (methodName == "insert") {
      OnInsertCall(method_call, std::move(result));
    } else if (methodName == "update") {
      OnUpdateCall(method_call, std::move(result));
    } else if (methodName == "batch") {
      OnBatchCall(method_call, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void CheckPermissions(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
#ifndef TV_PROFILE
    const char *privilege = "http://tizen.org/privilege/mediastorage";

    ppm_check_result_e permission;
    int ret = ppm_check_permission(privilege, &permission);
    if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      LOG_ERROR("ppm_check_permission fail! [%d]", ret);
    } else {
      switch (permission) {
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
          LOG_INFO("ppm_check_permission success! [%d]", (int)permission);
          return;
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
          ret = ppm_request_permission(privilege, AppRequestResponseCb, this);
          if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
            LOG_ERROR("ppm_request_permission fail! [%d]", ret);
            break;
          }
          return;
        default:
          LOG_ERROR("ppm_check_permission deny! [%d]", (int)permission);
          break;
      }
    }
    result->Error("Invalid permission");
#else
#endif
  }

#ifndef TV_PROFILE
  static void AppRequestResponseCb(ppm_call_cause_e cause,
                                   ppm_request_result_e result,
                                   const char *privilege, void *data) {
    SqflitePlugin *plugin = (SqflitePlugin *)data;
    assert(plugin);

    if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
      LOG_ERROR("app_request_response_cb failed! [%d]", result);
      // plugin->SendResultWithError("Invalid permission");
      return;
    }
  }
#endif

  static bool isInMemoryPath(std::string path) {
    return (path.empty() || path == ":memory:");
  }

 private:
  static int *getDatabaseId(std::string path) {
    int *result = nullptr;
    auto itr = singleInstancesByPath.find(path);
    if (itr != singleInstancesByPath.end()) {
      result = &itr->second;
    }
    return result;
  }
  static std::shared_ptr<DatabaseManager> getDatabase(int databaseId) {
    std::shared_ptr<DatabaseManager> result = nullptr;
    auto itr = databaseMap.find(databaseId);
    if (itr != databaseMap.end()) {
      result = itr->second;
    }
    return result;
  }

  static std::shared_ptr<DatabaseManager> getDatabaseOrError(
      const flutter::MethodCall<flutter::EncodableValue> &method_call) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    GetValueFromEncodableMap(arguments, "id", databaseId);
    return getDatabase(databaseId);
  }

  static void handleQueryException(
      DatabaseError exception, std::string sql,
      DatabaseManager::parameters sqlParams,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap exceptionMap;
    exceptionMap.insert(
        std::pair<flutter::EncodableValue, flutter::EncodableValue>(
            flutter::EncodableValue("sql"), flutter::EncodableValue(sql)));
    exceptionMap.insert(
        std::pair<flutter::EncodableValue, flutter::EncodableList>(
            flutter::EncodableValue("arguments"), sqlParams));
    result->Error(DATABASE_ERROR_CODE, exception.what(),
                  flutter::EncodableValue(exceptionMap));
  }

  void OnExecuteCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    std::string sql;
    DatabaseManager::parameters params;

    GetValueFromEncodableMap(arguments, "arguments", params);
    GetValueFromEncodableMap(arguments, "sql", sql);
    GetValueFromEncodableMap(arguments, "id", databaseId);

    auto database = getDatabaseOrError(method_call);
    if (database == nullptr) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    try {
      execute(database, sql, params);
    } catch (const DatabaseError &e) {
      result->Error(DATABASE_ERROR_CODE, e.what());
      return;
    }
    result->Success();
  }

  void execute(std::shared_ptr<DatabaseManager> database, std::string sql,
               DatabaseManager::parameters params) {
    database->execute(sql, params);
  }

  int64_t queryUpdateChanges(std::shared_ptr<DatabaseManager> database) {
    std::string changesSql = "SELECT changes();";
    std::list<std::string> columns;
    DatabaseManager::resultset resultset;

    database->query(changesSql, flutter::EncodableList(), columns, resultset);

    auto rs = resultset.begin();
    auto newList = *rs;
    auto it = newList.begin();
    return std::get<int64_t>(it->second);
  }

  std::pair<int64_t, int64_t> queryInsertChanges(
      std::shared_ptr<DatabaseManager> database) {
    std::string changesSql = "SELECT changes(), last_insert_rowid();";
    std::list<std::string> columns;
    DatabaseManager::resultset resultset;

    database->query(changesSql, flutter::EncodableList(), columns, resultset);
    auto rs = resultset.begin();
    auto newList = *rs;
    auto it = newList.begin();
    auto changes = std::get<int64_t>(it->second);
    int lastId = 0;
    if (changes > 0) {
      std::advance(it, 1);
      lastId = std::get<int64_t>(it->second);
    }
    return std::make_pair(changes, lastId);
  }

  flutter::EncodableValue update(std::shared_ptr<DatabaseManager> database,
                                 std::string sql,
                                 DatabaseManager::parameters params,
                                 bool noResult) {
    database->execute(sql, params);
    if (noResult) {
      LOG_DEBUG("ignoring insert result, 'noResult' is turned on");
      return flutter::EncodableValue();
    }

    auto changes = queryUpdateChanges(database);
    return flutter::EncodableValue(changes);
  }

  flutter::EncodableValue insert(std::shared_ptr<DatabaseManager> database,
                                 std::string sql,
                                 DatabaseManager::parameters params,
                                 bool noResult) {
    database->execute(sql, params);
    if (noResult) {
      LOG_DEBUG("ignoring insert result, 'noResult' is turned on");
      return flutter::EncodableValue();
    }

    auto insertChanges = queryInsertChanges(database);

    if (insertChanges.first == 0) {
      return flutter::EncodableValue();
    }
    return flutter::EncodableValue(insertChanges.second);
  }

  struct DBResultVisitor {
    flutter::EncodableValue operator()(int64_t val) {
      return flutter::EncodableValue(val);
    };
    flutter::EncodableValue operator()(std::string val) {
      return flutter::EncodableValue(val);
    };
    flutter::EncodableValue operator()(double val) {
      return flutter::EncodableValue(val);
    };
    flutter::EncodableValue operator()(std::vector<uint8_t> val) {
      return flutter::EncodableValue(val);
    };
    flutter::EncodableValue operator()(std::nullptr_t val) {
      return flutter::EncodableValue();
    };
  };

  flutter::EncodableValue query(std::shared_ptr<DatabaseManager> database,
                                std::string sql,
                                DatabaseManager::parameters params) {
    DatabaseManager::columns columns;
    DatabaseManager::resultset resultset;
    auto dbResultVisitor = DBResultVisitor{};
    database->query(sql, params, columns, resultset);
    if (queryAsMapList) {
      flutter::EncodableList response;
      if (resultset.size() == 0) {
        return flutter::EncodableValue(response);
      }
      for (auto row : resultset) {
        flutter::EncodableMap rowMap;
        for (auto col : row) {
          LOG_DEBUG("Trying to visit value");
          auto rowValue = std::visit(dbResultVisitor, col.second);
          LOG_DEBUG("value visited!");
          rowMap.insert(
              std::pair<flutter::EncodableValue, flutter::EncodableValue>(
                  flutter::EncodableValue(col.first), rowValue));
        }
        response.push_back(flutter::EncodableValue(rowMap));
      }
      return flutter::EncodableValue(response);
    } else {
      flutter::EncodableMap response;
      if (resultset.size() == 0) {
        return flutter::EncodableValue(response);
      }
      flutter::EncodableList colsResponse;
      flutter::EncodableList rowsResponse;
      for (auto col : columns) {
        colsResponse.push_back(flutter::EncodableValue(col));
      }
      for (auto row : resultset) {
        flutter::EncodableList rowList;
        for (auto col : row) {
          auto rowValue = std::visit(dbResultVisitor, col.second);
          rowList.push_back(rowValue);
        }
        rowsResponse.push_back(flutter::EncodableValue(rowList));
      }
      response.insert(
          std::pair<flutter::EncodableValue, flutter::EncodableValue>(
              flutter::EncodableValue("columns"),
              flutter::EncodableValue(colsResponse)));
      response.insert(
          std::pair<flutter::EncodableValue, flutter::EncodableValue>(
              flutter::EncodableValue("rows"),
              flutter::EncodableValue(rowsResponse)));
      return flutter::EncodableValue(response);
    }
  }

  void OnInsertCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    std::string sql;
    DatabaseManager::parameters params;
    bool noResult = false;

    GetValueFromEncodableMap(arguments, "arguments", params);
    GetValueFromEncodableMap(arguments, "sql", sql);
    GetValueFromEncodableMap(arguments, "id", databaseId);
    GetValueFromEncodableMap(arguments, "noResult", noResult);

    auto database = getDatabaseOrError(method_call);
    if (database == nullptr) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    flutter::EncodableValue response;
    try {
      response = insert(database, sql, params, noResult);
    } catch (const DatabaseError &e) {
      handleQueryException(e, sql, params, std::move(result));
      return;
    }
    result->Success(response);
  }

  void OnUpdateCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    std::string sql;
    DatabaseManager::parameters params;
    bool noResult = false;

    GetValueFromEncodableMap(arguments, "arguments", params);
    GetValueFromEncodableMap(arguments, "sql", sql);
    GetValueFromEncodableMap(arguments, "id", databaseId);
    GetValueFromEncodableMap(arguments, "noResult", noResult);

    auto database = getDatabaseOrError(method_call);
    if (database == nullptr) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    flutter::EncodableValue response;
    try {
      response = update(database, sql, params, noResult);
    } catch (const DatabaseError &e) {
      handleQueryException(e, sql, params, std::move(result));
      return;
    }
    result->Success(response);
  }

  void OnOptionsCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    bool paramsAsList = false;
    int logLevel = 0;

    GetValueFromEncodableMap(arguments, "queryAsMapList", paramsAsList);
    GetValueFromEncodableMap(arguments, "logLevel", logLevel);

    queryAsMapList = paramsAsList;
    // TODO: Implement log level usage
    // TODO: Implement Thread Priority usage
    result->Success();
  }

  void OnQueryCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    std::string sql;
    DatabaseManager::parameters params;

    GetValueFromEncodableMap(arguments, "arguments", params);
    GetValueFromEncodableMap(arguments, "sql", sql);
    GetValueFromEncodableMap(arguments, "id", databaseId);
    auto database = getDatabaseOrError(method_call);
    if (database == nullptr) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    flutter::EncodableValue response;
    try {
      response = query(database, sql, params);
    } catch (const DatabaseError &e) {
      handleQueryException(e, sql, params, std::move(result));
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
    databasesPath = path;
    free(path);

    result->Success(flutter::EncodableValue(databasesPath));
  }

  void OnDeleteDatabase(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    std::string path;
    GetValueFromEncodableMap(arguments, "path", path);

    LOG_DEBUG("Trying to delete path %s", path.c_str());
    int *existingDatabaseId = getDatabaseId(path);
    if (existingDatabaseId) {
      LOG_DEBUG("db id exists: %d", *existingDatabaseId);
      auto dbm = getDatabase(*existingDatabaseId);
      if (dbm && dbm->sqliteDatabase) {
        LOG_DEBUG("db exists, deleting it...");
        LOG_DEBUG("erasing db id from map");
        databaseMap.erase(*existingDatabaseId);
        LOG_DEBUG("erasing path from map");
        singleInstancesByPath.erase(path);
      }
    }
    // TODO: Safe check before delete.
    std::filesystem::remove(path);
    result->Success();
  }

  flutter::EncodableValue makeOpenResult(int databaseId,
                                         bool recoveredInTransaction) {
    flutter::EncodableMap response;
    response.insert(std::make_pair(flutter::EncodableValue("id"),
                                   flutter::EncodableValue(databaseId)));
    if (recoveredInTransaction) {
      response.insert(
          std::make_pair(flutter::EncodableValue("recoveredInTransaction"),
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
    bool readOnly = false;
    bool singleInstance = false;

    GetValueFromEncodableMap(arguments, "path", path);
    GetValueFromEncodableMap(arguments, "readOnly", readOnly);
    GetValueFromEncodableMap(arguments, "singleInstance", singleInstance);

    const bool inMemory = isInMemoryPath(path);
    singleInstance = singleInstance && !inMemory;

    if (singleInstance) {
      int foundDatabaseId = 0;
      auto sit = singleInstancesByPath.find(path);
      if (sit != singleInstancesByPath.end()) {
        foundDatabaseId = sit->second;
      }
      if (foundDatabaseId) {
        auto dit = databaseMap.find(foundDatabaseId);
        if (dit != databaseMap.end()) {
          if (dit->second->sqliteDatabase) {
            auto response = makeOpenResult(foundDatabaseId, true);
            result->Success(response);
            return;
          }
        }
      }
    }
    // TODO: Protect with mutex
    const int newDatabaseId = ++databaseId;
    try {
      std::shared_ptr<DatabaseManager> databaseManager =
          std::make_shared<DatabaseManager>(path, newDatabaseId, singleInstance,
                                            0);
      if (!readOnly) {
        LOG_DEBUG("opening read-write database in path %s", path.c_str());
        databaseManager->open();
      } else {
        LOG_DEBUG("opening read only database in path %s", path.c_str());
        databaseManager->openReadOnly();
      }

      // Store dbid in internal map
      // TODO: Protect with mutex
      LOG_DEBUG("saving database id %d for path %s", databaseId, path.c_str());
      if (singleInstance) {
        singleInstancesByPath.insert(
            std::pair<std::string, int>(path, databaseId));
      }
      databaseMap.insert(std::pair<int, std::shared_ptr<DatabaseManager>>(
          databaseId, databaseManager));
    } catch (const DatabaseError &e) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_OPEN_FAILED + " " + path);
      return;
    }

    auto response = makeOpenResult(databaseId, false);
    result->Success(response);
  }

  void OnCloseDatabaseCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    GetValueFromEncodableMap(arguments, "id", databaseId);

    auto database = getDatabaseOrError(method_call);
    if (database == nullptr) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }

    auto path = database->path;

    try {
      LOG_DEBUG("closing database id %d in path %s", databaseId, path.c_str());
      // By erasing the entry from databaseMap, the destructor of
      // DatabaseManager is called, which finalizes all open statements and
      // closes the database.
      // TODO: Protect with mutex
      databaseMap.erase(databaseId);

      if (database->singleInstance) {
        singleInstancesByPath.erase(path);
      }
    } catch (const DatabaseError &e) {
      result->Error(DATABASE_ERROR_CODE, e.what());
      return;
    }

    result->Success();
  };

  flutter::EncodableValue buildSuccessBatchOperationResult(
      flutter::EncodableValue result) {
    flutter::EncodableMap operationResult;
    operationResult.insert(
        std::make_pair(flutter::EncodableValue("result"), result));
    return flutter::EncodableValue(operationResult);
  }

  flutter::EncodableValue buildErrorBatchOperationResult(
      const DatabaseError &e, std::string sql,
      DatabaseManager::parameters params) {
    flutter::EncodableMap operationResult;
    flutter::EncodableMap operationErrorDetailResult;
    flutter::EncodableMap operationErrorDetailData;
    operationErrorDetailResult.insert(
        std::make_pair(flutter::EncodableValue("code"),
                       flutter::EncodableValue(DATABASE_ERROR_CODE)));
    operationErrorDetailResult.insert(std::make_pair(
        flutter::EncodableValue("message"), flutter::EncodableValue(e.what())));
    operationErrorDetailData.insert(std::make_pair(
        flutter::EncodableValue("sql"), flutter::EncodableValue(sql)));
    operationErrorDetailData.insert(std::make_pair(
        flutter::EncodableValue("arguments"), flutter::EncodableValue(params)));
    operationErrorDetailResult.insert(
        std::make_pair(flutter::EncodableValue("data"),
                       flutter::EncodableValue(operationErrorDetailData)));
    operationResult.insert(std::make_pair(flutter::EncodableValue("error"),
                                          operationErrorDetailResult));
    return flutter::EncodableValue(operationResult);
  }

  void OnBatchCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    bool continueOnError = false;
    bool noResult = false;
    flutter::EncodableList operations;
    flutter::EncodableList results;
    GetValueFromEncodableMap(arguments, "id", databaseId);
    GetValueFromEncodableMap(arguments, "operations", operations);
    GetValueFromEncodableMap(arguments, "continueOnError", continueOnError);
    GetValueFromEncodableMap(arguments, "noResult", noResult);

    auto database = getDatabaseOrError(method_call);
    if (database == nullptr) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }

    for (auto item : operations) {
      auto itemMap = std::get<flutter::EncodableMap>(item);
      std::string method;
      std::string sql;
      DatabaseManager::parameters params;
      GetValueFromEncodableMap(itemMap, "method", method);
      GetValueFromEncodableMap(itemMap, "arguments", params);
      GetValueFromEncodableMap(itemMap, "sql", sql);

      if (method == "execute") {
        try {
          execute(database, sql, params);
          if (!noResult) {
            auto operationResult =
                buildSuccessBatchOperationResult(flutter::EncodableValue());
            results.push_back(operationResult);
          }
        } catch (const DatabaseError &e) {
          if (!continueOnError) {
            handleQueryException(e, sql, params, std::move(result));
            return;
          } else {
            if (!noResult) {
              auto operationResult =
                  buildErrorBatchOperationResult(e, sql, params);
              results.push_back(operationResult);
            }
          }
        }
      } else if (method == "insert") {
        try {
          auto response = insert(database, sql, params, noResult);
          if (!noResult) {
            auto operationResult = buildSuccessBatchOperationResult(response);
            results.push_back(operationResult);
          }
        } catch (const DatabaseError &e) {
          if (!continueOnError) {
            handleQueryException(e, sql, params, std::move(result));
            return;
          } else {
            if (!noResult) {
              auto operationResult =
                  buildErrorBatchOperationResult(e, sql, params);
              results.push_back(operationResult);
            }
          }
        }
      } else if (method == "query") {
        try {
          auto response = query(database, sql, params);
          if (!noResult) {
            auto operationResult = buildSuccessBatchOperationResult(response);
            results.push_back(operationResult);
          }
        } catch (const DatabaseError &e) {
          if (!continueOnError) {
            handleQueryException(e, sql, params, std::move(result));
            return;
          } else {
            if (!noResult) {
              auto operationResult =
                  buildErrorBatchOperationResult(e, sql, params);
              results.push_back(operationResult);
            }
          }
        }
      } else if (method == "update") {
        try {
          auto response = update(database, sql, params, noResult);
          if (!noResult) {
            auto operationResult = buildSuccessBatchOperationResult(response);
            results.push_back(operationResult);
          }
        } catch (const DatabaseError &e) {
          if (!continueOnError) {
            handleQueryException(e, sql, params, std::move(result));
            return;
          } else {
            if (!noResult) {
              auto operationResult =
                  buildErrorBatchOperationResult(e, sql, params);
              results.push_back(operationResult);
            }
          }
        }
      } else {
        result->NotImplemented();
        return;
      }
    }
    if (noResult) {
      result->Success();
    } else {
      result->Success(flutter::EncodableValue(results));
    }
  }

  flutter::PluginRegistrar *registrar_;
  std::unique_ptr<PermissionManager> pmm_;
};

void SqflitePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SqflitePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}