// Copyright 2024 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wakelock_plus_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <glib.h>
#include <tizen.h>

#include <cerrno>
#include <memory>
#include <string>
#include <variant>

#include "ftpw_wakelock_plus.h"
#include "log.h"

namespace {

class WakelockPlusTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/wakelock_plus_plugin",
            &flutter::StandardMethodCodec::GetInstance());
    auto plugin = std::make_unique<WakelockPlusTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  WakelockPlusTizenPlugin() {}

  virtual ~WakelockPlusTizenPlugin() {
    if (timer_id_ != 0) {
      g_source_remove(timer_id_);
      timer_id_ = 0;
    }
  }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();
    if (!is_initialized_screensaver_api_) {
      int ret = ftpw_wakelock_plus_screensaver_override_reset(false);
      if (ret != 0) {
        LOG_ERROR("screensaver_override_reset failed: %s",
                  get_error_message(ret));
      }
      is_initialized_screensaver_api_ = true;
    }

    if (method_name == "toggle") {
      const auto &arguments = *method_call.arguments();
      if (std::holds_alternative<bool>(arguments)) {
        bool enable = std::get<bool>(arguments);
        if (enable) {
          int ret = ftpw_wakelock_plus_screensaver_reset_timeout();
          if (ret == -ENOSYS) {
            result->Error("Not supported",
                          "The screensaver API is not supported.");
            return;
          }
          if (ret != 0) {
            result->Error(std::to_string(ret), get_error_message(ret));
            return;
          }
          if (timer_id_ != 0) {
            g_source_remove(timer_id_);
          }
          timer_id_ = g_timeout_add(30000, OnResetScreensaverTimeout, this);
          is_enabled_ = true;
        } else {
          if (timer_id_ != 0) {
            g_source_remove(timer_id_);
            timer_id_ = 0;
          }
          is_enabled_ = false;
        }
        result->Success();
      } else {
        result->Error("Invalid argument",
                      "The argument must be a boolean value.");
      }
    } else if (method_name == "isEnabled") {
      result->Success(flutter::EncodableValue(is_enabled_));
    } else {
      result->NotImplemented();
    }
  }

  static gboolean OnResetScreensaverTimeout(gpointer data) {
    auto *plugin = static_cast<WakelockPlusTizenPlugin *>(data);
    int ret = ftpw_wakelock_plus_screensaver_reset_timeout();
    if (ret != 0) {
      LOG_ERROR("screensaver_reset_timeout failed: %s", get_error_message(ret));
      plugin->timer_id_ = 0;
      return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
  }

  bool is_initialized_screensaver_api_ = false;
  guint timer_id_ = 0;
  bool is_enabled_ = false;
};

}  // namespace

void WakelockPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WakelockPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
