// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEVIEW_H_
#define FLUTTER_PLUGIN_WEVIEW_H_

#include <Ecore.h>
#include <Ecore_Evas.h>
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
#include <string>

#include "chromium-ewk/EWebKit_internal.h"
#include "chromium-ewk/EWebKit_product.h"

class BufferPool;
class BufferUnit;

class WebView : public PlatformView {
 public:
  WebView(flutter::PluginRegistrar* registrar, int view_id,
          flutter::TextureRegistrar* texture_registrar, double width,
          double height, const flutter::EncodableValue& params);
  ~WebView();

  virtual void Dispose() override;

  virtual void Resize(double width, double height) override;
  virtual void Touch(int type, int button, double x, double y, double dx,
                     double dy) override;
  virtual void SetDirection(int direction) override;

  virtual void ClearFocus() override {}

  virtual bool SendKey(const char* key, const char* string, const char* compose,
                       uint32_t modifiers, uint32_t scan_code,
                       bool is_down) override;

  Evas_Object* GetWebViewInstance() { return webview_instance_; }

  FlutterDesktopGpuBuffer* ObtainGpuBuffer(size_t width, size_t height);

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void HandleCookieMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  void ApplySettings(const flutter::EncodableMap& settings);
  void RegisterJavaScriptChannelName(const std::string& name);
  std::string GetChannelName();

  void InitWebView();

  static void OnFrameRendered(void* data, Evas_Object*, void* buffer);
  static void OnLoadStarted(void* data, Evas_Object*, void*);
  static void OnLoadInProgress(void* data, Evas_Object*, void*);
  static void OnLoadFinished(void* data, Evas_Object*, void*);
  static void OnLoadError(void* data, Evas_Object*, void* rawError);
  static void OnUrlChanged(void* data, Evas_Object*, void* newUrl);
  static void OnConsoleMessage(void*, Evas_Object*, void* eventInfo);
  static void OnNavigationPolicy(void*, Evas_Object*, void* eventInfo);
  static void OnEvaluateJavaScript(Evas_Object* o, const char* result,
                                   void* data);
  static void OnJavaScriptMessage(Evas_Object* o, Ewk_Script_Message message);
  static Eina_Bool OnJavaScriptAlert(Evas_Object* o, const char* alert_text,
                                     void*);
  static Eina_Bool OnJavaScriptConfirm(Evas_Object* o, const char* message,
                                       void*);
  static Eina_Bool OnJavaScriptPrompt(Evas_Object* o, const char* message,
                                      const char* default_value, void*);

  Evas_Object* webview_instance_ = nullptr;
  flutter::TextureRegistrar* texture_registrar_;
  double width_;
  double height_;
  BufferUnit* working_surface_ = nullptr;
  BufferUnit* candidate_surface_ = nullptr;
  BufferUnit* rendered_surface_ = nullptr;
  bool has_navigation_delegate_ = false;
  bool has_progress_tracking_ = false;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
  std::unique_ptr<flutter::TextureVariant> texture_variant_;
  std::mutex mutex_;
  std::unique_ptr<BufferPool> tbm_pool_;
};

#endif  // FLUTTER_PLUGIN_WEVIEW_H_
