// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_app_manager_plugin.h"

#include <app.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <optional>
#include <string>

#include "log.h"
#include "tizen_app_info.h"
#include "tizen_app_manager.h"

namespace {

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

template <typename T>
static bool GetValueFromEncodableMap(const flutter::EncodableMap *map,
                                     const char *key, T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

class AppContextStreamHandler : public FlStreamHandler {
 public:
  AppContextStreamHandler(bool is_launch) : is_launch_(is_launch) {}

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    OnAppContextEvent callback = [this](std::string app_id,
                                        void *app_context_handle) {
      flutter::EncodableMap map = {
          {flutter::EncodableValue("appId"), flutter::EncodableValue(app_id)},
          {flutter::EncodableValue("handle"),
           flutter::EncodableValue(reinterpret_cast<int>(app_context_handle))},
      };
      events_->Success(flutter::EncodableValue(map));
    };

    TizenAppManager &app_manager = TizenAppManager::GetInstance();
    if (is_launch_) {
      app_manager.SetAppLaunchHandler(callback);
    } else {
      app_manager.SetAppTerminateHandler(callback);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    TizenAppManager::GetInstance().SetAppLaunchHandler(nullptr);
    events_.reset();
    return nullptr;
  }

 private:
  bool is_launch_;
  std::unique_ptr<FlEventSink> events_;
};

class TizenAppManagerPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr = std::unique_ptr<FlMethodResult>;

  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<TizenAppManagerPlugin>();

    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "tizen/app_manager",
        &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin->launch_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/app_manager/launch_event",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->launch_event_channel_->SetStreamHandler(
        std::make_unique<AppContextStreamHandler>(true));

