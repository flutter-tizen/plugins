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
#include "webview.h"

WebViewFactory::WebViewFactory(flutter::PluginRegistrar* registrar,
                               void* window)
    : PlatformViewFactory(registrar), window_(window) {
  texture_registrar_ = registrar->texture_registrar();
}

PlatformView* WebViewFactory::Create(int view_id, double width, double height,
                                     double pixel_ratio,
                                     const ByteMessage& params) {
  return new WebView(GetPluginRegistrar(), view_id, texture_registrar_, width,
                     height, pixel_ratio, *GetCodec().DecodeMessage(params),
                     window_);
}

void WebViewFactory::Dispose() {}
