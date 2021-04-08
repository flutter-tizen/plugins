#include "google_map_factory.h"

#include <app_common.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "google_maps_flutter_tizen_plugin.h"
#include "log.h"
#include "lwe/LWEWebView.h"

GoogleMapFactory::GoogleMapFactory(flutter::PluginRegistrar* registrar,
                                   FlutterTextureRegistrar* textureRegistrar)
    : PlatformViewFactory(registrar), textureRegistrar_(textureRegistrar) {
  char* path = app_get_data_path();
  if (!path || strlen(path) == 0) {
    path = strdup("/tmp/");
  }
  std::string localstoragePath = path + std::string("StarFish_localStorage.db");
  std::string cookiePath = path + std::string("StarFish_cookies.db");
  std::string cachePath = path + std::string("Starfish_cache.db");

  LWE::LWE::Initialize(localstoragePath.c_str(), cookiePath.c_str(),
                       cachePath.c_str());

  if (path) {
    free(path);
    path = nullptr;
  }
}

PlatformView* GoogleMapFactory::Create(
    int viewId, double width, double height,
    const std::vector<uint8_t>& createParams) {
  flutter::EncodableMap params;
  auto decodedValue = *GetCodec().DecodeMessage(createParams);
  if (std::holds_alternative<flutter::EncodableMap>(decodedValue)) {
    params = std::get<flutter::EncodableMap>(decodedValue);
  }

  try {
    return new GoogleMapController(GetPluginRegistrar(), viewId,
                                   textureRegistrar_, width, height, params);
  } catch (const std::invalid_argument& ex) {
    LOG_ERROR("[Exception] %s\n", ex.what());
    return nullptr;
  }
}

void GoogleMapFactory::Dispose() { LWE::LWE::Finalize(); }
