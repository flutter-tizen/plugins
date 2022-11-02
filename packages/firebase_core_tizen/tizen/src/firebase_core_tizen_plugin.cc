#include "firebase_core_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "log.h"

namespace {

class FirebaseCoreTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/firebase_core",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FirebaseCoreTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  FirebaseCoreTizenPlugin() {}

  virtual ~FirebaseCoreTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "Firebase#initializeCore") {
      // called by:
      // https://github.com/firebase/flutterfire/blob/1da9dc1/packages/firebase_core/firebase_core_platform_interface/lib/src/method_channel/method_channel_firebase.dart#L32
      // accorting to comment there, platform can potentially provide some apps
      // data to initialize
      result->Success(flutter::EncodableValue(flutter::EncodableList()));
    } else if (method_name == "Firebase#initializeApp") {
      // called by:
      // https://github.com/firebase/flutterfire/blob/1da9dc1/packages/firebase_core/firebase_core_platform_interface/lib/src/method_channel/method_channel_firebase.dart#L86
      auto app = std::get<flutter::EncodableMap>(*method_call.arguments());
      auto response = flutter::EncodableMap();
      auto name = app[flutter::EncodableValue("appName")];
      LOG_INFO("Firebase core initializing app `%s`",
               std::get<std::string>(name).c_str());

      response[flutter::EncodableValue("name")] = name;
      const auto options = flutter::EncodableValue("options");
      response[options] = app[options];
      result->Success(flutter::EncodableValue(response));
    } else {
      result->NotImplemented();
    }
  }
};

}  // namespace

void FirebaseCoreTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FirebaseCoreTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
