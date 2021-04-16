// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_factory.h"

#include <app_common.h>
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
#include "lwe/LWEWebView.h"
#include "webview_flutter_tizen_plugin.h"

WebViewFactory::WebViewFactory(flutter::PluginRegistrar* registrar,
                               FlutterTextureRegistrar* textureRegistrar)
    : PlatformViewFactory(registrar), textureRegistrar_(textureRegistrar) {
  char* path = app_get_data_path();
  if (!path || strlen(path) == 0) {
    path = "/tmp/";
  }
  LOG_DEBUG("application data path : %s\n", path);
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

PlatformView* WebViewFactory::Create(int viewId, double width, double height,
                                     const std::vector<uint8_t>& createParams) {
  flutter::EncodableMap params;
  auto decodedValue = *GetCodec().DecodeMessage(createParams);
  if (std::holds_alternative<flutter::EncodableMap>(decodedValue)) {
    params = std::get<flutter::EncodableMap>(decodedValue);
  }

  try {
    return new WebView(GetPluginRegistrar(), viewId, textureRegistrar_, width,
                       height, params);
  } catch (const std::invalid_argument& ex) {
    LOG_ERROR("[Exception] %s\n", ex.what());
    return nullptr;
  }
}

void WebViewFactory::Dispose() { LWE::LWE::Finalize(); }
