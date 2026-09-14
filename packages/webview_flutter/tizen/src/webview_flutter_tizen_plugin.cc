// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_flutter_tizen_plugin.h"

#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_tizen.h>

#include <memory>

#include "webview.h"
#include "webview_factory.h"

namespace {

constexpr char kViewType[] = "plugins.flutter.io/webview";
constexpr char kCookieManagerChannelName[] =
    "plugins.flutter.io/tizen_cookie_manager";

// Constructed/destroyed exactly once by flutter-tizen's engine start/stop,
// not per-WebView.
class WebviewFlutterTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<WebviewFlutterTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  explicit WebviewFlutterTizenPlugin(flutter::PluginRegistrar* registrar) {
    WebView::InitializeEngine();

    cookie_channel_ = std::make_unique<FlMethodChannel>(
        registrar->messenger(), kCookieManagerChannelName,
        &flutter::StandardMethodCodec::GetInstance());
    cookie_channel_->SetMethodCallHandler(
        [](const FlMethodCall& call, std::unique_ptr<FlMethodResult> result) {
          WebView::HandleCookieMethodCall(call, std::move(result));
        });
  }

  virtual ~WebviewFlutterTizenPlugin() {
    cookie_channel_->SetMethodCallHandler(nullptr);
    WebView::ShutdownEngine();
  }

 private:
  std::unique_ptr<FlMethodChannel> cookie_channel_;
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
