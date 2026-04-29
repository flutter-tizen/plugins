// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_H_
#define FLUTTER_PLUGIN_WEBVIEW_H_

#include <EWebKit.h>
#include <Evas.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/texture_registrar.h>
#include <flutter_platform_view.h>

#include <cstddef>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "ewk_internal_api_binding.h"

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;

class BufferPool;
class BufferUnit;

class WebView : public PlatformView {
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

  // Used by NavigationRequestResult after a suspended navigation has been
  // accepted (Resume) or cancelled (Stop) by the Dart shouldOverrideUrlLoading
  // callback.
  void ResumeNavigation();
  void StopNavigation();

  Evas_Object* GetWebViewInstance() { return webview_instance_; }

  FlutterDesktopGpuSurfaceDescriptor* ObtainGpuSurface(size_t width,
                                                       size_t height);

  // Process-wide helpers used by static channels. They iterate live WebView
  // instances and rely on the shared EWK context.
  static void ClearAllCache();
  static bool ClearAllCookies();
  static std::string GetDefaultUserAgent();

 private:
  void HandleWebViewMethodCall(const FlMethodCall& method_call,
                               std::unique_ptr<FlMethodResult> result);
  void ApplyInitialParams(const flutter::EncodableValue& params);
  void ApplySettings(const flutter::EncodableMap& settings);

  template <typename T>
  void SetBackgroundColor(const T& color);

  std::string GetWebViewChannelName();

  bool InitWebView();

  static void OnFrameRendered(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadStarted(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadFinished(void* data, Evas_Object* obj, void* event_info);
  static void OnProgress(void* data, Evas_Object* obj, void* event_info);
  static void OnLoadError(void* data, Evas_Object* obj, void* event_info);
  static void OnConsoleMessage(void* data, Evas_Object* obj, void* event_info);
  static void OnNavigationPolicy(void* data, Evas_Object* obj,
                                 void* event_info);
  static void OnUrlChange(void* data, Evas_Object* obj, void* event_info);
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

  Evas_Object* webview_instance_ = nullptr;
  flutter::TextureRegistrar* texture_registrar_;
  double width_ = 0.0;
  double height_ = 0.0;
  double left_ = 0.0;
  double top_ = 0.0;
  void* window_ = nullptr;
  BufferUnit* working_surface_ = nullptr;
  BufferUnit* candidate_surface_ = nullptr;
  BufferUnit* rendered_surface_ = nullptr;
  bool has_navigation_delegate_ = false;
  std::unique_ptr<FlMethodChannel> webview_channel_;
  std::unique_ptr<flutter::TextureVariant> texture_variant_;
  std::mutex mutex_;
  std::unique_ptr<BufferPool> tbm_pool_;
  bool disposed_ = false;
  Ewk_Mouse_Button_Type mouse_button_type_ = (Ewk_Mouse_Button_Type)0;

  static std::set<WebView*> instances_;
  static std::mutex instances_mutex_;
  // Cached on the first WebView creation so static channel callers can read it
  // even after every InAppWebView has been disposed.
  static std::string default_user_agent_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_H_
