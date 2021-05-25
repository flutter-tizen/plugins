// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "battery_tizen_plugin.h"

#include <device/battery.h>
#include <device/callback.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <string>

#include "log.h"

class BatteryTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    LOG_DEBUG("RegisterWithRegistrar for BatteryTizenPlugin");
    auto method_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/battery",
            &flutter::StandardMethodCodec::GetInstance());
    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/charging",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<BatteryTizenPlugin>();

    auto method_channel_handler = [plugin_pointer = plugin.get()](
                                      const auto &call, auto result) {
      LOG_DEBUG("HandleMethodCall call");
      plugin_pointer->HandleMethodCall(call, std::move(result));
    };
    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [plugin_pointer = plugin.get()](
                const flutter::EncodableValue *arguments,
                std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen");
              plugin_pointer->RegisterObserver(std::move(events));
              return nullptr;
            },
            [plugin_pointer =
                 plugin.get()](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel");
              plugin_pointer->UnregisterObserver();
              return nullptr;
            });

    method_channel->SetMethodCallHandler(method_channel_handler);
    event_channel->SetStreamHandler(std::move(event_channel_handler));

    registrar->AddPlugin(std::move(plugin));
  }

  BatteryTizenPlugin() {}

  virtual ~BatteryTizenPlugin() {}

  void RegisterObserver(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events) {
    m_events = std::move(events);

    // DEVICE_CALLBACK_BATTERY_CHARGING callback is called in only two cases
    // like charging and discharing. When the charging status becomes "Full",
    // discharging status event is called. So if it is full and disconnected
    // from USB or AC charger, then any callbacks will not be called because the
    // status already is the discharging status. To resolve this issue,
    // DEVICE_CALLBACK_BATTERY_LEVEL callback is added. This callback can check
    // whether it is disconnected in "Full" status. That is, when the battery
    // status is full and disconnected, the level status will be changed from
    // DEVICE_BATTERY_LEVEL_FULL to DEVICE_BATTERY_LEVEL_HIGH.
    int ret = device_add_callback(DEVICE_CALLBACK_BATTERY_CHARGING,
                                  BatteryChangedCB, this);
    if (ret != DEVICE_ERROR_NONE) {
      m_events->Error("failed_to_add_callback", get_error_message(ret));
      return;
    }

    ret = device_add_callback(DEVICE_CALLBACK_BATTERY_LEVEL, BatteryChangedCB,
                              this);
    if (ret != DEVICE_ERROR_NONE) {
      m_events->Error("failed_to_add_callback", get_error_message(ret));
      return;
    }

    std::string status = GetBatteryStatus();
    if (status.empty()) {
      m_events->Error("invalid_status", "Charging status error");
    } else {
      m_events->Success(flutter::EncodableValue(status));
    }
  }

  void UnregisterObserver() {
    int ret = device_remove_callback(DEVICE_CALLBACK_BATTERY_CHARGING,
                                     BatteryChangedCB);
    if (ret != DEVICE_ERROR_NONE) {
      LOG_ERROR("Failed to run device_remove_callback (%d: %s)", ret,
                get_error_message(ret));
    }
    ret =
        device_remove_callback(DEVICE_CALLBACK_BATTERY_LEVEL, BatteryChangedCB);
    if (ret != DEVICE_ERROR_NONE) {
      LOG_ERROR("Failed to run device_remove_callback (%d: %s)", ret,
                get_error_message(ret));
    }

    m_events = nullptr;
  }

  static std::string GetBatteryStatus() {
    device_battery_status_e status;
    int ret = device_battery_get_status(&status);
    if (ret != DEVICE_ERROR_NONE) {
      LOG_ERROR("Failed to run device_battery_get_status (%d: %s)", ret,
                get_error_message(ret));
      return "";
    }

    std::string value;
    if (status == DEVICE_BATTERY_STATUS_CHARGING) {
      value = "charging";
    } else if (status == DEVICE_BATTERY_STATUS_FULL) {
      value = "full";
    } else if (status == DEVICE_BATTERY_STATUS_DISCHARGING ||
               status == DEVICE_BATTERY_STATUS_NOT_CHARGING) {
      value = "discharging";
    }
    LOG_INFO("battery status [%s]", value.c_str());
    return value;
  }

  static void BatteryChangedCB(device_callback_e type, void *value,
                               void *user_data) {
    if (!user_data) {
      LOG_ERROR("Invalid user data");
      return;
    }

    // DEVICE_CALLBACK_BATTERY_LEVEL callback is used only for checking whether
    // the battery became a discharging status while it is full.
    if (type == DEVICE_CALLBACK_BATTERY_LEVEL &&
        intptr_t(value) < DEVICE_BATTERY_LEVEL_HIGH) {
      return;
    }

    BatteryTizenPlugin *plugin_pointer = (BatteryTizenPlugin *)user_data;

    std::string status = GetBatteryStatus();
    bool isFull = (status == "full") ? true : false;
    if (isFull == true && plugin_pointer->m_isFull == true) {
      // This function is called twice by registered callbacks when battery
      // status is full. So, it needs to avoid the unnecessary second call.
      return;
    }
    plugin_pointer->m_isFull = isFull;

    if (status.empty()) {
      plugin_pointer->m_events->Error("invalid_status",
                                      "Charging status error");
    } else {
      plugin_pointer->m_events->Success(flutter::EncodableValue(status));
    }
  }

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &methodCall,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("method : %s", methodCall.method_name().data());

    if (methodCall.method_name().compare("getBatteryLevel") != 0) {
      result->Error("not_supported_method", "Not supported method");
      return;
    }

    int ret, percentage;
    ret = device_battery_get_percent(&percentage);
    if (ret != DEVICE_ERROR_NONE) {
      result->Error("failed_to_get_percentage", get_error_message(ret));
    }
    LOG_INFO("battery percentage [%d]", percentage);
    result->Success(flutter::EncodableValue(percentage));
  }

  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> m_events;
  bool m_isFull;
};

void BatteryTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  BatteryTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
