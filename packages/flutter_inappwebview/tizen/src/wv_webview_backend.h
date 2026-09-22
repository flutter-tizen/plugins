// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WV_WEBVIEW_BACKEND_H_
#define FLUTTER_PLUGIN_WV_WEBVIEW_BACKEND_H_

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "webview_backend.h"
#include "wv_internal_api_binding.h"

class WvWebViewBackend : public WebViewBackend {
 public:
  explicit WvWebViewBackend(Delegate* delegate);
  ~WvWebViewBackend() override;

  bool Create(double width, double height, void* window) override;
  std::function<void()> PrepareTeardown(
      std::shared_ptr<BufferPool> pool) override;

  void Offset(double left, double top) override;
  void Resize(double width, double height) override;
  void Touch(int type, int button, double x, double y, double dx,
             double dy) override;
  bool SendKey(const char* key, const char* string, const char* compose,
               uint32_t modifiers, uint32_t scan_code, bool is_down) override;
  void Resume() override;
  void Stop() override;

  void SetJavaScriptEnabled(bool enabled) override;

  bool LoadUrl(const std::string& url) override;
  bool LoadUrlRequest(const std::string& url, int32_t method,
                      const std::map<std::string, std::string>& headers,
                      const std::vector<uint8_t>& body) override;
  bool LoadHtmlString(const std::string& html,
                      const std::string& base_url) override;
  bool CanGoBack() override;
  bool CanGoForward() override;
  bool GoBack() override;
  bool GoForward() override;
  bool Reload() override;
  std::string GetCurrentUrl() override;
  void EvaluateJavaScript(
      const std::string& javascript,
      std::function<void(bool success, const char* result_value)> callback)
      override;
  void ClearCache() override;
  std::string GetTitle() override;
  void ScrollTo(int32_t x, int32_t y) override;
  void GetScrollPosition(int32_t* x, int32_t* y) override;
  void SetBackgroundColor(int r, int g, int b, int a) override;
  void SetUserAgent(const std::string& user_agent) override;
  std::string GetUserAgent() override;
  void EnableZoom(bool enabled) override;
  void JavaScriptAlertReply() override;
  void JavaScriptConfirmReply(bool result) override;
  void JavaScriptPromptReply(const char* result) override;
  bool ClearCookies() override;

  void Suspend() override;
  int32_t GetProgress() override;
  double GetScale() override;
  void SetScale(double scale, int32_t x, int32_t y) override;

  static bool GlobalInitialize(bool standalone);

  static void GlobalShutdown();

 private:
  wv_view_h DetachView();

  static void OnFrameRendered(wv_view_h obj, void* event_info, void* user_data);
  static void OnLoadStarted(wv_view_h obj, void* event_info, void* user_data);
  static void OnLoadFinished(wv_view_h obj, void* event_info, void* user_data);
  static void OnProgress(wv_view_h obj, void* event_info, void* user_data);
  static void OnLoadError(wv_view_h obj, void* event_info, void* user_data);
  static void OnConsoleMessage(wv_view_h obj, void* event_info,
                               void* user_data);
  static void OnNavigationPolicy(wv_view_h obj, void* event_info,
                                 void* user_data);
  static void OnUrlChange(wv_view_h obj, void* event_info, void* user_data);
  static void OnTitleChange(wv_view_h obj, void* event_info, void* user_data);
  static void OnEvaluateJavaScript(wv_view_h obj, const char* result_value,
                                   void* user_data);
  static bool OnJavaScriptAlertDialog(wv_view_h view, const char* message,
                                      void* data);
  static bool OnJavaScriptConfirmDialog(wv_view_h view, const char* message,
                                        void* data);
  static bool OnJavaScriptPromptDialog(wv_view_h view, const char* message,
                                       const char* default_text, void* data);

  void SendTouchEvent(int type, double x, double y);
  void SendMouseEvent(int type, int button, double x, double y, double dx,
                      double dy);

  Delegate* delegate_;
  wv_view_h view_ = nullptr;
  wv_mouse_button_type_e mouse_button_type_ =
      static_cast<wv_mouse_button_type_e>(0);
};

#endif  // FLUTTER_PLUGIN_WV_WEBVIEW_BACKEND_H_
