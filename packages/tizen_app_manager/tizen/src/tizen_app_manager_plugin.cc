// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_app_manager_plugin.h"

#include "tizen_app_manager.h"

TizenAppManagerPlugin::TizenAppManagerPlugin() {
  launch_events_ = nullptr;
  terminate_events_ = nullptr;
  has_registered_event_ = false;
  registered_cnt_ = 0;
}

TizenAppManagerPlugin::~TizenAppManagerPlugin() { UnregisterObserver(); }

void TizenAppManagerPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *registrar) {
  LOG_INFO("called RegisterWithRegistrar");
  auto plugin = std::make_unique<TizenAppManagerPlugin>();
  plugin->SetupChannels(registrar);
  registrar->AddPlugin(std::move(plugin));
}

flutter::EncodableList &TizenAppManagerPlugin::Applications() {
  return this->applications_;
}

void TizenAppManagerPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  LOG_INFO("method : %s", method_call.method_name().data());
  const auto &arguments = *method_call.arguments();

  if (method_call.method_name().compare("getCurrentAppId") == 0) {
    GetCurrentId(std::move(result));
  } else if (method_call.method_name().compare("getAppInfo") == 0) {
    GetApplicationInfo(arguments, std::move(result));
  } else if (method_call.method_name().compare("getInstalledApps") == 0) {
    GetInstalledApplicationsInfo(std::move(result));
  } else if (method_call.method_name().compare("isRunning") == 0) {
    ApplicationIsRunning(arguments, std::move(result));
  } else {
    result->NotImplemented();
  }
}

void TizenAppManagerPlugin::RegisterObserver(
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events) {
  LOG_DEBUG("RegisterObserver");
  int ret = app_manager_set_app_context_event_cb(
      [](app_context_h app_context, app_context_event_e event,
         void *user_data) {
        if (app_context == nullptr) {
          return;
        }
        char *app_id = nullptr;
        app_context_h clone_context = nullptr;
        int ret = app_context_get_app_id(app_context, &app_id);
        if (ret != APP_MANAGER_ERROR_NONE || app_id == nullptr) {
          LOG_ERROR("get app Id error! : %s", get_error_message(ret));
          if (app_id) {
            free(app_id);
          }
          return;
        }

        ret = app_context_clone(&clone_context, app_context);
        if (ret != APP_MANAGER_ERROR_NONE || clone_context == nullptr) {
          LOG_ERROR("clone context error! : %s", get_error_message(ret));
          free(app_id);
          return;
        }
        int int_address = std::intptr_t(clone_context);
        LOG_INFO("event: %d, app_id: %s", event, app_id);
        LOG_INFO("app_context: %p, clone_context: %p, int_addr: %p",
                 app_context, clone_context, int_address);
        TizenAppManagerPlugin *plugin =
            static_cast<TizenAppManagerPlugin *>(user_data);
        flutter::EncodableMap msg;
        msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
            "appId", std::string(app_id)));
        msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
            "handle", int_address));
        if (event == APP_CONTEXT_EVENT_LAUNCHED &&
            plugin->launch_events_ != nullptr) {
          plugin->launch_events_->Success(flutter::EncodableValue(msg));
        } else if (event == APP_CONTEXT_EVENT_TERMINATED &&
                   plugin->terminate_events_ != nullptr) {
          plugin->terminate_events_->Success(flutter::EncodableValue(msg));
        }

        free(app_id);
      },
      (void *)this);
  if (ret != APP_MANAGER_ERROR_NONE) {
    char *err_msg = get_error_message(ret);
    LOG_ERROR("Failed app_manager_set_app_context_event_cb : %s", err_msg);
    events->Error("Failed to add callback", std::string(err_msg));
    return;
  }
  has_registered_event_ = true;
}

void TizenAppManagerPlugin::UnregisterObserver() {
  LOG_DEBUG("UnregisterObserver");
  if (has_registered_event_ == true) {
    app_manager_unset_app_context_event_cb();
    has_registered_event_ = false;
  }
  launch_events_ = nullptr;
  terminate_events_ = nullptr;
}

