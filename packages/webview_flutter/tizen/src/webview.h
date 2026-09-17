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

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>

#include "webview_backend.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;

class BufferPool;
class BufferUnit;

class WebView : public PlatformView, public WebViewBackend::Delegate {
 public:
  WebView(flutter::PluginRegistrar* registrar, int view_id,
          flutter::TextureRegistrar* texture_registrar, double width,
          double height, const flutter::EncodableValue& params, void* window);
  ~WebView();

  virtual void Dispose() override;

  virtual void Offset(double left, double top) override;
  virtual void Resize(double width, double height) override;
  virtual void Touch(int type, int button, double x, double y, double dx,
                     double dy) override;
  virtual void SetDirection(int direction) override;

  virtual void ClearFocus() override {}

  virtual bool SendKey(const char* key, const char* string, const char* compose,
                       uint32_t modifiers, uint32_t scan_code,
                       bool is_down) override;

  void Resume();

  void Stop();

  FlutterDesktopGpuSurfaceDescriptor* ObtainGpuSurface(size_t width,
                                                       size_t height);

  // Must be called exactly once, before any WebView is constructed.
  static void InitializeEngine();

  // Must be called exactly once, after every WebView has been destroyed.
  static void ShutdownEngine();

 private:
  void HandleWebViewMethodCall(const FlMethodCall& method_call,
                               std::unique_ptr<FlMethodResult> result);
  void HandleCookieMethodCall(const FlMethodCall& method_call,
                              std::unique_ptr<FlMethodResult> result);

  template <typename T>
  void SetBackgroundColor(const T& color);

  std::string GetWebViewChannelName();
  std::string GetWebViewControllerChannelName();
  std::string GetNavigationDelegateChannelName();

  void OnFrameRendered(void* tbm_surface) override;
  void OnLoadStarted(const std::string& url) override;
  void OnLoadFinished(const std::string& url) override;
  void OnProgress(int32_t progress) override;
  void OnLoadError(int32_t error_code, const std::string& description,
                   const std::string& failing_url) override;
  void OnConsoleMessage(const std::string& level,
                        const std::string& message) override;
  void OnNavigationPolicyDecide(const std::string& url) override;
  void OnResponsePolicyDecide(const std::string& url,
                              int32_t status_code) override;
  void OnUrlChanged(const std::string& url) override;
  void OnJavaScriptMessage(const std::string& channel,
                           const std::string& message) override;
  void OnJavaScriptAlertDialog(const std::string& message,
                               const std::string& url) override;
  void OnJavaScriptConfirmDialog(const std::string& message,
                                 const std::string& url) override;
  void OnJavaScriptPromptDialog(const std::string& message,
                                const std::string& default_text,
                                const std::string& url) override;

  std::unique_ptr<WebViewBackend> backend_;
  bool webview_created_ = false;
  flutter::TextureRegistrar* texture_registrar_;
  bool engine_policy_ = false;
  double width_ = 0.0;
  double height_ = 0.0;
  void* window_ = nullptr;
  BufferUnit* working_surface_ = nullptr;
  BufferUnit* candidate_surface_ = nullptr;
  BufferUnit* rendered_surface_ = nullptr;
  std::unique_ptr<FlMethodChannel> webview_channel_;
  std::unique_ptr<FlMethodChannel> webview_controller_channel_;
  std::unique_ptr<FlMethodChannel> navigation_delegate_channel_;
  std::unique_ptr<flutter::TextureVariant> texture_variant_;
  std::mutex mutex_;
  std::shared_ptr<BufferPool> tbm_pool_;
  // Guarded by mutex_.
  bool disposed_ = false;
  // Outlives this WebView so a pending async Dart reply can tell it apart
  // from a destroyed one.
  std::shared_ptr<bool> is_alive_ = std::make_shared<bool>(true);
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_H_
