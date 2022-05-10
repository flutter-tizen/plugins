// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_factory.h"

#include <app_common.h>
#include <flutter/encodable_value.h>

#include <string>
#include <variant>

#include "log.h"
#include "lwe/LWEWebView.h"

WebViewFactory::WebViewFactory(flutter::PluginRegistrar* registrar,
                               flutter::TextureRegistrar* texture_registrar)
    : PlatformViewFactory(registrar), texture_registrar_(texture_registrar) {
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
  std::string local_storage_path =
      path_string + std::string("StarFish_localStorage.db");
  std::string cookie_path = path_string + std::string("StarFish_cookies.db");
  std::string cache_path = path_string + std::string("Starfish_cache.db");

  LWE::LWE::Initialize(local_storage_path.c_str(), cookie_path.c_str(),
                       cache_path.c_str());
}

PlatformView* WebViewFactory::Create(
    int view_id, double width, double height,
    const std::vector<uint8_t>& create_params) {
  flutter::EncodableMap params;
  auto decoded_value = *GetCodec().DecodeMessage(create_params);
  if (std::holds_alternative<flutter::EncodableMap>(decoded_value)) {
    params = std::get<flutter::EncodableMap>(decoded_value);
  }

  try {
    return new WebView(GetPluginRegistrar(), view_id, texture_registrar_, width,
                       height, params);
  } catch (const std::invalid_argument& ex) {
    LOG_ERROR("[Exception] %s\n", ex.what());
    return nullptr;
  }
}

void WebViewFactory::Dispose() { LWE::LWE::Finalize(); }
