// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_H_

#include <dart_api_dl.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include <player.h>

#include <memory>
#include <string>
#include <vector>

#include "drm_manager.h"
#include "video_player_options.h"

typedef void (*FuncEcoreWl2WindowGeometryGet)(void *window, int *x, int *y,
                                              int *width, int *height);
typedef int (*FuncPlayerSetEcoreWlDisplay)(player_h player,
                                           player_display_type_e type,
                                           void *ecore_wl_window, int x, int y,
                                           int width, int height);

class VideoPlayer {
 public:
  using SeekCompletedCallback = std::function<void()>;

  explicit VideoPlayer(flutter::PluginRegistrar *plugin_registrar,
                       void *native_window);
  ~VideoPlayer();

  int64_t Create(const std::string &uri, int drm_type,
                 const std::string &license_server_url);
  void Dispose();

  void SetDisplayRoi(int32_t x, int32_t y, int32_t width, int32_t height);
  void Play();
  void Pause();
  void SetLooping(bool is_looping);
  void SetVolume(double volume);
  void SetPlaybackSpeed(double speed);
  void SeekTo(int32_t position, SeekCompletedCallback callback);
  int32_t GetPosition();

  void RegisterSendPort(Dart_Port send_port) { send_port_ = send_port; }

 private:
  bool SetDisplay();
  void SetUpEventChannel(flutter::BinaryMessenger *messenger);
  void Initialize();

  void SendInitialized();
  void SendBufferingStart();
  void SendBufferingUpdate(int32_t value);
  void SendBufferingEnd();
  void SendSubtitleUpdate(int32_t duration, const std::string &text);

  static void OnPrepared(void *data);
  static void OnBuffering(int percent, void *data);
  static void OnSeekCompleted(void *data);
  static void OnPlayCompleted(void *data);
  static void OnError(int error_code, void *data);
  static void OnInterrupted(player_interrupted_code_e code, void *data);
  static void OnSubtitleUpdated(unsigned long duration, char *text, void *data);

  std::vector<uint8_t> OnLicenseChallenge(
      const std::vector<uint8_t> &challenge);

  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;

  player_h player_ = nullptr;
  flutter::PluginRegistrar *plugin_registrar_;
  void *native_window_;
  int64_t player_id_ = -1;
  std::unique_ptr<DrmManager> drm_manager_;

  bool is_initialized_ = false;
  bool is_interrupted_ = false;
  bool is_buffering_ = false;

  SeekCompletedCallback on_seek_completed_;
  Dart_Port send_port_;
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_H_
