// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_flutter_tizen_plugin.h"

#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <memory>

#include "webview_factory.h"

namespace {

constexpr char kViewType[] = "plugins.flutter.io/webview";

class WebviewFlutterTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<WebviewFlutterTizenPlugin>();
    registrar->AddPlugin(std::move(plugin));
  }

  WebviewFlutterTizenPlugin() {}

  virtual ~WebviewFlutterTizenPlugin() {}
};

}  // namespace

void WebviewFlutterTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef core_registrar) {
  flutter::PluginRegistrar* registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(core_registrar);
  FlutterDesktopViewRef view =
      FlutterDesktopPluginRegistrarGetView(core_registrar);
  FlutterDesktopRegisterViewFactory(
      core_registrar, kViewType,
      std::make_unique<WebViewFactory>(
          registrar, FlutterDesktopViewGetNativeHandle(view)));
  WebviewFlutterTizenPlugin::RegisterWithRegistrar(registrar);
}
