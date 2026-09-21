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

#include "ewk_internal_api_binding.h"
#include "webview_backend.h"

typedef struct _Ecore_Evas Ecore_Evas;

class EwkWebViewBackend : public WebViewBackend {
 public:
  explicit EwkWebViewBackend(Delegate* delegate);
  ~EwkWebViewBackend() override;

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

  // ewk_init()/ewk_shutdown() are process-wide and must bracket every view.
  static void GlobalInitialize();
  static void GlobalShutdown();

 private:
  static Ecore_Evas* GetOffscreenHost();

  // Detaches every callback from the view and hands ownership of the raw
  // Evas_Object back to the caller, which must delete it. Returns nullptr if
  // the view was already detached.
  Evas_Object* DetachView();

  static void OnFrameRendered(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadStarted(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadFinished(void* data, Evas_Object* obj, void* event_info);
  static void OnProgress(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadError(void* data, Evas_Object* obj, void* event_info);
  static void OnConsoleMessage(void* data, Evas_Object* obj, void* event_info);
  static void OnNavigationPolicy(void* data, Evas_Object* obj,
                                 void* event_info);
  static void OnUrlChange(void* data, Evas_Object* obj, void* event_info);
  static void OnTitleChange(void* data, Evas_Object* obj, void* event_info);
  static void OnEvaluateJavaScript(Evas_Object* obj, const char* result_value,
                                   void* user_data);
  static Eina_Bool OnJavaScriptAlertDialog(Evas_Object* o, const char* message,
                                           void* data);
  static Eina_Bool OnJavaScriptConfirmDialog(Evas_Object* o,
                                             const char* message, void* data);
  static Eina_Bool OnJavaScriptPromptDialog(Evas_Object* o, const char* message,
                                            const char* default_text,
                                            void* data);

  void SendTouchEvent(int type, double x, double y);
  void SendMouseEvent(int type, int button, double x, double y, double dx,
                      double dy);

  Delegate* delegate_;
  Evas_Object* view_ = nullptr;
  double left_ = 0.0;
  double top_ = 0.0;
  Ewk_Mouse_Button_Type mouse_button_type_ = (Ewk_Mouse_Button_Type)0;
};

#endif  // FLUTTER_PLUGIN_EWK_WEBVIEW_BACKEND_H_
