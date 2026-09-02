// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_flutter_lwe_plugin.h"

#include <flutter/encodable_value.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_tizen.h>

#include <memory>
#include <string>
#include <variant>

#include "lwe/LWEWebView.h"
#include "webview.h"
#include "webview_factory.h"

namespace {

constexpr char kViewType[] = "plugins.flutter.io/webview";
constexpr char kCookieManagerChannelName[] =
    "plugins.flutter.io/lwe_cookie_manager";

class WebviewFlutterLwePlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<WebviewFlutterLwePlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  explicit WebviewFlutterLwePlugin(flutter::PluginRegistrar* registrar) {
    cookie_channel_ = std::make_unique<FlMethodChannel>(
        registrar->messenger(), kCookieManagerChannelName,
        &flutter::StandardMethodCodec::GetInstance());
    cookie_channel_->SetMethodCallHandler(
        [](const FlMethodCall& call, std::unique_ptr<FlMethodResult> result) {
          HandleCookieMethodCall(call, std::move(result));
        });
  }

  virtual ~WebviewFlutterLwePlugin() {
    cookie_channel_->SetMethodCallHandler(nullptr);
  }

 private:
  static void HandleCookieMethodCall(const FlMethodCall& method_call,
                                     std::unique_ptr<FlMethodResult> result) {
    const std::string& method_name = method_call.method_name();

    if (method_name == "clearCookies") {
      LWE::CookieManager::GetInstance()->ClearCookies();
      result->Success(flutter::EncodableValue(true));
    } else if (method_name == "getCookies") {
      const auto* url = std::get_if<std::string>(method_call.arguments());
      if (!url) {
        result->Error("Invalid argument", "The argument must be a string.");
        return;
      }
      result->Success(flutter::EncodableValue(
          LWE::CookieManager::GetInstance()->GetCookie(*url)));
    } else {
      result->NotImplemented();
    }
  }

  std::unique_ptr<FlMethodChannel> cookie_channel_;
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