    plugin->terminate_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/app_manager/terminate_event",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->terminate_event_channel_->SetStreamHandler(
        std::make_unique<AppContextStreamHandler>(false));

    registrar->AddPlugin(std::move(plugin));
  }

  TizenAppManagerPlugin() {}

  virtual ~TizenAppManagerPlugin() {}

 private:
  static void AppInfoToEncodableMap(
      TizenAppInfo *app_info,
      std::function<void(flutter::EncodableMap map)> on_success,
      std::function<void(int error_code, std::string error_string)> on_error) {
    std::optional<std::string> app_id = app_info->GetAppId();
    if (!app_id.has_value()) {
      on_error(app_info->GetLastError(), app_info->GetLastErrorString());
      return;
    }

    std::optional<std::string> package_id = app_info->GetPackageId();
    if (!package_id.has_value()) {
      on_error(app_info->GetLastError(), app_info->GetLastErrorString());
      return;
    }

    std::optional<std::string> label = app_info->GetLabel();
    if (!label.has_value()) {
      on_error(app_info->GetLastError(), app_info->GetLastErrorString());
      return;
    }

    std::optional<std::string> type = app_info->GetType();
    if (!type.has_value()) {
      on_error(app_info->GetLastError(), app_info->GetLastErrorString());
      return;
    }

    std::optional<std::string> icon_path = app_info->GetIconPath();

    std::optional<std::string> executable_path = app_info->GetExecutablePath();
    if (!executable_path.has_value()) {
      on_error(app_info->GetLastError(), app_info->GetLastErrorString());
      return;
    }

    TizenAppManager &app_manager = TizenAppManager::GetInstance();
    std::optional<std::string> shared_res_path =
        app_manager.GetSharedResourcePath(app_id.value());
    if (!shared_res_path.has_value()) {
      on_error(app_manager.GetLastError(), app_manager.GetLastErrorString());
      return;
    }

    std::optional<bool> is_no_display = app_info->IsNoDisplay();

    flutter::EncodableMap metadata;
    for (const auto &[key, value] : app_info->GetMetadata()) {
      metadata[flutter::EncodableValue(key)] = flutter::EncodableValue(value);
    }

    flutter::EncodableMap result = {
        {flutter::EncodableValue("appId"),
         flutter::EncodableValue(app_id.value())},
        {flutter::EncodableValue("packageId"),
         flutter::EncodableValue(package_id.value())},
        {flutter::EncodableValue("label"),
         flutter::EncodableValue(label.value())},
        {flutter::EncodableValue("type"),
         flutter::EncodableValue(type.value())},
        {flutter::EncodableValue("iconPath"),
         icon_path.has_value() ? flutter::EncodableValue(icon_path.value())
                               : flutter::EncodableValue()},
        {flutter::EncodableValue("executablePath"),
         flutter::EncodableValue(executable_path.value())},
        {flutter::EncodableValue("sharedResourcePath"),
         flutter::EncodableValue(shared_res_path.value())},
        {flutter::EncodableValue("isNoDisplay"),
         flutter::EncodableValue(is_no_display.value())},
        {flutter::EncodableValue("metadata"),
         flutter::EncodableValue(metadata)},
    };
    on_success(result);
  }

  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "getCurrentAppId") {
      GetCurrentAppId(std::move(result));
    } else if (method_name == "getAppInfo") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      GetAppInfo(arguments, std::move(result));
    } else if (method_name == "getInstalledApps") {
      GetInstalledApps(std::move(result));
    } else if (method_name == "isRunning") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      IsAppRunning(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void GetCurrentAppId(MethodResultPtr result) {
    char *app_id = nullptr;
    int ret = app_get_id(&app_id);
    if (ret == APP_ERROR_NONE) {
      result->Success(flutter::EncodableValue(std::string(app_id)));
      free(app_id);
    } else {
      result->Error(std::to_string(ret), get_error_message(ret));
    }
  }

  void GetAppInfo(const flutter::EncodableMap *arguments,
                  MethodResultPtr result) {
    std::string app_id;
    if (!GetValueFromEncodableMap(arguments, "appId", app_id)) {
      result->Error("Invalid arguments", "No appId provided.");
      return;
    }

    std::unique_ptr<TizenAppInfo> app_info =
        TizenAppManager::GetInstance().GetAppInfo(app_id);

    AppInfoToEncodableMap(
        app_info.get(),
        [result = result.get()](flutter::EncodableMap map) {
          result->Success(flutter::EncodableValue(map));
        },
        [result = result.get()](int error_code, std::string error_string) {
          result->Error(std::to_string(error_code), error_string);
        });
  }

  void GetInstalledApps(MethodResultPtr result) {
    flutter::EncodableList list;
    for (const auto &app_info :
         TizenAppManager::GetInstance().GetAllAppsInfo()) {
      AppInfoToEncodableMap(
          app_info.get(),
          [&list](flutter::EncodableMap map) {
            list.push_back(flutter::EncodableValue(map));
          },
          [app_info = app_info.get()](int error_code,
                                      std::string error_string) {
            std::optional<std::string> app_id = app_info->GetAppId();
            LOG_ERROR("Failed to get app info [%s]: %s",
                      app_id.has_value() ? app_id->c_str() : "",
                      error_string.c_str());
          });
    }
    result->Success(flutter::EncodableValue(list));
  }

  void IsAppRunning(const flutter::EncodableMap *arguments,
                    MethodResultPtr result) {
    std::string app_id;
    if (!GetValueFromEncodableMap(arguments, "appId", app_id)) {
      result->Error("Invalid arguments", "No appId provided.");
      return;
    }

    TizenAppManager &app_manager = TizenAppManager::GetInstance();
    std::optional<bool> is_running = app_manager.IsAppRunning(app_id);
    if (!is_running.has_value()) {
      result->Error(std::to_string(app_manager.GetLastError()),
                    app_manager.GetLastErrorString());
      return;
    }
    result->Success(flutter::EncodableValue(is_running.value()));
  }

  std::unique_ptr<FlEventChannel> launch_event_channel_;
  std::unique_ptr<FlEventChannel> terminate_event_channel_;
};

}  // namespace

void TizenAppManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenAppManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
