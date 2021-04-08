#include "webview_flutter_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <flutter_platform_view.h>
#include <flutter_tizen_texture_registrar.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"
#include "webview.h"
#include "webview_factory.h"

static constexpr char kViewType[] = "plugins.flutter.io/webview";

class WebviewFlutterTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<WebviewFlutterTizenPlugin>();
    registrar->AddPlugin(std::move(plugin));
  }
  WebviewFlutterTizenPlugin() {}
  virtual ~WebviewFlutterTizenPlugin() {}
};

void WebviewFlutterTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter::PluginRegistrar* core_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar);
  auto factory = std::make_unique<WebViewFactory>(
      core_registrar, FlutterPluginRegistrarGetTexture(registrar));
  FlutterRegisterViewFactory(registrar, kViewType, std::move(factory));
  WebviewFlutterTizenPlugin::RegisterWithRegistrar(core_registrar);
}
