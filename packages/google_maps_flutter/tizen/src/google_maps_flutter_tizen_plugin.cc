#include "google_maps_flutter_tizen_plugin.h"

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "google_map_factory.h"
#include "log.h"

static constexpr char kViewType[] = "plugins.flutter.io/google_maps";

class GoogleMapsFlutterTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<GoogleMapsFlutterTizenPlugin>();
    registrar->AddPlugin(std::move(plugin));
  }

  GoogleMapsFlutterTizenPlugin() {}
  virtual ~GoogleMapsFlutterTizenPlugin() {}
};

void GoogleMapsFlutterTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter::PluginRegistrar *core_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar);

  auto factory = std::make_unique<GoogleMapFactory>(
      core_registrar, FlutterPluginRegistrarGetTexture(registrar));

  FlutterRegisterViewFactory(registrar, kViewType, std::move(factory));

  GoogleMapsFlutterTizenPlugin::RegisterWithRegistrar(core_registrar);
}
