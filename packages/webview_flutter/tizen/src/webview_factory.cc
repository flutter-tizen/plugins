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

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"
#include "lwe/LWEWebView.h"
#include "webview_flutter_tizen_plugin.h"

WebViewFactory::WebViewFactory(flutter::PluginRegistrar* registrar,
                               flutter::TextureRegistrar* textureRegistrar)
    : PlatformViewFactory(registrar), texture_registrar_(textureRegistrar) {
  char* path = app_get_data_path();
  std::string path_string;
  if (!path || strlen(path) == 0) {
    path_string = "/tmp/";
  } else {
    path_string = path;
    free(path);
    path = nullptr;
  }
  LOG_DEBUG("application data path : %s\n", path_string.c_str());
  std::string localstoragePath =
      path_string + std::string("StarFish_localStorage.db");
  std::string cookiePath = path_string + std::string("StarFish_cookies.db");
  std::string cachePath = path_string + std::string("Starfish_cache.db");

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

  try {
    return new WebView(GetPluginRegistrar(), viewId, texture_registrar_, width,
                       height, params, platform_window_);
  } catch (const std::invalid_argument& ex) {
    LOG_ERROR("[Exception] %s\n", ex.what());
    return nullptr;
  }
}

void WebViewFactory::Dispose() { LWE::LWE::Finalize(); }
