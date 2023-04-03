#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_TIZEN_PLUGIN_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_TIZEN_PLUGIN_H_

#include <flutter_plugin_registrar.h>

#include <functional>

#include "../src/dart_api/dart_api.h"

#ifdef FLUTTER_PLUGIN_IMPL
#define FLUTTER_PLUGIN_EXPORT __attribute__((visibility("default")))
#else
#define FLUTTER_PLUGIN_EXPORT
#endif

#if defined(__cplusplus)
extern "C" {
#endif

FLUTTER_PLUGIN_EXPORT void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);
// FFI native function
typedef std::function<void()> CallbackWrapper;
typedef intptr_t (*FuncLicenseCB)(uint8_t *challenge_data, size_t challenge_len,
                                  int64_t player_id);
FuncLicenseCB challenge_cb_ = nullptr;
Dart_Port send_port_;
FLUTTER_PLUGIN_EXPORT intptr_t InitDartApiDL(void *data);
FLUTTER_PLUGIN_EXPORT void RegisterSendPort(Dart_Port send_port);
intptr_t ChallengeCb(uint8_t *challenge_data, size_t challenge_len,
                     int64_t player_id);
#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_TIZEN_PLUGIN_H_
