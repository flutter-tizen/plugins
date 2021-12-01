#include "integration_test_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "log.h"

class IntegrationTestPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/integration_test",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<IntegrationTestPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  IntegrationTestPlugin() {}

  virtual ~IntegrationTestPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "allTestsFinished") {
      const auto &arguments = *method_call.arguments();
      if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
        flutter::EncodableMap map = std::get<flutter::EncodableMap>(arguments);
        flutter::EncodableValue results =
            map[flutter::EncodableValue("results")];
        if (std::holds_alternative<flutter::EncodableMap>(results)) {
          flutter::EncodableMap results_map =
              std::get<flutter::EncodableMap>(results);
          for (const auto &pair : results_map) {
            if (std::holds_alternative<std::string>(pair.first) &&
                std::holds_alternative<std::string>(pair.second)) {
              LOG_DEBUG("%s test: %s\n",
                        std::get<std::string>(pair.first).data(),
                        std::get<std::string>(pair.second).data());
            }
          }
        }
      }
      result->Success();
    } else {
      result->NotImplemented();
    }
  }
};

void IntegrationTestPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  IntegrationTestPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
