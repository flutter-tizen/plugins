#include "webview_factory.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <flutter_platform_view.h>
#include <flutter_texture_registrar.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"
#include "lwe/LWEWebView.h"
#include "webview_flutter_tizen_plugin.h"

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

PlatformView* WebViewFactory::Create(int viewId, double width, double height,
                                     const std::vector<uint8_t>& createParams) {
  flutter::EncodableMap params;
  auto decodedValue = *GetCodec().DecodeMessage(createParams);
  if (std::holds_alternative<flutter::EncodableMap>(decodedValue)) {
    params = std::get<flutter::EncodableMap>(decodedValue);
  }
  return new WebView(GetPluginRegistrar(), viewId, textureRegistrar_, width,
                     height, params);
}

void WebViewFactory::Dispose() { LWE::LWE::Finalize(); }
