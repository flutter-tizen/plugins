// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_H_
#define FLUTTER_PLUGIN_WEBVIEW_H_

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/texture_registrar.h>
#include <flutter_platform_view.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "webview_backend.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;

struct WebViewLifetimeState;
struct RenderState;
class JavaScriptReply;

class WebView : public PlatformView, public WebViewBackend::Delegate {
 public:
  WebView(flutter::PluginRegistrar* registrar, int view_id,
          flutter::TextureRegistrar* texture_registrar, double width,
          double height, const flutter::EncodableValue& params, void* window);
  ~WebView();

  virtual void Dispose() override;
  bool IsInitialized() const { return backend_ != nullptr; }

  virtual void Offset(double left, double top) override;
  virtual void Resize(double width, double height) override;
  virtual void Touch(int type, int button, double x, double y, double dx,
                     double dy) override;
  virtual void SetDirection(int direction) override;

  virtual void ClearFocus() override {}

  virtual bool SendKey(const char* key, const char* string, const char* compose,
                       uint32_t modifiers, uint32_t scan_code,
                       bool is_down) override;

  // Used by NavigationRequestResult after a suspended navigation has been
  // accepted (Resume) or cancelled (Stop) by the Dart shouldOverrideUrlLoading
  // callback.
  void ResumeNavigation();
  void StopNavigation();

  static void ClearAllCache();
  static bool ClearAllCookies();
  static std::string GetDefaultUserAgent();

  // NOTE: Call exactly once before constructing any WebView.
  static void InitializeEngine();
  // NOTE: Call exactly once after destroying every WebView.
  static void ShutdownEngine();

 private:
  void HandleWebViewMethodCall(const FlMethodCall& method_call,
                               std::unique_ptr<FlMethodResult> result);
  void ApplyInitialParams(const flutter::EncodableValue& params);
  void ApplySettings(const flutter::EncodableMap& settings);

  std::string GetWebViewChannelName();

  bool InitWebView();

  bool NavigateProgrammatically(const std::function<bool()>& backend_call);

  void OnFrameRendered(void* tbm_surface) override;
  void OnLoadStarted(const std::string& url) override;
  void OnLoadFinished(const std::string& url) override;
  void OnProgress(int32_t progress) override;
  void OnLoadError(int32_t error_code, const std::string& description,
                   const std::string& failing_url) override;
  void OnConsoleMessage(const std::string& level,
                        const std::string& message) override;
  void OnNavigationPolicyDecide(const std::string& url,
                                const std::string& current_url) override;
  void OnUrlChanged(const std::string& url) override;
  void OnTitleChanged(const std::string& title) override;
  void OnJavaScriptAlertDialog(const std::string& message,
                               const std::string& url) override;
  void OnJavaScriptConfirmDialog(const std::string& message,
                                 const std::string& url) override;
  void OnJavaScriptPromptDialog(const std::string& message,
                                const std::string& default_text,
                                const std::string& url) override;

  std::unique_ptr<WebViewBackend> backend_;
  flutter::TextureRegistrar* texture_registrar_;
  double width_ = 0.0;
  double height_ = 0.0;
  void* window_ = nullptr;
  bool has_navigation_delegate_ = false;
  std::unique_ptr<FlMethodChannel> webview_channel_;
  std::unique_ptr<flutter::TextureVariant> texture_variant_;
  std::shared_ptr<RenderState> render_state_;
  std::shared_ptr<WebViewLifetimeState> lifetime_;
  std::vector<std::weak_ptr<JavaScriptReply>> pending_js_replies_;
  bool texture_registered_ = false;
  bool disposed_ = false;
  bool is_programmatic_navigation_ = false;
  bool is_navigation_cancelled_ = false;
  std::string committed_url_;
  std::string url_before_navigation_;
  int32_t target_scroll_x_ = -1;
  int32_t target_scroll_y_ = -1;
  std::chrono::steady_clock::time_point target_scroll_set_time_;

  static std::set<WebView*> instances_;
  static std::mutex instances_mutex_;
  // Cached on the first WebView creation so static channel callers can read it
  // even after every InAppWebView has been disposed.
  static std::string default_user_agent_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_H_
