// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sqflite_plugin.h"

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
#include "sqlite3.h"

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

static std::map<std::string, int> _singleInstancesByPath;
static std::map<int, DatabaseManager> databaseMap;
// static bool QUERY_AS_MAP_LIST = false;
static std::string databasesPath;
static int databaseId = 0;
static int internal_storage_id;

class SqflitePlugin : public flutter::Plugin {
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
    // CheckPermissions();
    std::string method_name = method_call.method_name();
    if (method_name == "openDatabase") {
      OnOpenDatabaseCall(method_call, std::move(result));
    } else if (method_name == "closeDatabase") {
      OnCloseDatabaseCall(method_call, std::move(result));
    } else if (method_name == "deleteDatabase") {
      OnDeleteDatabase(method_call, std::move(result));
    } else if (method_name == "getDatabasesPath") {
      OnGetDatabasesPathCall(method_call, std::move(result));
      // } else if (method_name == "execute") {
      //   OnExecuteCall(method_call, std::move(result));
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
  int *getDatabaseId(std::string path) {
    auto it = _singleInstancesByPath.find(path);
    if (it != _singleInstancesByPath.end()) {
      return &it->second;
    } else {
      return NULL;
    }
  }
  DatabaseManager *getDatabase(int databaseId) {
    auto it = databaseMap.find(databaseId);
    if (it != databaseMap.end()) {
      return &it->second;
    } else {
      return NULL;
    }
  }

  DatabaseManager *getDatabaseOrError(
      const flutter::MethodCall<flutter::EncodableValue> &method_call) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    GetValueFromEncodableMap(arguments, "id", databaseId);
    DatabaseManager *database = getDatabase(databaseId);

    return database;
  }

  void OnExecuteCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableMap arguments =
        std::get<flutter::EncodableMap>(*method_call.arguments());
    int databaseId;
    std::string sql;

    auto params = std::get<flutter::EncodableList>(
        arguments[flutter::EncodableValue("arguments")]);
    GetValueFromEncodableMap(arguments, "sql", sql);
    GetValueFromEncodableMap(arguments, "id", databaseId);
    DatabaseManager *database = getDatabaseOrError(method_call);
    if (database == NULL) {
      result->Error("sqlite_error",
                    "database_closed " + std::to_string(databaseId));
      return;
    }
    int rc = database->execSQL(sql, params);
    if (rc != SQLITE_OK) {
      result->Error("sqlite_error", std::string("faile while exec sql: ") +
                                        database->getErrorMsg());
      return;
    }
    result->Success();
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

    const int *existingDatabaseId = getDatabaseId(path);
    if (existingDatabaseId != NULL) {
      DatabaseManager *dbm = getDatabase(*existingDatabaseId);
      if (dbm != NULL && dbm->sqliteDatabase != NULL) {
        int closeResult = dbm->close();
        if (closeResult != SQLITE_OK) {
          result->Error("sqlite_error",
                        std::string("close failed: ") + dbm->getErrorMsg());
          return;
        }
        std::filesystem::remove(path);
        databaseMap.erase(*existingDatabaseId);
        _singleInstancesByPath.erase(path);
      }
    }
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

    int returnCode;
    if (readOnly) {
      LOG_DEBUG("opening read only database at path %s", path.c_str());
      returnCode = databaseManager.open();
    } else {
      LOG_DEBUG("opening read-write database at path %s", path.c_str());
      returnCode = databaseManager.openReadOnly();
    }
    if (returnCode != SQLITE_OK) {
      result->Error("sqlite_error", std::string("open_failed: ") +
                                        databaseManager.getErrorMsg() +
                                        std::string(", target path: ") + path);
      return;
    }

    // Store dbid in internal map
    LOG_DEBUG("saving database id %d for path %s", databaseId, path.c_str());
    _singleInstancesByPath.insert(
        std::pair<std::string, int>(path, databaseId));
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
      result->Error("sqlite_error",
                    "database_closed " + std::to_string(databaseId));
      return;
    }

    const std::string path = database->path;

    // Remove from map right away
    databaseMap.erase(databaseId);

    if (database->singleInstance) {
      _singleInstancesByPath.erase(path);
    }

    LOG_DEBUG("closing database id %d at path %s", databaseId,
              database->path.c_str());
    const int closeResult = database->close();
    if (closeResult != SQLITE_OK) {
      result->Error("sqlite_error",
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