void TizenAppManagerPlugin::GetCurrentId(MethodResultPtr result) {
  char *app_id = nullptr;
  int ret = app_get_id(&app_id);
  LOG_INFO("app_get_id ret== %d, app_id == %s", ret, app_id);
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

void TizenAppManagerPlugin::GetApplicationInfo(
    const flutter::EncodableValue &arguments, MethodResultPtr result) {
  std::string id = "";
  char *err_msg = nullptr;
  app_info_h app_info = nullptr;

  if (!application_utils::ExtractValueFromMap(arguments, "appId", id)) {
    result->Error("InvalidArguments", "Please check appId");
    return;
  }
  const char *app_id = id.c_str();
  LOG_INFO("GetApplicationInfo() app_id : %s", app_id);

  int ret = app_manager_get_app_info(app_id, &app_info);
  if (ret != APP_MANAGER_ERROR_NONE || app_info == nullptr) {
    err_msg = get_error_message(ret);
    LOG_ERROR("Failed to get app_info handler : %s", err_msg);
    result->Error(std::to_string(ret), "Failed to get app_info handler.",
                  flutter::EncodableValue(std::string(err_msg)));
    if (app_info) {
      app_info_destroy(app_info);
    }
    return;
  }

  flutter::EncodableMap value;
  ret = application_utils::GetAppData(app_info, value);
  app_info_destroy(app_info);
  if (ret != APP_MANAGER_ERROR_NONE) {
    err_msg = get_error_message(ret);
    LOG_ERROR("Failed to get app info : %s", err_msg);
    result->Error(std::to_string(ret), "Failed to get app info.",
                  flutter::EncodableValue(std::string(err_msg)));
    return;
  }
  result->Success(flutter::EncodableValue(value));
}

void TizenAppManagerPlugin::ApplicationIsRunning(
    const flutter::EncodableValue &arguments, MethodResultPtr result) {
  std::string id = "";
  bool running = false;

  if (!application_utils::ExtractValueFromMap(arguments, "appId", id)) {
    result->Error("InvalidArguments", "Please check appId");
    return;
  }
  const char *app_id = id.c_str();
  int ret = app_manager_is_running(app_id, &running);
  if (ret != APP_MANAGER_ERROR_NONE) {
    char *err_msg = get_error_message(ret);
    LOG_ERROR("Failed to check application is running : %s", err_msg);
    result->Error(std::to_string(ret),
                  "Failed to check application is running.",
                  flutter::EncodableValue(std::string(err_msg)));
    return;
  }
  result->Success(flutter::EncodableValue(running));
}

void TizenAppManagerPlugin::GetInstalledApplicationsInfo(
    MethodResultPtr result) {
  int ret = APP_MANAGER_ERROR_NONE;
  LOG_INFO("GetInstalledApplicationsInfo()");

  applications_.erase(applications_.begin(), applications_.end());
  ret = app_manager_foreach_app_info(
      [](app_info_h app_info, void *user_data) {
        if (app_info == nullptr) {
          return false;
        }
        TizenAppManagerPlugin *plugin =
            static_cast<TizenAppManagerPlugin *>(user_data);
        flutter::EncodableMap value;
        int ret = application_utils::GetAppData(app_info, value);
        if (ret == APP_MANAGER_ERROR_NONE) {
          plugin->Applications().push_back(flutter::EncodableValue(value));
          return true;
        }
        return false;
      },
      (void *)this);
  if (ret != APP_MANAGER_ERROR_NONE) {
    char *err_msg = get_error_message(ret);
    LOG_ERROR("app_manager_foreach_app_info error, %s", err_msg);
    result->Error(std::to_string(ret), "app_manager_foreach_app_info error.",
                  flutter::EncodableValue(std::string(err_msg)));
    return;
  }
  result->Success(flutter::EncodableValue(applications_));
}

void TizenAppManagerPlugin::SetupChannels(flutter::PluginRegistrar *registrar) {
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
            LOG_INFO("OnListen launch");
            launch_events_ = std::move(events);
            if (registered_cnt_ == 0) {
              this->RegisterObserver(std::move(events));
            }
            registered_cnt_++;
            return nullptr;
          },
          [this](const flutter::EncodableValue *arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_INFO("OnCancel launch");
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
            LOG_INFO("OnListen terminate");
            terminate_events_ = std::move(events);
            if (registered_cnt_ == 0) {
              this->RegisterObserver(std::move(events));
            }
            registered_cnt_++;
            return nullptr;
          },
          [this](const flutter::EncodableValue *arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_INFO("OnCancel terminate");
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

void TizenAppManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenAppManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
