// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_FACTORY_H_
#define FLUTTER_PLUGIN_WEBVIEW_FACTORY_H_

#include <flutter/plugin_registrar.h>
#include <flutter/texture_registrar.h>
#include <flutter_platform_view.h>

#include <vector>

class WebViewFactory : public PlatformViewFactory {
 public:
  WebViewFactory(flutter::PluginRegistrar* registrar, void* win);

  virtual PlatformView* Create(int view_id, double width, double height,
                               const ByteMessage& params) override;

  virtual void Dispose() override;

 private:
  flutter::TextureRegistrar* texture_registrar_;
  void* win_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FACTORY_H_
