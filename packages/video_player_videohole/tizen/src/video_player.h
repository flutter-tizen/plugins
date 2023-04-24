// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIDEO_PLAYER_VIDEOHOLE_PLUGIN_VIDEO_PLAYER_H_
#define VIDEO_PLAYER_VIDEOHOLE_PLUGIN_VIDEO_PLAYER_H_

#include <dart_api_dl.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>
#include <player.h>

#include <memory>
#include <string>
#include <vector>

#include "drm_manager.h"
#include "messages.h"
#include "video_player_options.h"

using SeekCompletedCb = std::function<void()>;

typedef void (*FuncEcoreWl2WindowGeometryGet)(void *window, int *x, int *y,
                                              int *width, int *height);
typedef int (*FuncPlayerSetEcoreWlDisplay)(player_h player,
                                           player_display_type_e type,
                                           void *ecore_wl_window, int x, int y,
                                           int width, int height);

class VideoPlayer {
 public:
  VideoPlayer(flutter::PluginRegistrar *plugin_registrar, void *native_window,
              const CreateMessage &create_message);
  ~VideoPlayer();

  int64_t Create();
  void Dispose();
  int GetPosition();
  void Play();
  void Pause();
  void SetDisplayRoi(int x, int y, int w, int h);
  void SetLooping(bool is_looping);
  void SetPlaybackSpeed(double speed);
  void SeekTo(int position);
  void SetVolume(double volume);

  void RegisterSendPort(Dart_Port send_port) { send_port_ = send_port; }

 private:
  void Initialize();
  void SetupEventChannel(flutter::BinaryMessenger *messenger);
  void SendInitialized();
  void SendBufferingStart();
  void SendBufferingUpdate(int position);
  void SendBufferingEnd();
  void SendSubtitleUpdate(int duration, char *text);
  bool Open(const std::string &uri);
  void ParseCreateMessage(const CreateMessage &create_message);
  bool SetDisplay();

  static void OnPrepared(void *data);
  static void OnBuffering(int percent, void *data);
  static void OnSeekCompleted(void *data);
  static void OnPlayCompleted(void *data);
  static void OnError(int error_code, void *user_data);
  static void onInterrupted(player_interrupted_code_e code, void *data);
  static void OnSubtitleUpdated(unsigned long duration, char *text,
                                void *user_data);

  std::vector<uint8_t> OnLicenseChallenge(
      const std::vector<uint8_t> &challenge);

  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
  SeekCompletedCb on_seek_completed_;
  player_h player_;
  flutter::PluginRegistrar *plugin_registrar_;
  void *native_window_;
  std::unique_ptr<DrmManager> drm_manager_;
  std::string uri_;
  std::string license_url_;
  int64_t player_id_ = -1;
  int drm_type_ = DRM_TYPE_NONE;
  bool is_initialized_ = false;
  bool is_interrupted_ = false;
  bool is_buffering_ = false;

  Dart_Port send_port_;
};

#endif  // VIDEO_PLAYER_VIDEOHOLE_PLUGIN_VIDEO_PLAYER_H_
