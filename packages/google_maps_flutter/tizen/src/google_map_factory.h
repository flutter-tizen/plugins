#ifndef FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_GOOGLE_MAP_FACTORY_H_
#define FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_GOOGLE_MAP_FACTORY_H_

#include "google_map_controller.h"

class GoogleMapFactory : public PlatformViewFactory {
 public:
  GoogleMapFactory(flutter::PluginRegistrar* registrar,
                   FlutterTextureRegistrar* textureRegistrar);
  virtual void Dispose() override;
  virtual PlatformView* Create(
      int viewId, double width, double height,
      const std::vector<uint8_t>& createParams) override;

 private:
  FlutterTextureRegistrar* textureRegistrar_;
};

#endif  // FLUTTER_PLUGIN_GOOGLE_MAPS_FLUTTER_TIZEN_GOOGLE_MAP_FACTORY_H_
