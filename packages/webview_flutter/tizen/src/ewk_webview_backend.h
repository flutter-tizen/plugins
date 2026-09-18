// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_EWK_WEBVIEW_BACKEND_H_
#define FLUTTER_PLUGIN_EWK_WEBVIEW_BACKEND_H_

#include <EWebKit.h>
#include <Evas.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ftpw_webview_flutter.h"
#include "webview_backend.h"

typedef struct _Ecore_Evas Ecore_Evas;

class EwkWebViewBackend : public WebViewBackend {
 public:
  explicit EwkWebViewBackend(Delegate* delegate);
  ~EwkWebViewBackend() override = default;

  bool Create(double width, double height, void* window,
              bool engine_policy) override;
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
  void SetHasNavigationDelegate(bool has_navigation_delegate) override;

  void LoadUrl(const std::string& url) override;
  bool LoadUrlRequest(const std::string& url, int32_t method,
                      const std::map<std::string, std::string>& headers,
                      const std::vector<uint8_t>& body) override;
  void LoadHtmlString(const std::string& html,
                      const std::string& base_url) override;
  bool CanGoBack() override;
  bool CanGoForward() override;
  void GoBack() override;
  void GoForward() override;
  void Reload() override;
  std::string GetCurrentUrl() override;
  void EvaluateJavaScript(
      const std::string& javascript,
      std::function<void(bool success, const char* result_value)> callback)
      override;
  void RegisterJavaScriptChannel(const std::string& name) override;
  void ClearCache() override;
  void ClearLocalStorage() override;
  std::string GetTitle() override;
  void ScrollTo(int32_t x, int32_t y) override;
  void ScrollBy(int32_t x, int32_t y) override;
  void GetScrollPosition(int32_t* x, int32_t* y) override;
  void SetBackgroundColor(int r, int g, int b, int a) override;
  void SetUserAgent(const std::string& user_agent) override;
  std::string GetUserAgent() override;
  void EnableZoom(bool enabled) override;
  void JavaScriptAlertReply() override;
  void JavaScriptConfirmReply(bool result) override;
  void JavaScriptPromptReply(const std::string& result) override;
  void SetScrollbarVisible(bool visible) override;
  bool ClearCookies() override;

  static void GlobalInitialize();

  static void GlobalShutdown();

 private:
  static Ecore_Evas* GetOffscreenHost();
  static void FreeOffscreenHost();

  static void OnFrameRendered(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadStarted(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadFinished(void* data, Evas_Object* obj, void* event_info);
  static void OnProgress(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadError(void* data, Evas_Object* obj, void* event_info);
  static void OnConsoleMessage(void* data, Evas_Object* obj, void* event_info);
  static void OnNavigationPolicy(void* data, Evas_Object* obj,
                                 void* event_info);
  static void OnResponsePolicy(void* data, Evas_Object* obj, void* event_info);
  static void OnUrlChange(void* data, Evas_Object* obj, void* event_info);
  static void OnEvaluateJavaScript(Evas_Object* obj, const char* result_value,
                                   void* user_data);
  static void OnJavaScriptMessage(Evas_Object* obj, Ewk_Script_Message message);
  static Eina_Bool OnJavaScriptAlertDialog(void* o, const char* message,
                                           void* data);
  static Eina_Bool OnJavaScriptConfirmDialog(void* o, const char* message,
                                             void* data);
  static Eina_Bool OnJavaScriptPromptDialog(void* o, const char* message,
                                            const char* default_text,
                                            void* data);

  void SendTouchEvent(int type, double x, double y);
  void SendMouseEvent(int type, int button, double x, double y, double dx,
                      double dy);

  Delegate* delegate_;
  Evas_Object* view_ = nullptr;
  void* window_ = nullptr;
  double left_ = 0.0;
  double top_ = 0.0;
  bool has_navigation_delegate_ = false;
  bool scrollbar_enabled_ = true;
  Ewk_Mouse_Button_Type mouse_button_type_ = (Ewk_Mouse_Button_Type)0;
};

#endif  // FLUTTER_PLUGIN_EWK_WEBVIEW_BACKEND_H_
