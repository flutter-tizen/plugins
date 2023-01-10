#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_TIZEN_PLUGIN_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_TIZEN_PLUGIN_H_

#include <../../../../../dart-sdk/include/dart_api.h>
#include <flutter_plugin_registrar.h>

#include <functional>

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
typedef int (*FuncLicenseCB)(uint8_t *challenge_data, uint64_t challenge_len);
FuncLicenseCB challenge_cb_ = nullptr;
Dart_Port send_port_;
FLUTTER_PLUGIN_EXPORT void GetChallengeData(FuncLicenseCB callback);
FLUTTER_PLUGIN_EXPORT void SetLicense(unsigned char *response_data,
                                      unsigned long response_len);
FLUTTER_PLUGIN_EXPORT intptr_t InitDartApiDL(void *data);
FLUTTER_PLUGIN_EXPORT void RegisterSendPort(Dart_Port send_port);
FLUTTER_PLUGIN_EXPORT void ExecuteCallback(CallbackWrapper *wrapper_ptr);
void NotifyDart(CallbackWrapper *wrapper);
int GetChallengeCb(uint8_t *challenge_data, uint64_t challenge_len);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_TIZEN_PLUGIN_H_
