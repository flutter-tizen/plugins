#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter/standard_message_codec.h>
#include <flutter_platform_view.h>
#include <flutter_texture_registrar.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"
#include "webview_flutter_tizen_plugin.h"
#include "webview_factory.h"
#include "lwe/LWEWebView.h"

WebViewFactory::WebViewFactory(flutter::PluginRegistrar* registrar,
                               FlutterTextureRegistrar* textureRegistrar)
    : PlatformViewFactory(registrar), textureRegistrar_(textureRegistrar) {
  // temporlal soluation
  std::string localstoragePath =
      "/tmp/" + std::string("StarFish_localStorage.db");
  std::string cookiePath = "/tmp/" + std::string("StarFish_cookies.db");
  std::string cachePath = "/tmp/" + std::string("Starfish_cache.db");

  LWE::LWE::Initialize(localstoragePath.c_str(), cookiePath.c_str(),
                       cachePath.c_str());
}

PlatformView* WebViewFactory::create(int viewId, double width, double height,
                                     const std::vector<uint8_t>& createParams) {
  std::string initialUrl = "about:blank";
  auto decoded_value = *getCodec().DecodeMessage(createParams);
  if (std::holds_alternative<flutter::EncodableMap>(decoded_value)) {
    flutter::EncodableMap createParams =
        std::get<flutter::EncodableMap>(decoded_value);
    flutter::EncodableValue initialUrlValue =
        createParams[flutter::EncodableValue("initialUrl")];
    if (std::holds_alternative<std::string>(initialUrlValue)) {
      initialUrl = std::get<std::string>(initialUrlValue);
    }
  }
  return new WebView(getPluginRegistrar(), viewId, textureRegistrar_, width,
                     height, initialUrl);
}

void WebViewFactory::dispose() {
  LWE::LWE::Finalize();
}
