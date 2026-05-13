// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_factory.h"

#include <app_common.h>
#include <flutter/encodable_value.h>
#include <flutter/message_codec.h>

#include <memory>
#include <string>
#include <variant>

#include "log.h"
#include "webview.h"

WebViewFactory::WebViewFactory(flutter::PluginRegistrar* registrar,
                               void* window)
    : PlatformViewFactory(registrar), window_(window) {
  texture_registrar_ = registrar->texture_registrar();
}

PlatformView* WebViewFactory::Create(int view_id, double width, double height,
                                     const ByteMessage& params) {
  auto decoded_params = GetCodec().DecodeMessage(params);
  if (!decoded_params) {
    LOG_ERROR("Failed to decode WebView creation params.");
    return nullptr;
  }
  auto webview = std::make_unique<WebView>(GetPluginRegistrar(), view_id,
                                           texture_registrar_, width, height,
                                           *decoded_params, window_);
  if (!webview->IsInitialized()) {
    return nullptr;
  }
  return webview.release();
}

void WebViewFactory::Dispose() {}
