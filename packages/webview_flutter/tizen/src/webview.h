#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_

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
          double height, const std::string initialUrl);
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

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  std::string GetChannelName();
  const std::string& GetCurrentUrl() { return currentUrl_; }
  void InitWebView();
  FlutterTextureRegistrar* textureRegistrar_;
  LWE::WebContainer* webViewInstance_;
  std::string currentUrl_;
  double width_;
  double height_;
  tbm_surface_h tbmSurface_;
  bool isMouseLButtonDown_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
