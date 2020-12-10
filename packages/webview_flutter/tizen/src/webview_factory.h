#ifndef FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_FACTORY_H_
#define FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_FACTORY_H_

#include "webview.h"
class WebViewFactory : public PlatformViewFactory {
 public:
  WebViewFactory(flutter::PluginRegistrar* registrar,
                 FlutterTextureRegistrar* textureRegistrar);
  virtual void dispose() override ;
  virtual PlatformView* create(
      int viewId, double width, double height,
      const std::vector<uint8_t>& createParams) override;

 private:
  FlutterTextureRegistrar* textureRegistrar_;
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_FLUTTER_TIZEN_WEVIEW_FACTORY_H_
