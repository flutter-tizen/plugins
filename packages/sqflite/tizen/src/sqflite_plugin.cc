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
  inline static std::map<int, DatabaseManager> databaseMap;
  inline static std::string databasesPath;
  inline static int internalStorageId;
  inline static bool queryAsMapList = false;
  inline static int databaseId = 0;
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
    for (const auto &elem : singleInstancesByPath) {
      LOG_DEBUG("Item in singleInstancesByPath map: %s-%d", elem.first.c_str(),
                elem.second);
    }
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
    int *result = NULL;
    auto itr = singleInstancesByPath.find(path);
    if (itr != singleInstancesByPath.end()) {
      result = &itr->second;
    }
    return result;
  }
  static DatabaseManager *getDatabase(int databaseId) {
    DatabaseManager *result = NULL;
    auto itr = databaseMap.find(databaseId);
    if (itr != databaseMap.end()) {
      result = &itr->second;
    }
    return result;
  }

  static DatabaseManager *getDatabaseOrError(
      const flutter::MethodCall<flutter::EncodableValue> &method_call) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    GetValueFromEncodableMap(arguments, "id", databaseId);
    DatabaseManager *database = getDatabase(databaseId);

    return database;
  }

  static void OnExecuteCall(
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

    DatabaseManager *database = getDatabaseOrError(method_call);
    if (database == NULL) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    auto resultCode = database->execute(sql, params);
    if (resultCode != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE, std::string("execute failed sql: ") +
                                             database->getErrorMsg());
      return;
    }
    result->Success();
    // LOG_DEBUG("Before thread struct declaration");
    // struct ThreadData {
    //   DatabaseManager *dm;
    //   std::string sql;
    //   flutter::EncodableList params;
    //   std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
    //   result;
    // };
    // LOG_DEBUG("Before thread struct variable declaration");
    // struct ThreadData *thd, d;
    // thd = &d;
    // LOG_DEBUG("Before thread struct variable assignment: database");
    // thd->dm = database;
    // LOG_DEBUG("Before thread struct variable assignment: params");
    // thd->params = params;
    // LOG_DEBUG("Before thread struct variable assignment: sql %s",
    // sql.c_str()); thd->sql = sql; LOG_DEBUG("Before thread struct variable
    // assignment: result"); thd->result = std::move(result);
    // LOG_DEBUG("Before thread execution");
    // ecore_thread_run(
    //     [](void *data, Ecore_Thread *thread) {
    //       ThreadData *thd = (ThreadData *)data;
    //       LOG_DEBUG("Thread cb init");
    //       LOG_DEBUG("Thread cb executing sql: %s", thd->sql.c_str());
    //       int rc = thd->dm->execute(thd->sql, thd->params);
    //       LOG_DEBUG("Thread cb result code: %d", rc);
    //       if (rc != DATABASE_STATUS_OK) {
    //         thd->result->Error(
    //             DATABASE_ERROR_CODE,
    //             std::string("faile while exec sql: ") +
    //             thd->dm->getErrorMsg());
    //       }
    //       LOG_DEBUG("Thread cb executed!");
    //     },
    //     [](void *data, Ecore_Thread *thread) {
    //       ThreadData *thd = (ThreadData *)data;
    //       LOG_DEBUG("Thread cb finished callback start!");
    //       thd->result->Success();
    //       LOG_DEBUG("Thread cb finished!");
    //     },
    //     NULL, thd);
  }

  void update(
      DatabaseManager *database, std::string sql,
      DatabaseManager::parameters params, bool noResult,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    int resultCode = database->execute(sql, params);
    if (resultCode != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE, std::string("execute failed sql: ") +
                                             database->getErrorMsg());
      return;
    }
    if (noResult) {
      LOG_DEBUG("ignoring insert result, 'noResult' is turned on");
      result->Success();
      return;
    }
    std::string changesSql = "SELECT changes();";
    std::list<std::string> columns;
    DatabaseManager::resultset resultset;

    resultCode = database->query(changesSql, flutter::EncodableList(), columns,
                                 resultset);
    if (resultCode != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE, std::string("fail while exec sql: ") +
                                             database->getErrorMsg());
      return;
    }
    auto rs = resultset.begin();
    auto newList = *rs;
    auto it = newList.begin();
    auto changes = std::get<int>(it->second);
    result->Success(flutter::EncodableValue(changes));
  }

  void insert(
      DatabaseManager *database, std::string sql,
      DatabaseManager::parameters params, bool noResult,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    int resultCode = database->execute(sql, params);
    if (resultCode != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE, std::string("execute failed sql: ") +
                                             database->getErrorMsg());
      return;
    }
    if (noResult) {
      LOG_DEBUG("ignoring insert result, 'noResult' is turned on");
      result->Success();
      return;
    }
    std::string changesSql = "SELECT changes(), last_insert_rowid();";
    std::list<std::string> columns;
    DatabaseManager::resultset resultset;

    resultCode = database->query(changesSql, flutter::EncodableList(), columns,
                                 resultset);
    if (resultCode != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE, std::string("fail while exec sql: ") +
                                             database->getErrorMsg());
      return;
    }
    auto rs = resultset.begin();
    auto newList = *rs;
    auto it = newList.begin();
    auto changes = std::get<int>(it->second);
    if (changes == 0) {
      result->Success();
      return;
    } else {
      std::advance(it, 1);
      auto lastId = std::get<int>(it->second);
      result->Success(flutter::EncodableValue(lastId));
    };
  }

  void query(
      DatabaseManager *database, std::string sql,
      DatabaseManager::parameters params,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    DatabaseManager::columns columns;
    DatabaseManager::resultset resultset;
    int resultCode = database->query(sql, params, columns, resultset);
    if (resultCode != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE, std::string("fail while exec sql: ") +
                                             database->getErrorMsg());
      return;
    }
    const auto columnsLength = columns.size();
    const auto resultsetLength = resultset.size();
    if (queryAsMapList) {
      flutter::EncodableList response;
      for (auto row : resultset) {
        flutter::EncodableMap rowMap;
        for (auto col : row) {
          flutter::EncodableValue rowValue;
          LOG_DEBUG("Col type is %d", col.second.index());
          switch (col.second.index()) {
            case 0:
              rowValue = flutter::EncodableValue(std::get<int>(col.second));
              break;
            case 1:
              rowValue =
                  flutter::EncodableValue(std::get<std::string>(col.second));
              break;
            case 2:
              rowValue = flutter::EncodableValue(std::get<double>(col.second));
              break;
            case 3:
              rowValue = flutter::EncodableValue();
              break;
            default:
              break;
          }
          rowMap.insert(
              std::pair<flutter::EncodableValue, flutter::EncodableValue>(
                  flutter::EncodableValue(col.first), rowValue));
        }
        response.push_back(flutter::EncodableValue(rowMap));
      }
      result->Success(flutter::EncodableValue(response));
    } else {
      flutter::EncodableMap response;
      flutter::EncodableList colsResponse;
      flutter::EncodableList rowsResponse;
      for (auto col : columns) {
        LOG_DEBUG("pushing back col %s", col.c_str());
        colsResponse.push_back(flutter::EncodableValue(col));
      }
      for (auto row : resultset) {
        flutter::EncodableList rowList;
        for (auto col : row) {
          LOG_DEBUG("Col type is %d", col.second.index());
          switch (col.second.index()) {
            case 0:
              rowList.push_back(
                  flutter::EncodableValue(std::get<int>(col.second)));
              break;
            case 1:
              rowList.push_back(
                  flutter::EncodableValue(std::get<std::string>(col.second)));
              break;
            case 2:
              rowList.push_back(
                  flutter::EncodableValue(std::get<double>(col.second)));
            case 3:
              rowList.push_back(flutter::EncodableValue());
              break;
            default:
              break;
          }
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
      result->Success(flutter::EncodableValue(response));
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

    DatabaseManager *database = getDatabaseOrError(method_call);
    if (database == NULL) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    return insert(database, sql, params, noResult, std::move(result));
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

    DatabaseManager *database = getDatabaseOrError(method_call);
    if (database == NULL) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    return update(database, sql, params, noResult, std::move(result));
  }

  void OnOptionsCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    bool paramsAsList;
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
    DatabaseManager *database = getDatabaseOrError(method_call);
    if (database == NULL) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }
    return query(database, sql, params, std::move(result));
  }

  void OnGetDatabasesPathCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    auto path = app_get_data_path();
    if (path == NULL) {
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
      DatabaseManager *dbm = getDatabase(*existingDatabaseId);
      if (dbm && dbm->sqliteDatabase) {
        LOG_DEBUG("db exists, deleting it...");
        int closeResult = dbm->close();
        if (closeResult != DATABASE_STATUS_OK) {
          result->Error(DATABASE_ERROR_CODE,
                        std::string("close failed: ") + dbm->getErrorMsg());
          return;
        }
        databaseMap.erase(*existingDatabaseId);
        singleInstancesByPath.erase(path);
      }
    }
    // TODO: Safe check before delete.
    std::filesystem::remove(path);
    result->Success();
  }

  void OnOpenDatabaseCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    std::string path;
    bool readOnly;
    GetValueFromEncodableMap(arguments, "path", path);
    GetValueFromEncodableMap(arguments, "readOnly", readOnly);
    // const bool inMemory = isInMemoryPath(path);
    const int newDatabaseId = ++databaseId;

    DatabaseManager databaseManager =
        DatabaseManager(path, newDatabaseId, true, 0);

    int returnCode = DATABASE_STATUS_OK;
    if (readOnly) {
      LOG_DEBUG("opening read only database in path %s", path.c_str());
      returnCode = databaseManager.open();
    } else {
      LOG_DEBUG("opening read-write database in path %s", path.c_str());
      returnCode = databaseManager.openReadOnly();
    }
    if (returnCode != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE, std::string("open_failed: ") +
                                             databaseManager.getErrorMsg() +
                                             std::string(", target path: ") +
                                             path);
      return;
    }

    // Store dbid in internal map
    LOG_DEBUG("saving database id %d for path %s", databaseId, path.c_str());
    singleInstancesByPath.insert(std::pair<std::string, int>(path, databaseId));
    databaseMap.insert(
        std::pair<int, DatabaseManager>(databaseId, databaseManager));

    result->Success(flutter::EncodableValue(flutter::EncodableMap{
        {flutter::EncodableValue("id"), flutter::EncodableValue(databaseId)},
    }));
  }

  void OnCloseDatabaseCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    GetValueFromEncodableMap(arguments, "id", databaseId);

    DatabaseManager *database = getDatabaseOrError(method_call);
    if (database == NULL) {
      result->Error(DATABASE_ERROR_CODE, DATABASE_MSG_ERROR_CLOSED + " " +
                                             std::to_string(databaseId));
      return;
    }

    auto path = database->path;

    // Remove from map right away
    databaseMap.erase(databaseId);

    if (database->singleInstance) {
      singleInstancesByPath.erase(path);
    }

    LOG_DEBUG("closing database id %d in path %s", databaseId, path.c_str());
    const int closeResult = database->close();
    if (closeResult != DATABASE_STATUS_OK) {
      result->Error(DATABASE_ERROR_CODE,
                    std::string("close_failed: ") + database->getErrorMsg());
      return;
    }

    result->Success();
  };

  flutter::PluginRegistrar *registrar_;
  std::unique_ptr<PermissionManager> pmm_;
};

void SqflitePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SqflitePlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}