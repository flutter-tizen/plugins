#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_

#include <flutter_platform_view.h>
#include <flutter_plugin_registrar.h>
#include <tbm_surface.h>

// #include "lwe/LWEWebView.h"
// #include "lwe/PlatformIntegrationData.h"
namespace LWE {
class WebContainer;
}

class WebView : public PlatformView {
 public:
  WebView(flutter::PluginRegistrar* registrar, int viewId,
          FlutterTextureRegistrar* textureRegistrar, double width,
          double height, const std::string initialUrl);
  ~WebView();
  virtual void dispose() override;
  virtual void resize(double width, double height) override;
  virtual void touch(int type, int button, double x, double y, double dx,
                     double dy) override;
  virtual void setDirection(int direction) override;
  virtual void clearFocus() override;

  // Key input event
  virtual void dispatchKeyDownEvent(Ecore_Event_Key* key) override;
  virtual void dispatchKeyUpEvent(Ecore_Event_Key* key) override;

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  std::string getChannelName();
  const std::string& getCurrentUrl() { return currentUrl_; }
  void initWebView();
  FlutterTextureRegistrar* textureRegistrar_;
  LWE::WebContainer* webViewInstance_;
  std::string currentUrl_;
  double width_;
  double height_;
  tbm_surface_h tbmSurface_;
  bool isMouseLButtonDown_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_H_
