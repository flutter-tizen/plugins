// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_package_manager_plugin.h"

#include <Ecore.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "tizen_package_manager.h"

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
bool GetValueFromEncodableMap(const flutter::EncodableMap *map, const char *key,
                              T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

flutter::EncodableMap PackageInfoToMap(PackageInfo package) {
  flutter::EncodableMap map;
  map[flutter::EncodableValue("packageId")] =
      flutter::EncodableValue(package.package_id);
  map[flutter::EncodableValue("label")] =
      flutter::EncodableValue(package.label);
  map[flutter::EncodableValue("type")] = flutter::EncodableValue(package.type);
  if (package.icon_path.has_value()) {
    map[flutter::EncodableValue("iconPath")] =
        flutter::EncodableValue(package.icon_path.value());
  }
  map[flutter::EncodableValue("version")] =
      flutter::EncodableValue(package.version);
  map[flutter::EncodableValue("installedStorageType")] =
      flutter::EncodableValue(package.installed_storage_type);
  map[flutter::EncodableValue("isSystem")] =
      flutter::EncodableValue(package.is_system);
  map[flutter::EncodableValue("isPreloaded")] =
      flutter::EncodableValue(package.is_preloaded);
  map[flutter::EncodableValue("isRemovable")] =
      flutter::EncodableValue(package.is_removable);

  return map;
}

flutter::EncodableMap PackageSizeInfoToMap(PackageSizeInfo size_info) {
  flutter::EncodableMap map;
  map[flutter::EncodableValue("dataSize")] =
      flutter::EncodableValue((int64_t)(size_info.data_size));
  map[flutter::EncodableValue("cacheSize")] =
      flutter::EncodableValue((int64_t)(size_info.cache_size));
  map[flutter::EncodableValue("appSize")] =
      flutter::EncodableValue((int64_t)(size_info.app_size));
  map[flutter::EncodableValue("externalDataSize")] =
      flutter::EncodableValue((int64_t)(size_info.external_data_size));
  map[flutter::EncodableValue("externalCacheSize")] =
      flutter::EncodableValue((int64_t)(size_info.external_cache_size));
  map[flutter::EncodableValue("externalAppSize")] =
      flutter::EncodableValue((int64_t)(size_info.external_app_size));
  return map;
}

class PackageEventStreamHandler : public FlStreamHandler {
 public:
  PackageEventStreamHandler(const std::string &event_type)
      : event_type_(event_type) {}

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    OnPackageEvent callback =
        [this](std::string package_id, std::string package_type,
               PacakgeEventState state, int32_t progress) {
          std::string event_state;
          if (state == PacakgeEventState::kStarted) {
            event_state = "started";
          } else if (state == PacakgeEventState::kProcessing) {
            event_state = "processing";
          } else if (state == PacakgeEventState::kFailed) {
            event_state = "failed";
          } else {
            event_state = "completed";
          }
          flutter::EncodableMap map = {
              {flutter::EncodableValue("packageId"),
               flutter::EncodableValue(package_id)},
              {flutter::EncodableValue("type"),
               flutter::EncodableValue(package_type)},
              {flutter::EncodableValue("eventType"),
               flutter::EncodableValue(event_type_)},
              {flutter::EncodableValue("eventState"),
               flutter::EncodableValue(event_state)},
              {flutter::EncodableValue("progress"),
               flutter::EncodableValue(progress)},
          };
          events_->Success(flutter::EncodableValue(map));
        };

    TizenPackageManager &package_manager = TizenPackageManager::GetInstance();
    if (event_type_ == "install") {
      package_manager.SetInstallHandler(callback);
    } else if (event_type_ == "uninstall") {
      package_manager.SetUninstallHandler(callback);
    } else {
      package_manager.SetUpdateHandler(callback);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    TizenPackageManager &package_manager = TizenPackageManager::GetInstance();
    if (event_type_ == "install") {
      package_manager.SetInstallHandler(nullptr);
    } else if (event_type_ == "uninstall") {
      package_manager.SetUninstallHandler(nullptr);
    } else {
      package_manager.SetUpdateHandler(nullptr);
    }
    events_.reset();
    return nullptr;
  }

 private:
  std::unique_ptr<FlEventSink> events_;
  std::string event_type_;
};

class TizenPackageManagerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<TizenPackageManagerPlugin>();

    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "tizen/package_manager",
        &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin->install_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/package_manager/install_event",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->install_event_channel_->SetStreamHandler(
        std::make_unique<PackageEventStreamHandler>("install"));

    plugin->uninstall_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/package_manager/uninstall_event",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->uninstall_event_channel_->SetStreamHandler(
        std::make_unique<PackageEventStreamHandler>("uninstall"));

    plugin->update_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/package_manager/update_event",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->update_event_channel_->SetStreamHandler(
        std::make_unique<PackageEventStreamHandler>("update"));

    registrar->AddPlugin(std::move(plugin));
  }

  TizenPackageManagerPlugin() {}

  virtual ~TizenPackageManagerPlugin() {}

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "getPackage") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      GetPackageInfo(arguments, std::move(result));
    } else if (method_name == "getPackages") {
      GetAllPackagesInfo(std::move(result));
    } else if (method_name == "getPackageSizeInfo") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      GetPackageSizeInfo(arguments, std::move(result));
    } else if (method_name == "install") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      Install(arguments, std::move(result));
    } else if (method_name == "uninstall") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      Uninstall(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void GetPackageInfo(const flutter::EncodableMap *arguments,
                      std::unique_ptr<FlMethodResult> result) {
    std::string package_id;
    if (!GetValueFromEncodableMap(arguments, "packageId", package_id)) {
      result->Error("Invalid arguments", "No packageId provided.");
      return;
    }

    TizenPackageManager &package_manager = TizenPackageManager::GetInstance();
    std::optional<PackageInfo> package =
        package_manager.GetPackageInfo(package_id);
    if (!package.has_value()) {
      result->Error(std::to_string(package_manager.GetLastError()),
                    package_manager.GetLastErrorString());
      return;
    }
    result->Success(flutter::EncodableValue(PackageInfoToMap(package.value())));
  }

  void GetAllPackagesInfo(std::unique_ptr<FlMethodResult> result) {
    // TizenPackageManager::GetAllPackagesInfo() is an expensive operation and
    // might cause unresponsiveness on low-end devices if run on the platform
    // thread.
    ecore_thread_run(
        [](void *data, Ecore_Thread *thread) {
          auto *result = static_cast<FlMethodResult *>(data);

          TizenPackageManager &package_manager =
              TizenPackageManager::GetInstance();
          std::optional<std::vector<PackageInfo>> packages =
              package_manager.GetAllPackagesInfo();
          if (packages.has_value()) {
            flutter::EncodableList list;
            for (const PackageInfo &package : packages.value()) {
              list.push_back(
                  flutter::EncodableValue(PackageInfoToMap(package)));
            }
            result->Success(flutter::EncodableValue(list));
          } else {
            result->Error(std::to_string(package_manager.GetLastError()),
                          package_manager.GetLastErrorString());
          }
          delete result;
        },
        nullptr,
        [](void *data, Ecore_Thread *thread) {
          auto *result = static_cast<FlMethodResult *>(data);
          result->Error("Operation failed", "Failed to start a thread.");
          delete result;
        },
        result.release());
  }

  void GetPackageSizeInfo(const flutter::EncodableMap *arguments,
                          std::unique_ptr<FlMethodResult> result) {
    std::string package_id;
    if (!GetValueFromEncodableMap(arguments, "packageId", package_id)) {
      result->Error("Invalid arguments", "No packageId provided.");
      return;
    }

    TizenPackageManager &package_manager = TizenPackageManager::GetInstance();
    auto shared_result = std::shared_ptr<FlMethodResult>(std::move(result));
    package_manager.GetPackageSizeInfo(
        package_id, [shared_result, &package_manager](PackageSizeInfo size_info,
                                                      bool success) {
          if (!success) {
            shared_result->Error(std::to_string(package_manager.GetLastError()),
                                 package_manager.GetLastErrorString());
            return;
          }
          shared_result->Success(
              flutter::EncodableValue(PackageSizeInfoToMap(size_info)));
        });
  }

  void Install(const flutter::EncodableMap *arguments,
               std::unique_ptr<FlMethodResult> result) {
    std::string path;
    if (!GetValueFromEncodableMap(arguments, "path", path)) {
      result->Error("Invalid arguments", "No path provided.");
      return;
    }

    TizenPackageManager &package_manager = TizenPackageManager::GetInstance();
    if (!package_manager.Install(path)) {
      result->Error(std::to_string(package_manager.GetLastError()),
                    package_manager.GetLastErrorString());
      return;
    }
    result->Success();
  }

  void Uninstall(const flutter::EncodableMap *arguments,
                 std::unique_ptr<FlMethodResult> result) {
    std::string package_id;
    if (!GetValueFromEncodableMap(arguments, "packageId", package_id)) {
      result->Error("Invalid arguments", "No packageId provided.");
      return;
    }

    TizenPackageManager &package_manager = TizenPackageManager::GetInstance();
    if (!package_manager.Uninstall(package_id)) {
      result->Error(std::to_string(package_manager.GetLastError()),
                    package_manager.GetLastErrorString());
      return;
    }
    result->Success();
  }

  std::unique_ptr<FlEventChannel> install_event_channel_;
  std::unique_ptr<FlEventChannel> uninstall_event_channel_;
  std::unique_ptr<FlEventChannel> update_event_channel_;
};

}  // namespace

void TizenPackageManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenPackageManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
