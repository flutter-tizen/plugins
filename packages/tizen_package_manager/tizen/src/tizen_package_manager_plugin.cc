// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_package_manager_plugin.h"

#include "tizen_package_manager.h"
#include "tizen_package_manager_plugin.h"

const char kPackageTypeUnkown[] = "unknown";
const char kPackageTypeTpk[] = "tpk";
const char kPackageTypeWgt[] = "wgt";

static bool PackageInfoCB(package_info_h package_info, void *user_data) {
  if (package_info != nullptr) {
    TizenPackageManagerPlugin *plugin = (TizenPackageManagerPlugin *)user_data;
    flutter::EncodableMap value;
    int ret = package_manager_utils::GetPackageData(package_info, value);
    if (ret == PACKAGE_MANAGER_ERROR_NONE) {
      plugin->m_packages.push_back(flutter::EncodableValue(value));
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

  msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "packageId", std::string(package)));
  msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "type", std::string(type)));
  msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "eventType",
      package_manager_utils::PacakgeEventTypeToString(event_type)));
  msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "eventState",
      package_manager_utils::PacakgeEventStateToString(event_state)));
  msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "progress", progress));

  if (event_type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL &&
      plugin->m_install_events != nullptr) {
    plugin->m_install_events->Success(flutter::EncodableValue(msg));
  } else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL &&
             plugin->m_uninstall_events != nullptr) {
    plugin->m_uninstall_events->Success(flutter::EncodableValue(msg));
  } else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UPDATE &&
             plugin->m_update_events != nullptr) {
    plugin->m_update_events->Success(flutter::EncodableValue(msg));
  }
}

TizenPackageManagerPlugin::TizenPackageManagerPlugin()
    : m_registered_event_cb(false),
      m_registered_cnt(0),
      m_package_manager_h(nullptr) {}

TizenPackageManagerPlugin::~TizenPackageManagerPlugin() {
  UnregisterObserver();
}

void TizenPackageManagerPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *registrar) {
  auto plugin = std::make_unique<TizenPackageManagerPlugin>();
  plugin->SetupChannels(registrar);
  registrar->AddPlugin(std::move(plugin));
}

void TizenPackageManagerPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
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

void TizenPackageManagerPlugin::RegisterObserver(
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events) {
  int ret = PACKAGE_MANAGER_ERROR_NONE;
  LOG_INFO("RegisterObserver");

  if (m_package_manager_h == nullptr) {
    ret = package_manager_create(&m_package_manager_h);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      char *err_msg = get_error_message(ret);
      LOG_ERROR("Failed package_manager_create : %s", err_msg);
      events->Error("Failed to create package manager handle",
                    std::string(err_msg));
      return;
    }
  }

  package_manager_set_event_status(m_package_manager_h,
                                   PACKAGE_MANAGER_STATUS_TYPE_INSTALL |
                                       PACKAGE_MANAGER_STATUS_TYPE_UNINSTALL |
                                       PACKAGE_MANAGER_STATUS_TYPE_UPGRADE);
  ret = package_manager_set_event_cb(m_package_manager_h, PackageEventCB,
                                     (void *)this);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    char *err_msg = get_error_message(ret);
    LOG_ERROR("Failed package_manager_set_event_cb : %s", err_msg);
    events->Error("Failed to add callback", std::string(err_msg));
    return;
  }
  m_registered_event_cb = true;
}

void TizenPackageManagerPlugin::UnregisterObserver() {
  LOG_INFO("UnregisterObserver");
  if (m_registered_event_cb == true && m_package_manager_h != nullptr) {
    int ret = package_manager_unset_event_cb(m_package_manager_h);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      LOG_ERROR("Failed package_manager_unset_event_cb : %s",
                get_error_message(ret));
    }

    package_manager_destroy(m_package_manager_h);
    m_package_manager_h = nullptr;
    m_registered_event_cb = false;
  }
  m_install_events = nullptr;
  m_uninstall_events = nullptr;
  m_update_events = nullptr;
}

