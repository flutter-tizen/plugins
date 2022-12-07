// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_factory.h"

#include <app_common.h>
#include <flutter/encodable_value.h>
#include <flutter/message_codec.h>

#include <string>
#include <variant>

#include "log.h"
#include "lwe/LWEWebView.h"
#include "webview.h"

static std::string GetAppDataPath() {
  char* path = app_get_data_path();
  if (!path) {
    return "/tmp/";
  }
  std::string result = std::string(path);
  free(path);
  return result;
}

WebViewFactory::WebViewFactory(flutter::PluginRegistrar* registrar)
    : PlatformViewFactory(registrar) {
  texture_registrar_ = registrar->texture_registrar();

  std::string data_path = GetAppDataPath();
  std::string local_storage_path = data_path + "StarFish_localStorage.db";
  std::string cookie_path = data_path + "StarFish_cookies.db";
  std::string cache_path = data_path + "Starfish_cache.db";

  LWE::LWE::Initialize(local_storage_path.c_str(), cookie_path.c_str(),
                       cache_path.c_str());
}

PlatformView* WebViewFactory::Create(int view_id, double width, double height,
                                     const ByteMessage& params) {
  return new WebView(GetPluginRegistrar(), view_id, texture_registrar_, width,
                     height, *GetCodec().DecodeMessage(params));
}

void WebViewFactory::Dispose() { LWE::LWE::Finalize(); }
