// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wakelock_tizen_plugin.h"

#include <device/power.h>
#include <flutter/basic_message_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>

#include <typeinfo>

#include "log.h"

class IsEnabledMessage {
 public:
  flutter::EncodableValue encode() const {
    flutter::EncodableMap wrapped = {
        {flutter::EncodableValue("enabled"), flutter::EncodableValue(enabled)}};
    return flutter::EncodableValue(wrapped);
  }

  static flutter::EncodableValue decode(flutter::EncodableValue value) {
    auto &map = std::get<flutter::EncodableMap>(value);
    IsEnabledMessage message;
    message.enabled = std::get<bool>(map[flutter::EncodableValue("enabled")]);
    return flutter::CustomEncodableValue(message);
  }

  bool enabled;
};

class ToggleMessage {
 public:
  flutter::EncodableValue encode() const {
    flutter::EncodableMap wrapped = {
        {flutter::EncodableValue("enable"), flutter::EncodableValue(enable)}};
    return flutter::EncodableValue(wrapped);
  }

  static flutter::EncodableValue decode(flutter::EncodableValue value) {
    auto &map = std::get<flutter::EncodableMap>(value);
    ToggleMessage message;
    message.enable = std::get<bool>(map[flutter::EncodableValue("enable")]);
    return flutter::CustomEncodableValue(message);
  }

  bool enable;
};

class WakelockApiCodec : public flutter::StandardCodecSerializer {
 public:
  static const WakelockApiCodec &GetInstance() {
    static WakelockApiCodec sInstance;
    return sInstance;
  }

  // Prevent copying.
  WakelockApiCodec(WakelockApiCodec const &) = delete;
  WakelockApiCodec &operator=(WakelockApiCodec const &) = delete;

  void WriteValue(const flutter::EncodableValue &value,
                  flutter::ByteStreamWriter *stream) const override {
    if (std::holds_alternative<flutter::CustomEncodableValue>(value)) {
      const auto &custom_type = std::get<flutter::CustomEncodableValue>(value);
      if (custom_type.type() == typeid(IsEnabledMessage)) {
        const IsEnabledMessage &my_type_value =
            std::any_cast<IsEnabledMessage>(custom_type);
        stream->WriteByte(static_cast<uint8_t>(128));
        StandardCodecSerializer::WriteValue(my_type_value.encode(), stream);
      } else if (custom_type.type() == typeid(ToggleMessage)) {
        const ToggleMessage &my_type_value =
            std::any_cast<ToggleMessage>(custom_type);
        stream->WriteByte(static_cast<uint8_t>(129));
        StandardCodecSerializer::WriteValue(my_type_value.encode(), stream);
      }
    } else {
      StandardCodecSerializer::WriteValue(value, stream);
    }
  }

 protected:
  WakelockApiCodec() = default;

  flutter::EncodableValue ReadValueOfType(
      uint8_t type, flutter::ByteStreamReader *stream) const override {
    switch (type) {
      case 128:
        return IsEnabledMessage::decode(ReadValue(stream));
        break;
      case 129:
        return ToggleMessage::decode(ReadValue(stream));
      default:
        return StandardCodecSerializer::ReadValueOfType(type, stream);
    }
  }
};

class WakelockTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto enabled_channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), "dev.flutter.pigeon.WakelockApi.isEnabled",
            &flutter::StandardMessageCodec::GetInstance(
                &WakelockApiCodec::GetInstance()));
    auto toggle_channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), "dev.flutter.pigeon.WakelockApi.toggle",
            &flutter::StandardMessageCodec::GetInstance(
                &WakelockApiCodec::GetInstance()));

    auto plugin = std::make_unique<WakelockTizenPlugin>();
    enabled_channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto &message, auto &reply) {
          LOG_DEBUG("Fetching wakelock status: %s",
                    plugin_pointer->wakelocked_ ? "enabled" : "disabled");

          IsEnabledMessage is_enabled_message;
          is_enabled_message.enabled = plugin_pointer->wakelocked_;
          flutter::EncodableMap wrapped = {
              {flutter::EncodableValue("result"),
               flutter::CustomEncodableValue(is_enabled_message)}};
          reply(flutter::EncodableValue(wrapped));
        });

    toggle_channel->SetMessageHandler(
        [plugin_pointer = plugin.get()](const auto &message, auto &reply) {
          bool enable;
          bool argument_parsed = false;
          if (std::holds_alternative<flutter::EncodableList>(message)) {
            const flutter::EncodableList &elist =
                std::get<flutter::EncodableList>(message);
            if (!elist.empty()) {
              const flutter::EncodableValue &args = elist[0];
              if (std::holds_alternative<flutter::CustomEncodableValue>(args)) {
                const auto &custom_type =
                    std::get<flutter::CustomEncodableValue>(args);
                if (custom_type.type() == typeid(ToggleMessage)) {
                  const ToggleMessage &toggle_message =
                      std::any_cast<ToggleMessage>(custom_type);
                  enable = toggle_message.enable;
                  argument_parsed = true;
                }
              }
            }
          }

          if (!argument_parsed) {
            reply(WrapError("Invalid arguments for toggle.", ""));
            return;
          }

          flutter::EncodableMap result_map = {
              {flutter::EncodableValue("result"), flutter::EncodableValue()}};
          flutter::EncodableValue wrapped = flutter::EncodableValue(result_map);
          LOG_DEBUG("Toggling wakelock status to %s",
                    enable ? "enable" : "disable");
          if (enable != plugin_pointer->wakelocked_) {
            const int kTimeoutPermanent = 0;
            int ret = enable ? device_power_request_lock(POWER_LOCK_DISPLAY,
                                                         kTimeoutPermanent)
                             : device_power_release_lock(POWER_LOCK_DISPLAY);
            if (ret == DEVICE_ERROR_NONE) {
              plugin_pointer->wakelocked_ = enable;
            } else {
              std::string details =
                  ret == DEVICE_ERROR_PERMISSION_DENIED
                      ? "You need to declare "
                        "\"http://tizen.org/privilege/display\" "
                        "privilege in your tizen manifest to toggle wakelock."
                      : "";
              wrapped = WrapError(get_error_message(ret), details);
            }
          }
          reply(wrapped);
        });

    registrar->AddPlugin(std::move(plugin));
  }

  WakelockTizenPlugin() : wakelocked_(false) {}
  virtual ~WakelockTizenPlugin() = default;

 private:
  bool wakelocked_;

  static flutter::EncodableValue WrapError(const std::string &message,
                                           const std::string &details) {
    flutter::EncodableMap error_map = {
        {flutter::EncodableValue("code"), flutter::EncodableValue("1")},
        {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
        {flutter::EncodableValue("details"), flutter::EncodableValue(details)},
    };
    flutter::EncodableMap wrapped = {
        {flutter::EncodableValue("error"), flutter::EncodableValue(error_map)},
    };
    return flutter::EncodableValue(wrapped);
  }
};

void WakelockTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WakelockTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
