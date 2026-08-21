// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_flutter_tizen_plugin.h"

#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <memory>

#include "webview.h"
#include "webview_factory.h"

namespace {

constexpr char kViewType[] = "plugins.flutter.io/webview";

// Constructed/destroyed exactly once by flutter-tizen's engine start/stop,
// not per-WebView.
class WebviewFlutterTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<WebviewFlutterTizenPlugin>();
    registrar->AddPlugin(std::move(plugin));
  }

  WebviewFlutterTizenPlugin() { WebView::InitializeEngine(); }

  virtual ~WebviewFlutterTizenPlugin() { WebView::ShutdownEngine(); }
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
