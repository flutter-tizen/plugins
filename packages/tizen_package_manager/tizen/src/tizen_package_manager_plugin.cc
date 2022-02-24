// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_package_manager_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>

#include "package_manager_utils.h"

const char kPackageTypeUnkown[] = "unknown";
const char kPackageTypeTpk[] = "tpk";
const char kPackageTypeWgt[] = "wgt";

class TizenPackageManagerPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr =
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>;

  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<TizenPackageManagerPlugin>();
    plugin->SetupChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  TizenPackageManagerPlugin() {}

  virtual ~TizenPackageManagerPlugin() { UnregisterObserver(); }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      MethodResultPtr result) {
    const auto &arguments = *method_call.arguments();

    if (method_call.method_name().compare("getPackage") == 0) {
      GetPackageInfo(arguments, std::move(result));
    } else if (method_call.method_name().compare("getPackages") == 0) {
      GetAllPackagesInfo(std::move(result));
    } else if (method_call.method_name().compare("install") == 0) {
      Install(arguments, std::move(result));
    } else if (method_call.method_name().compare("uninstall") == 0) {
      Uninstall(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void RegisterObserver(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events) {
    int ret = PACKAGE_MANAGER_ERROR_NONE;
    LOG_INFO("RegisterObserver");

    if (package_manager_h_ == nullptr) {
      ret = package_manager_create(&package_manager_h_);
      if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        char *err_msg = get_error_message(ret);
        LOG_ERROR("Failed package_manager_create : %s", err_msg);
        events->Error("Failed to create package manager handle",
                      std::string(err_msg));
        return;
      }
    }

    package_manager_set_event_status(package_manager_h_,
                                     PACKAGE_MANAGER_STATUS_TYPE_INSTALL |
                                         PACKAGE_MANAGER_STATUS_TYPE_UNINSTALL |
                                         PACKAGE_MANAGER_STATUS_TYPE_UPGRADE);
    ret = package_manager_set_event_cb(package_manager_h_, PackageEventCB,
                                       (void *)this);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      char *err_msg = get_error_message(ret);
      LOG_ERROR("Failed package_manager_set_event_cb : %s", err_msg);
      events->Error("Failed to add callback", std::string(err_msg));
      return;
    }
    is_event_callback_registered_ = true;
  }

  void UnregisterObserver() {
    LOG_INFO("UnregisterObserver");
    if (is_event_callback_registered_ && package_manager_h_) {
      int ret = package_manager_unset_event_cb(package_manager_h_);
      if (ret != PACKAGE_MANAGER_ERROR_NONE) {
        LOG_ERROR("Failed package_manager_unset_event_cb : %s",
                  get_error_message(ret));
      }

      package_manager_destroy(package_manager_h_);
      package_manager_h_ = nullptr;
      is_event_callback_registered_ = false;
    }
    install_events_ = nullptr;
    uninstall_events_ = nullptr;
    update_events_ = nullptr;
  }

  void GetPackageInfo(const flutter::EncodableValue &arguments,
                      MethodResultPtr result) {
    std::string id = "";
    const char *package_id;
    char *err_msg;
    package_info_h package_info = nullptr;
    flutter::EncodableMap value;

    if (!package_manager_utils::ExtractValueFromMap(arguments, "packageId",
                                                    id)) {
      result->Error("InvalidArguments", "Please check packageId");
      return;
    }
    package_id = id.c_str();
    LOG_INFO("GetPackageInfo() package_id : %s", package_id);

    int ret = package_info_create(package_id, &package_info);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || package_info == nullptr) {
      err_msg = get_error_message(ret);
      LOG_ERROR("Failed to get package_info handler : %s", err_msg);
      result->Error(std::to_string(ret),
                    "Failed to create package_info handler.",
                    flutter::EncodableValue(std::string(err_msg)));
      goto cleanup;
    }

    ret = package_manager_utils::GetPackageData(package_info, value);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      err_msg = get_error_message(ret);
      result->Error(std::to_string(ret), "Failed to package info.",
                    flutter::EncodableValue(std::string(err_msg)));
      goto cleanup;
    }
    result->Success(flutter::EncodableValue(value));

  cleanup:
    if (package_info) {
      package_info_destroy(package_info);
    }
  }

  void GetAllPackagesInfo(MethodResultPtr result) {
    LOG_INFO("GetAllPackagesInfo()");
    packages_.erase(packages_.begin(), packages_.end());
    int ret = package_manager_foreach_package_info(PackageInfoCB, (void *)this);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      char *err_msg = get_error_message(ret);
      LOG_ERROR("package_manager_foreach_package_info error: %s", err_msg);
      result->Error(std::to_string(ret),
                    "package_manager_foreach_package_info error.",
                    flutter::EncodableValue(std::string(err_msg)));
    }
    result->Success(flutter::EncodableValue(packages_));
  }

  void Install(const flutter::EncodableValue &arguments,
               MethodResultPtr result) {
    std::string path = "";
    const char *package_path;
    char *err_msg;
    package_manager_request_h package_manager_request = nullptr;
    int request_id;

    if (!package_manager_utils::ExtractValueFromMap(arguments, "path", path)) {
      result->Error("InvalidArguments", "Please check path");
      return;
    }
    package_path = path.c_str();
    LOG_INFO("Install() package_path : %s", package_path);

    int ret = package_manager_request_create(&package_manager_request);
    if (ret != PACKAGE_MANAGER_ERROR_NONE ||
        package_manager_request == nullptr) {
      err_msg = get_error_message(ret);
      LOG_ERROR("Failed to get package_manager_request handler : %s", err_msg);
      result->Error(std::to_string(ret),
                    "Failed to create package_manager_request handler.",
                    flutter::EncodableValue(std::string(err_msg)));
      goto cleanup;
    }

    ret = package_manager_request_install(package_manager_request, package_path,
                                          &request_id);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      err_msg = get_error_message(ret);
      result->Error(std::to_string(ret), "Failed to install.",
                    flutter::EncodableValue(std::string(err_msg)));
      goto cleanup;
    }
    result->Success(flutter::EncodableValue(true));

  cleanup:
    if (package_manager_request) {
      package_manager_request_destroy(package_manager_request);
    }
  }

  void Uninstall(const flutter::EncodableValue &arguments,
                 MethodResultPtr result) {
    std::string id = "";
    const char *package_id;
    char *err_msg;
    package_manager_request_h package_manager_request = nullptr;
    int request_id;

    if (!package_manager_utils::ExtractValueFromMap(arguments, "packageId",
                                                    id)) {
      result->Error("InvalidArguments", "Please check packageId");
      return;
    }
    package_id = id.c_str();
    LOG_INFO("Uninstall() package_id : %s", package_id);

    int ret = package_manager_request_create(&package_manager_request);
    if (ret != PACKAGE_MANAGER_ERROR_NONE ||
        package_manager_request == nullptr) {
      err_msg = get_error_message(ret);
      LOG_ERROR("Failed to get package_manager_request handler : %s", err_msg);
      result->Error(std::to_string(ret),
                    "Failed to create package_manager_request handler.",
                    flutter::EncodableValue(std::string(err_msg)));
      goto cleanup;
    }

    ret = package_manager_request_set_type(package_manager_request,
                                           kPackageTypeUnkown);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      err_msg = get_error_message(ret);
      LOG_ERROR("Failed to set request type : %s", err_msg);
      result->Error(std::to_string(ret), "Failed to set request type.",
                    flutter::EncodableValue(std::string(err_msg)));
      goto cleanup;
    }

    ret = package_manager_request_uninstall(package_manager_request, package_id,
                                            &request_id);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      err_msg = get_error_message(ret);
      result->Error(std::to_string(ret), "Failed to uninstall.",
                    flutter::EncodableValue(std::string(err_msg)));
      goto cleanup;
    }
    result->Success(flutter::EncodableValue(true));

  cleanup:
    if (package_manager_request) {
      package_manager_request_destroy(package_manager_request);
    }
  }

  void SetupChannels(flutter::PluginRegistrar *registrar) {
    auto method_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/package_manager",
            &flutter::StandardMethodCodec::GetInstance());

    method_channel->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });

    auto install_event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/package_manager/install_event",
            &flutter::StandardMethodCodec::GetInstance());

    auto uninstall_event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/package_manager/uninstall_event",
            &flutter::StandardMethodCodec::GetInstance());

    auto update_event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/package_manager/update_event",
            &flutter::StandardMethodCodec::GetInstance());

    auto install_event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnListen install");
              install_events_ = std::move(events);
              if (registered_cnt_ == 0) {
                this->RegisterObserver(std::move(events));
              }
              registered_cnt_++;
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              registered_cnt_--;
              LOG_INFO("OnCancel install");
              if (registered_cnt_ == 0) {
                this->UnregisterObserver();
              }
              install_events_ = nullptr;
              return nullptr;
            });

    auto uninstall_event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnListen uninstall");
              uninstall_events_ = std::move(events);
              if (registered_cnt_ == 0) {
                this->RegisterObserver(std::move(events));
              }
              registered_cnt_++;
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnCancel uninstall");
              registered_cnt_--;
              if (registered_cnt_ == 0) {
                this->UnregisterObserver();
              }
              uninstall_events_ = nullptr;
              return nullptr;
            });

    auto update_event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnListen update");
              update_events_ = std::move(events);
              if (registered_cnt_ == 0) {
                this->RegisterObserver(std::move(events));
              }
              registered_cnt_++;
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnCancel update");
              registered_cnt_--;
              if (registered_cnt_ == 0) {
                this->UnregisterObserver();
              }
              update_events_ = nullptr;
              return nullptr;
            });

    install_event_channel->SetStreamHandler(
        std::move(install_event_channel_handler));
    uninstall_event_channel->SetStreamHandler(
        std::move(uninstall_event_channel_handler));
    update_event_channel->SetStreamHandler(
        std::move(update_event_channel_handler));
  }

  static bool PackageInfoCB(package_info_h package_info, void *user_data) {
    if (package_info) {
      TizenPackageManagerPlugin *plugin =
          (TizenPackageManagerPlugin *)user_data;
      flutter::EncodableMap value;
      int ret = package_manager_utils::GetPackageData(package_info, value);
      if (ret == PACKAGE_MANAGER_ERROR_NONE) {
        plugin->packages_.push_back(flutter::EncodableValue(value));
      }
      return true;
    }
    return false;
  }

  static void PackageEventCB(const char *type, const char *package,
                             package_manager_event_type_e event_type,
                             package_manager_event_state_e event_state,
                             int progress, package_manager_error_e error,
                             void *user_data) {
    LOG_INFO("PackageEventCB, packageId : %s, type: %s", package, type);
    LOG_INFO(
        "event_type: %s, event_state: %s, progress : %d ",
        package_manager_utils::PacakgeEventTypeToString(event_type).c_str(),
        package_manager_utils::PacakgeEventStateToString(event_state).c_str(),
        progress);

    TizenPackageManagerPlugin *plugin = (TizenPackageManagerPlugin *)user_data;
    flutter::EncodableMap msg;
    msg[flutter::EncodableValue("packageId")] =
        flutter::EncodableValue(std::string(package));
    msg[flutter::EncodableValue("type")] =
        flutter::EncodableValue(std::string(type));
    msg[flutter::EncodableValue("eventType")] = flutter::EncodableValue(
        package_manager_utils::PacakgeEventTypeToString(event_type));
    msg[flutter::EncodableValue("eventState")] = flutter::EncodableValue(
        package_manager_utils::PacakgeEventStateToString(event_state));
    msg[flutter::EncodableValue("progress")] =
        flutter::EncodableValue(progress);

    if (event_type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL &&
        plugin->install_events_) {
      plugin->install_events_->Success(flutter::EncodableValue(msg));
    } else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL &&
               plugin->uninstall_events_) {
      plugin->uninstall_events_->Success(flutter::EncodableValue(msg));
    } else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UPDATE &&
               plugin->update_events_) {
      plugin->update_events_->Success(flutter::EncodableValue(msg));
    }
  }

  flutter::EncodableList packages_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> install_events_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>
      uninstall_events_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> update_events_;
  bool is_event_callback_registered_ = false;
  int registered_cnt_ = 0;
  package_manager_h package_manager_h_ = nullptr;
};

void TizenPackageManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenPackageManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