void TizenPackageManagerPlugin::SetupChannels(
    flutter::PluginRegistrar *registrar) {
  auto method_channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "tizen/package_manager",
          &flutter::StandardMethodCodec::GetInstance());

  method_channel->SetMethodCallHandler([this](const auto &call, auto result) {
    this->HandleMethodCall(call, std::move(result));
  });

  auto m_install_event_channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          registrar->messenger(), "tizen/package_manager/install_event",
          &flutter::StandardMethodCodec::GetInstance());

  auto m_uninstall_event_channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          registrar->messenger(), "tizen/package_manager/uninstall_event",
          &flutter::StandardMethodCodec::GetInstance());

  auto m_update_event_channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          registrar->messenger(), "tizen/package_manager/update_event",
          &flutter::StandardMethodCodec::GetInstance());

  auto install_event_channel_handler =
      std::make_unique<flutter::StreamHandlerFunctions<>>(
          [this](const flutter::EncodableValue *arguments,
                 std::unique_ptr<flutter::EventSink<>> &&events)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_INFO("OnListen install");
            m_install_events = std::move(events);
            if (m_registered_cnt == 0) {
              this->RegisterObserver(std::move(events));
            }
            m_registered_cnt++;
            return nullptr;
          },
          [this](const flutter::EncodableValue *arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            m_registered_cnt--;
            LOG_INFO("OnCancel install");
            if (m_registered_cnt == 0) {
              this->UnregisterObserver();
            }
            m_install_events = nullptr;
            return nullptr;
          });

  auto uninstall_event_channel_handler =
      std::make_unique<flutter::StreamHandlerFunctions<>>(
          [this](const flutter::EncodableValue *arguments,
                 std::unique_ptr<flutter::EventSink<>> &&events)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_INFO("OnListen uninstall");
            m_uninstall_events = std::move(events);
            if (m_registered_cnt == 0) {
              this->RegisterObserver(std::move(events));
            }
            m_registered_cnt++;
            return nullptr;
          },
          [this](const flutter::EncodableValue *arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_INFO("OnCancel uninstall");
            m_registered_cnt--;
            if (m_registered_cnt == 0) {
              this->UnregisterObserver();
            }
            m_uninstall_events = nullptr;
            return nullptr;
          });

  auto update_event_channel_handler =
      std::make_unique<flutter::StreamHandlerFunctions<>>(
          [this](const flutter::EncodableValue *arguments,
                 std::unique_ptr<flutter::EventSink<>> &&events)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_INFO("OnListen update");
            m_update_events = std::move(events);
            if (m_registered_cnt == 0) {
              this->RegisterObserver(std::move(events));
            }
            m_registered_cnt++;
            return nullptr;
          },
          [this](const flutter::EncodableValue *arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_INFO("OnCancel update");
            m_registered_cnt--;
            if (m_registered_cnt == 0) {
              this->UnregisterObserver();
            }
            m_update_events = nullptr;
            return nullptr;
          });

  m_install_event_channel->SetStreamHandler(
      std::move(install_event_channel_handler));
  m_uninstall_event_channel->SetStreamHandler(
      std::move(uninstall_event_channel_handler));
  m_update_event_channel->SetStreamHandler(
      std::move(update_event_channel_handler));
}

void TizenPackageManagerPlugin::GetPackageInfo(
    const flutter::EncodableValue &arguments, MethodResultPtr result) {
  std::string id = "";
  const char *package_id;
  char *err_msg;
  package_info_h package_info = nullptr;
  flutter::EncodableMap value;

  if (!package_manager_utils::ExtractValueFromMap(arguments, "packageId", id)) {
    result->Error("InvalidArguments", "Please check packageId");
    return;
  }
  package_id = id.c_str();
  LOG_INFO("GetPackageInfo() package_id : %s", package_id);

  int ret = package_info_create(package_id, &package_info);
  if (ret != PACKAGE_MANAGER_ERROR_NONE || package_info == nullptr) {
    err_msg = get_error_message(ret);
    LOG_ERROR("Failed to get package_info handler : %s", err_msg);
    result->Error(std::to_string(ret), "Failed to create package_info handler.",
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

void TizenPackageManagerPlugin::GetAllPackagesInfo(MethodResultPtr result) {
  LOG_INFO("GetAllPackagesInfo()");
  m_packages.erase(m_packages.begin(), m_packages.end());
  int ret = package_manager_foreach_package_info(PackageInfoCB, (void *)this);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    char *err_msg = get_error_message(ret);
    LOG_ERROR("package_manager_foreach_package_info error: %s", err_msg);
    result->Error(std::to_string(ret),
                  "package_manager_foreach_package_info error.",
                  flutter::EncodableValue(std::string(err_msg)));
  }
  result->Success(flutter::EncodableValue(m_packages));
}

void TizenPackageManagerPlugin::Install(
    const flutter::EncodableValue &arguments, MethodResultPtr result) {
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
  if (ret != PACKAGE_MANAGER_ERROR_NONE || package_manager_request == nullptr) {
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

void TizenPackageManagerPlugin::Uninstall(
    const flutter::EncodableValue &arguments, MethodResultPtr result) {
  std::string id = "";
  const char *package_id;
  char *err_msg;
  package_manager_request_h package_manager_request = nullptr;
  int request_id;

  if (!package_manager_utils::ExtractValueFromMap(arguments, "packageId", id)) {
    result->Error("InvalidArguments", "Please check packageId");
    return;
  }
  package_id = id.c_str();
  LOG_INFO("Uninstall() package_id : %s", package_id);

  int ret = package_manager_request_create(&package_manager_request);
  if (ret != PACKAGE_MANAGER_ERROR_NONE || package_manager_request == nullptr) {
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

void TizenPackageManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenPackageManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
