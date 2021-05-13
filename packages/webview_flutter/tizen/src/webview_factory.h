// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_FACTORY_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_FACTORY_H_

#include "webview.h"
class WebViewFactory : public PlatformViewFactory {
 public:
  WebViewFactory(flutter::PluginRegistrar* registrar,
                 flutter::TextureRegistrar* textureRegistrar);
  virtual void Dispose() override;
  virtual PlatformView* Create(
      int viewId, double width, double height,
      const std::vector<uint8_t>& createParams) override;

 private:
  flutter::TextureRegistrar* textureRegistrar_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_FACTORY_H_
