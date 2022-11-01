// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_flutter_lwe_plugin.h"

#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <memory>

#include "webview_factory.h"

namespace {

constexpr char kViewType[] = "plugins.flutter.io/webview";

class WebviewFlutterLwePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<WebviewFlutterLwePlugin>();
    registrar->AddPlugin(std::move(plugin));
  }

  WebviewFlutterLwePlugin() {}

  virtual ~WebviewFlutterLwePlugin() {}
};

}  // namespace

void WebviewFlutterLwePluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef core_registrar) {
  flutter::PluginRegistrar* registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(core_registrar);
  FlutterDesktopRegisterViewFactory(
      core_registrar, kViewType, std::make_unique<WebViewFactory>(registrar));
  WebviewFlutterLwePlugin::RegisterWithRegistrar(registrar);
}
