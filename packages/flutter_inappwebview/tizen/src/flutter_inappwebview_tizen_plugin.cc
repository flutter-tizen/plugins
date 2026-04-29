// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_inappwebview_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_tizen.h>

#include <memory>

#include "webview.h"
#include "webview_factory.h"

namespace {

constexpr char kViewType[] = "com.pichillilorenzo/flutter_inappwebview";
constexpr char kManagerChannelName[] =
    "com.pichillilorenzo/flutter_inappwebview_manager";
constexpr char kCookieChannelName[] =
    "com.pichillilorenzo/flutter_inappwebview_cookiemanager";

class FlutterInappwebviewTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto manager_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), kManagerChannelName,
            &flutter::StandardMethodCodec::GetInstance());
    auto cookie_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), kCookieChannelName,
            &flutter::StandardMethodCodec::GetInstance());
    auto plugin = std::make_unique<FlutterInappwebviewTizenPlugin>(
        std::move(manager_channel), std::move(cookie_channel));
    registrar->AddPlugin(std::move(plugin));
  }

  FlutterInappwebviewTizenPlugin(
      std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
          manager_channel,
      std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
          cookie_channel)
      : manager_channel_(std::move(manager_channel)),
        cookie_channel_(std::move(cookie_channel)) {
    manager_channel_->SetMethodCallHandler(
        [this](const auto& call, auto result) {
          HandleManagerMethodCall(call, std::move(result));
        });
    cookie_channel_->SetMethodCallHandler(
        [this](const auto& call, auto result) {
          HandleCookieMethodCall(call, std::move(result));
        });
  }

  virtual ~FlutterInappwebviewTizenPlugin() {}

 private:
  void HandleManagerMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const std::string& method_name = method_call.method_name();
    const auto* arguments = method_call.arguments();

    if (method_name == "clearAllCache") {
      WebView::ClearAllCache();
      result->Success();
    } else if (method_name == "handlesURLScheme") {
      std::string url_scheme;
      if (auto* map = std::get_if<flutter::EncodableMap>(arguments)) {
        auto it = map->find(flutter::EncodableValue("urlScheme"));
        if (it != map->end()) {
          if (auto* value = std::get_if<std::string>(&it->second)) {
            url_scheme = *value;
          }
        }
      }
      const bool is_supported = url_scheme == "http" || url_scheme == "https" ||
                                url_scheme == "file" || url_scheme == "data" ||
                                url_scheme == "about" ||
                                url_scheme == "javascript";
      result->Success(flutter::EncodableValue(is_supported));
    } else if (method_name == "getDefaultUserAgent") {
      // Returns the cached EWK user agent. The value is captured the first
      // time an InAppWebView is created in this process; before that the
      // returned string is empty.
      result->Success(flutter::EncodableValue(WebView::GetDefaultUserAgent()));
    } else {
      result->NotImplemented();
    }
  }

  void HandleCookieMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const std::string& method_name = method_call.method_name();
    if (method_name == "deleteAllCookies") {
      result->Success(flutter::EncodableValue(WebView::ClearAllCookies()));
    } else {
      result->NotImplemented();
    }
  }

  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      manager_channel_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      cookie_channel_;
};

}  // namespace

void FlutterInappwebviewTizenPluginRegisterWithRegistrar(
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
  FlutterInappwebviewTizenPlugin::RegisterWithRegistrar(registrar);
}
