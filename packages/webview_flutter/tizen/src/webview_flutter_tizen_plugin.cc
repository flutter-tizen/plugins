// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_flutter_tizen_plugin.h"

#include <memory>

#include "webview_factory.h"

static constexpr char kViewType[] = "plugins.flutter.io/webview";

class WebviewFlutterTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<WebviewFlutterTizenPlugin>();
    registrar->AddPlugin(std::move(plugin));
  }
  WebviewFlutterTizenPlugin() {}
  virtual ~WebviewFlutterTizenPlugin() {}
};

void WebviewFlutterTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter::PluginRegistrar* core_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar);
  auto factory = std::make_unique<WebViewFactory>(
      core_registrar, core_registrar->texture_registrar());
  FlutterRegisterViewFactory(registrar, kViewType, std::move(factory));
  WebviewFlutterTizenPlugin::RegisterWithRegistrar(core_registrar);
}
