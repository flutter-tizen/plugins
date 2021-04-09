#ifndef FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_GOOGLE_MAP_CONTROLLER_H_
#define FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_GOOGLE_MAP_CONTROLLER_H_

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

// class TextInputChannel;

class GoogleMapController : public PlatformView {
 public:
  GoogleMapController(flutter::PluginRegistrar* registrar, int viewId,
                      FlutterTextureRegistrar* textureRegistrar, double width,
                      double height, flutter::EncodableMap& params);
  ~GoogleMapController();

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

 private:
  void InitWebView();
  std::string GetChannelName(int view_id);
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  FlutterTextureRegistrar* texture_registrar_;
  LWE::WebContainer* webview_instance_;
  double width_;
  double height_;
  tbm_surface_h tbm_surface_;
  bool is_mouse_lbutton_down_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
};

#endif  // FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_GOOGLE_MAP_CONTROLLER_H_
