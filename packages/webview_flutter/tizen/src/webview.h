// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <flutter_platform_view.h>
#include <flutter_tizen_texture_registrar.h>
#include <tbm_surface.h>

namespace LWE {
class WebContainer;
}

class TextInputChannel;

class WebView : public PlatformView {
 public:
  WebView(flutter::PluginRegistrar* registrar, int viewId,
          FlutterTextureRegistrar* textureRegistrar, double width,
          double height, flutter::EncodableMap& params);
  ~WebView();
  virtual void Dispose() override;
  virtual void Resize(double width, double height) override;
  virtual void Touch(int type, int button, double x, double y, double dx,
                     double dy) override;
  virtual void SetDirection(int direction) override;
  virtual void ClearFocus() override;

  // Key input event
  virtual void DispatchKeyDownEvent(Ecore_Event_Key* key) override;
  virtual void DispatchKeyUpEvent(Ecore_Event_Key* key) override;
  virtual void DispatchCompositionUpdateEvent(const char* str,
                                              int size) override;
  virtual void DispatchCompositionEndEvent(const char* str, int size) override;

  virtual void SetSoftwareKeyboardContext(Ecore_IMF_Context* context) override;

  LWE::WebContainer* GetWebViewInstance() { return webview_instance_; }

  void HidePanel();
  void ShowPanel();

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void HandleCookieMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  std::string GetChannelName();
  void InitWebView();
  void InitFlutterChannels();
  void InitWebViewCallbacks();

  void ReinitializeWebView();

  // web view properties
  std::string webview_url_;
  flutter::EncodableMap webview_settings_;
  flutter::EncodableList webview_javascript_channel_names_list_;
  std::optional<std::string> webview_user_agent_;

  void RegisterJavaScriptChannelName(const std::string& name);
  void ApplySettings(flutter::EncodableMap);

  FlutterTextureRegistrar* texture_registrar_;
  LWE::WebContainer* webview_instance_;
  double width_;
  double height_;
  tbm_surface_h tbm_surface_;
  bool is_mouse_lbutton_down_;
  bool has_navigation_delegate_;
  bool has_progress_tracking_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
  Ecore_IMF_Context* context_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
