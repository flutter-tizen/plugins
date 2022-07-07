// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_app_manager_plugin.h"

#include <app.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include "log.h"
#include "tizen_app_info.h"

typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;
namespace {

bool ExtractValueFromMap(const flutter::EncodableValue &arguments,
                         const char *key, std::string &out_value) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(arguments);
    auto iter = map.find(flutter::EncodableValue(key));
    if (iter != map.end() && !iter->second.IsNull()) {
      if (auto pval = std::get_if<std::string>(&iter->second)) {
        out_value = *pval;
        return true;
      }
    }
  }
  return false;
}

}  // namespace

class TizenAppManagerPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr = std::unique_ptr<FlMethodResult>;
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<TizenAppManagerPlugin>();
    plugin->SetupChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  TizenAppManagerPlugin() {
    launch_events_ = nullptr;
    terminate_events_ = nullptr;
    has_registered_observer_ = false;
    registered_cnt_ = 0;
  }

  virtual ~TizenAppManagerPlugin() { UnregisterObserver(); }

 private:
  static flutter::EncodableMap GetApplicationInfoMap(
      TizenAppInfo &tizen_app_info) {
    std::string app_id = tizen_app_info.GetAppId();
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    std::string package_id = tizen_app_info.GetPackageId();
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    std::string label = tizen_app_info.GetLabel();
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    std::string type = tizen_app_info.GetType();
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    std::string icon_path = tizen_app_info.GetIconPath();
    std::string executable_path = tizen_app_info.GetExecutablePath();
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    std::string shared_resource_path = tizen_app_info.GetSharedResourcePath();
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    bool is_no_display = tizen_app_info.GetIsNoDisplay();
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    flutter::EncodableMap metadata;
    tizen_app_info.GetForEachMetadata(
        [](const char *key, const char *value, void *user_data) {
          if (key) {
            flutter::EncodableMap *metadata =
                static_cast<flutter::EncodableMap *>(user_data);
            metadata->insert(
                std::pair<flutter::EncodableValue, flutter::EncodableValue>(
                    std::string(key), std::string(value)));
          }
          return true;
        },
        &metadata);
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      return flutter::EncodableMap();
    }

    flutter::EncodableMap value;
    value[flutter::EncodableValue("appId")] = app_id;
    value[flutter::EncodableValue("packageId")] = package_id;
    value[flutter::EncodableValue("label")] = label;
    value[flutter::EncodableValue("type")] = type;
    if (icon_path.size()) {
      value[flutter::EncodableValue("iconPath")] = icon_path;
    }
    value[flutter::EncodableValue("executablePath")] = executable_path;
    value[flutter::EncodableValue("sharedResourcePath")] = shared_resource_path;
    value[flutter::EncodableValue("isNoDisplay")] = is_no_display;
    value[flutter::EncodableValue("metadata")] = metadata;

    return value;
  }

  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &arguments = *method_call.arguments();

    const auto &method_name = method_call.method_name();
    if (method_name == "getCurrentAppId") {
      GetCurrentId(std::move(result));
    } else if (method_name == "getAppInfo") {
      GetApplicationInfo(arguments, std::move(result));
    } else if (method_name == "getInstalledApps") {
      GetInstalledApplicationsInfo(std::move(result));
    } else if (method_name == "isRunning") {
      ApplicationIsRunning(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void RegisterObserver(std::unique_ptr<FlEventSink> &&events) {
    int ret = app_manager_set_app_context_event_cb(
        [](app_context_h app_context, app_context_event_e event,
           void *user_data) {
          if (app_context == nullptr) {
            return;
          }
          char *app_id = nullptr;
          app_context_h clone_context = nullptr;
          int ret = app_context_get_app_id(app_context, &app_id);
          if (ret != APP_MANAGER_ERROR_NONE) {
            LOG_ERROR("get app Id error! : %s", get_error_message(ret));
            return;
          }

          ret = app_context_clone(&clone_context, app_context);
          if (ret != APP_MANAGER_ERROR_NONE) {
            LOG_ERROR("clone context error! : %s", get_error_message(ret));
            free(app_id);
            return;
          }
          int int_address = std::intptr_t(clone_context);
          TizenAppManagerPlugin *plugin =
              static_cast<TizenAppManagerPlugin *>(user_data);
          flutter::EncodableMap msg;
          msg.insert(
              std::pair<flutter::EncodableValue, flutter::EncodableValue>(
                  "appId", std::string(app_id)));
          msg.insert(
              std::pair<flutter::EncodableValue, flutter::EncodableValue>(
                  "handle", int_address));
          if (event == APP_CONTEXT_EVENT_LAUNCHED && plugin->launch_events_) {
            plugin->launch_events_->Success(flutter::EncodableValue(msg));
          } else if (event == APP_CONTEXT_EVENT_TERMINATED &&
                     plugin->terminate_events_) {
            plugin->terminate_events_->Success(flutter::EncodableValue(msg));
          }

          free(app_id);
        },
        this);
    if (ret != APP_MANAGER_ERROR_NONE) {
      char *err_msg = get_error_message(ret);
      LOG_ERROR("Failed app_manager_set_app_context_event_cb : %s", err_msg);
      events->Error("Failed to add callback", std::string(err_msg));
      return;
    }
    has_registered_observer_ = true;
  }

  void UnregisterObserver() {
    if (has_registered_observer_) {
      app_manager_unset_app_context_event_cb();
      has_registered_observer_ = false;
    }
    launch_events_ = nullptr;
    terminate_events_ = nullptr;
  }

  void GetCurrentId(MethodResultPtr result) {
    char *app_id = nullptr;
    int ret = app_get_id(&app_id);
    if (ret == APP_ERROR_NONE) {
      result->Success(flutter::EncodableValue(std::string(app_id)));
    } else {
      char *err_msg = get_error_message(ret);
      LOG_ERROR("Failed to get current app id : %s", err_msg);
      result->Error(std::to_string(ret), "Failed to get current app Id.",
                    flutter::EncodableValue(std::string(err_msg)));
    }

    if (app_id) {
      free(app_id);
    }
  }

  void GetApplicationInfo(const flutter::EncodableValue &arguments,
                          MethodResultPtr result) {
    std::string id = "";
    if (!ExtractValueFromMap(arguments, "appId", id)) {
      result->Error("InvalidArguments", "Please check appId");
      return;
    }
    const char *app_id = id.c_str();
    app_info_h app_info = nullptr;
    int ret = app_manager_get_app_info(app_id, &app_info);
    if (ret != APP_MANAGER_ERROR_NONE) {
      char *error_message = get_error_message(ret);
      LOG_ERROR("Failed to get app_info handler : %s", error_message);
      result->Error(std::to_string(ret), "Failed to get app_info handler.",
                    flutter::EncodableValue(std::string(error_message)));
      return;
    }

    TizenAppInfo tizen_app_info(app_info);
    flutter::EncodableMap value = GetApplicationInfoMap(tizen_app_info);
    app_info_destroy(app_info);
    if (tizen_app_info.GetLastError() != APP_MANAGER_ERROR_NONE) {
      LOG_ERROR("Failed to get app info : %s",
                get_error_message(tizen_app_info.GetLastError()));
      result->Error(
          std::to_string(tizen_app_info.GetLastError()),
          "Failed to get app info.",
          flutter::EncodableValue(tizen_app_info.GetLastErrorString()));
      return;
    }
    result->Success(flutter::EncodableValue(value));
  }

  void GetInstalledApplicationsInfo(MethodResultPtr result) {
    flutter::EncodableList applications;
    int ret = app_manager_foreach_app_info(
        [](app_info_h app_info, void *user_data) {
          if (app_info == nullptr) {
            return false;
          }
          flutter::EncodableList *applications =
              static_cast<flutter::EncodableList *>(user_data);
          TizenAppInfo tizen_app_info(app_info);
          std::string tizen_app_id = tizen_app_info.GetAppId();
          flutter::EncodableMap value =
              TizenAppManagerPlugin::GetApplicationInfoMap(tizen_app_info);
          if (tizen_app_info.GetLastError() == APP_MANAGER_ERROR_NONE) {
            applications->push_back(flutter::EncodableValue(value));
            return true;
          }
          return false;
        },
        &applications);
    if (ret != APP_MANAGER_ERROR_NONE) {
      char *error_message = get_error_message(ret);
      LOG_ERROR("app_manager_foreach_app_info error, %s", error_message);
      result->Error(std::to_string(ret), "app_manager_foreach_app_info error.",
                    flutter::EncodableValue(std::string(error_message)));
      return;
    }
    result->Success(flutter::EncodableValue(applications_));
  }

  void ApplicationIsRunning(const flutter::EncodableValue &arguments,
                            MethodResultPtr result) {
    std::string id = "";

    if (!ExtractValueFromMap(arguments, "appId", id)) {
      result->Error("InvalidArguments", "Please check appId");
      return;
    }
    const char *app_id = id.c_str();
    bool is_running = false;
    int ret = app_manager_is_running(app_id, &is_running);
    if (ret != APP_MANAGER_ERROR_NONE) {
      char *error_message = get_error_message(ret);
      LOG_ERROR("Failed to check application is running : %s", error_message);
      result->Error(std::to_string(ret),
                    "Failed to check application is running.",
                    flutter::EncodableValue(std::string(error_message)));
      return;
    }
    result->Success(flutter::EncodableValue(is_running));
  }

  void SetupChannels(flutter::PluginRegistrar *registrar) {
    auto method_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/app_manager",
            &flutter::StandardMethodCodec::GetInstance());

    method_channel->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });

    auto launch_event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/app_manager/launch_event",
            &flutter::StandardMethodCodec::GetInstance());

    auto terminate_event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/app_manager/terminate_event",
            &flutter::StandardMethodCodec::GetInstance());

    auto launch_event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              launch_events_ = std::move(events);
              if (registered_cnt_ == 0) {
                this->RegisterObserver(std::move(events));
              }
              registered_cnt_++;
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              registered_cnt_--;
              if (registered_cnt_ == 0) {
                this->UnregisterObserver();
              }
              launch_events_ = nullptr;
              return nullptr;
            });

    auto terminate_event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              terminate_events_ = std::move(events);
              if (registered_cnt_ == 0) {
                this->RegisterObserver(std::move(events));
              }
              registered_cnt_++;
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              registered_cnt_--;
              if (registered_cnt_ == 0) {
                this->UnregisterObserver();
              }
              terminate_events_ = nullptr;
              return nullptr;
            });

    launch_event_channel->SetStreamHandler(
        std::move(launch_event_channel_handler));
    terminate_event_channel->SetStreamHandler(
        std::move(terminate_event_channel_handler));
  }

  std::unique_ptr<FlEventSink> launch_events_;
  std::unique_ptr<FlEventSink> terminate_events_;
  flutter::EncodableList applications_;
  bool has_registered_observer_;
  int registered_cnt_;
};

void TizenAppManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenAppManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
