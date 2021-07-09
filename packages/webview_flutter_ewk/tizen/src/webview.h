// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_EWK_WEVIEW_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_EWK_WEVIEW_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <flutter_platform_view.h>
#include <tbm_surface.h>

#include "EWebKit_internal.h"
#include "EWebKit_product.h"

class WebView : public PlatformView {
 public:
  WebView(flutter::PluginRegistrar* registrar, int viewId,
          flutter::TextureRegistrar* textureRegistrar, double width,
          double height, flutter::EncodableMap& params, void* winHandle);
  ~WebView();
  virtual void Dispose() override;
  virtual void Resize(double width, double height) override;
  virtual void Touch(int type, int button, double x, double y, double dx,
                     double dy) override;
  virtual void SetDirection(int direction) override;
  virtual void ClearFocus() override;

  // Key input event
  virtual void DispatchKeyDownEvent(Ecore_Event_Key* key) {}
  virtual void DispatchKeyUpEvent(Ecore_Event_Key* key) {}
  virtual void DispatchCompositionUpdateEvent(const char* str, int size) {}
  virtual void DispatchCompositionEndEvent(const char* str, int size) {}
  virtual void SetSoftwareKeyboardContext(Ecore_IMF_Context* context) {}

  Evas_Object* GetWebViewInstance() { return webview_instance_; }

  FlutterDesktopGpuBuffer* ObtainGpuBuffer(size_t width, size_t height);
  void DestructBuffer(void* buffer);

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void HandleCookieMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  std::string GetChannelName();
  void InitWebView();

  void RegisterJavaScriptChannelName(const std::string& name);
  void ApplySettings(flutter::EncodableMap);

  flutter::TextureRegistrar* texture_registrar_;

  static void OnFrameRendered(void* data, Evas_Object*, void* buffer);
  static void OnLoadStarted(void* data, Evas_Object*, void*);
  static void OnLoadInProgress(void* data, Evas_Object*, void*);
  static void OnLoadFinished(void* data, Evas_Object*, void*);
  static void OnLoadError(void* data, Evas_Object*, void* rawError);
  static void OnUrlChanged(void* data, Evas_Object*, void* newUrl);
  static void OnConsoleMessage(void*, Evas_Object*, void* eventInfo);
  static void OnEvaluateJavaScript(Evas_Object* o, const char* result,
                                   void* data);
  static void OnJavaScriptMessage(Evas_Object* o, Ewk_Script_Message message);
  static Eina_Bool OnJavaScriptAlert(Evas_Object* o, const char* alert_text,
                                     void*);
  static Eina_Bool OnJavaScriptConfirm(Evas_Object* o, const char* message,
                                       void*);
  static Eina_Bool OnJavaScriptPrompt(Evas_Object* o, const char* message,
                                      const char* default_value, void*);

  Evas_Object* webview_instance_;

  double width_;
  double height_;
  tbm_surface_h candidate_surface_;
  tbm_surface_h rendered_surface_;
  bool has_navigation_delegate_;
  bool has_progress_tracking_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;

  flutter::TextureVariant* texture_variant_;
  FlutterDesktopGpuBuffer* gpu_buffer_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_EWK_WEVIEW_H_
