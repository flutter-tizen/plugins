#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <flutter_platform_view.h>
#include <flutter_plugin_registrar.h>
#include <flutter_texture_registrar.h>
#include <tbm_surface.h>

// #include "lwe/LWEWebView.h"
// #include "lwe/PlatformIntegrationData.h"
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

  virtual void SetSoftwareKeyboardContext(Ecore_IMF_Context* context) override;

  LWE::WebContainer* GetWebViewInstance() { return webViewInstance_; }

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

  FlutterTextureRegistrar* textureRegistrar_;
  LWE::WebContainer* webViewInstance_;
  double width_;
  double height_;
  tbm_surface_h tbmSurface_;
  bool isMouseLButtonDown_;
  bool hasNavigationDelegate_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
