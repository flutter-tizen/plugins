#include "wakelock_tizen_plugin.h"

#include <device/power.h>
#include <flutter/basic_message_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>

#include "log.h"

class WakelockTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    LOG_DEBUG(
        "[WakelockTizenPlugin.RegisterWithRegistrar] Setting up channels.");

    auto enabled_channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), "dev.flutter.pigeon.WakelockApi.isEnabled",
            &flutter::StandardMessageCodec::GetInstance());
    auto toggle_channel =
        std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
            registrar->messenger(), "dev.flutter.pigeon.WakelockApi.toggle",
            &flutter::StandardMessageCodec::GetInstance());

    auto plugin = std::make_unique<WakelockTizenPlugin>();
    enabled_channel->SetMessageHandler([plugin_pointer = plugin.get()](
                                           const auto &message, auto &reply) {
      LOG_DEBUG("[WakelockTizenPlugin] Fetching wakelock status.");
      LOG_DEBUG("[WakelockTizenPlugin] wakelock status: %s",
                plugin_pointer->wakelocked_ ? "true" : "false");

      flutter::EncodableMap resultMap = {
          {flutter::EncodableValue("enabled"),
           flutter::EncodableValue(plugin_pointer->wakelocked_)}};
      flutter::EncodableMap wrapped = {{flutter::EncodableValue("result"),
                                        flutter::EncodableValue(resultMap)}};
      reply(flutter::EncodableValue(wrapped));
    });
    toggle_channel->SetMessageHandler([plugin_pointer = plugin.get()](
                                          const auto &message, auto &reply) {
      LOG_DEBUG("[WakelockTizenPlugin] Toggling wakelock status.");

      bool enable;
      if (std::holds_alternative<flutter::EncodableMap>(message)) {
        flutter::EncodableMap emap = std::get<flutter::EncodableMap>(message);
        flutter::EncodableValue &enable_encoded =
            emap[flutter::EncodableValue("enable")];
        if (std::holds_alternative<bool>(enable_encoded)) {
          enable = std::get<bool>(enable_encoded);
        } else {
          LOG_ERROR("[WakelockTizenPlugin] Invalid arguments for toggle.");
          reply(WrapError("Invalid arguments for toggle.", ""));
          return;
        }
      } else {
        LOG_ERROR("[WakelockTizenPlugin] Invalid arguments for toggle.");
        reply(WrapError("Invalid arguments for toggle.", ""));
        return;
      }

      flutter::EncodableValue wrapped;
      LOG_DEBUG("[WakelockTizenPlugin] toggle to enable: %s",
                enable ? "true" : "false");
      if (enable != plugin_pointer->wakelocked_) {
        const int WAKELOCK_PERMANENT = 0;
        int ret = enable ? device_power_request_lock(POWER_LOCK_DISPLAY,
                                                     WAKELOCK_PERMANENT)
                         : device_power_release_lock(POWER_LOCK_DISPLAY);
        if (ret == DEVICE_ERROR_NONE) {
          plugin_pointer->wakelocked_ = enable;
        } else {
          LOG_ERROR("[WakelockTizenPlugin] toggling wakelock failed: %s",
                    get_error_message(ret));
          wrapped = WrapError(
              get_error_message(ret),
              "You need to declare \"http://tizen.org/privilege/display\" "
              "privilege in your tizen manifest to toggle wakelock.");
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
