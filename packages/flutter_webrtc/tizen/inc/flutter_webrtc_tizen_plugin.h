#ifndef FLUTTER_PLUGIN_FLUTTER_WEBRTC_TIZEN_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_WEBRTC_TIZEN_PLUGIN_H_

#include <flutter_plugin_registrar.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>

#include "flutter_data_channel.h"
#include "flutter_media_stream.h"
#include "flutter_peerconnection.h"
#include "flutter_video_renderer.h"
#include "libwebrtc.h"

#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define FLUTTER_PLUGIN_EXPORT
#endif

#if defined(__cplusplus)
extern "C" {
#endif

FLUTTER_PLUGIN_EXPORT void FlutterWebRtcTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  

namespace flutter_webrtc_plugin {
using namespace libwebrtc;

class FlutterWebRTCPlugin : public flutter::Plugin {
 public:
  virtual flutter::BinaryMessenger *messenger() = 0;

  virtual flutter::TextureRegistrar *textures() = 0;
};

class FlutterWebRTC : public FlutterWebRTCBase,
                      public FlutterVideoRendererManager,
                      public FlutterMediaStream,
                      public FlutterPeerConnection,
                      public FlutterDataChannel {
 public:
  FlutterWebRTC(FlutterWebRTCPlugin *plugin);
  virtual ~FlutterWebRTC();

  void HandleMethodCall(
      const flutter::MethodCall<EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<EncodableValue>> result);
};

}  // namespace flutter_webrtc_plugin
