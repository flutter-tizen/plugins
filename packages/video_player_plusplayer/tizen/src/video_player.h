// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_H_

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <memory>
#include <string>

#include "drm_manager.h"
#include "messages.h"
#include "plus_player_proxy.h"
#include "video_player_options.h"

typedef enum {
  PLUS_PLAYER_STATE_NONE,    /**< Player is not created */
  PLUS_PLAYER_STATE_IDLE,    /**< Player is created, but not prepared */
  PLUS_PLAYER_STATE_READY,   /**< Player is ready to play media */
  PLUS_PLAYER_STATE_PLAYING, /**< Player is playing media */
  PLUS_PLAYER_STATE_PAUSED,  /**< Player is paused while playing media */
} plusplayer_state_e;

using SeekCompletedCb = std::function<void()>;

class VideoPlayer {
 public:
  VideoPlayer(FlutterDesktopPluginRegistrarRef registrar_ref,
              const CreateMessage &create_message);
  ~VideoPlayer();

  int64_t Create();
  void Dispose();
  int GetPosition();
  void Play();
  void Pause();
  bool SetBufferingConfig(const std::string &option, int amount);
  void SetDisplayRoi(int x, int y, int w, int h);
  void SetLooping(bool is_looping);
  void SetPlaybackSpeed(double speed);
  void SeekTo(int position);      // milliseconds
  void SetVolume(double volume);  // milliseconds

 private:
  void Initialize();
  void SetupEventChannel(flutter::BinaryMessenger *messenger);
  void SendInitialized();
  void SendBufferingStart();
  void SendBufferingUpdate(int position);  // milliseconds
  void SendBufferingEnd();
  void SendSeeking(bool seeking);
  std::string GetApplicationId();
  bool Open(const std::string &uri);
  void ParseCreateMessage(const CreateMessage &create_message);
  void RegisterListener();
  bool SetDisplay(FlutterDesktopPluginRegistrarRef registrar_ref);

  static void OnPrepared(bool ret, void *data);
  static void OnBuffering(int percent, void *data);
  static void OnSeekCompleted(void *data);
  static void OnPlayCompleted(void *data);
  static void OnPlaying(void *data);
  static void OnError(const plusplayer::ErrorType &error_code, void *user_data);
  static void OnErrorMessage(const plusplayer::ErrorType &error_code,
                             const char *error_msg, void *user_data);
  static void OnPlayerAdaptiveStreamingControl(
      const plusplayer::StreamingMessageType &type,
      const plusplayer::MessageParam &msg, void *user_data);
  static void OnDrmInitData(int *drmhandle, unsigned int len,
                            unsigned char *psshdata, plusplayer::TrackType type,
                            void *user_data);
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
  PlusplayerRef plusplayer_{nullptr};
  PlusplayerListener listener_;
  FlutterDesktopPluginRegistrarRef registrar_ref_{nullptr};
  std::unique_ptr<DrmManager> drm_manager_;
  std::string uri_;
  std::string license_url_;
  int64_t player_id_{-1};
  int drm_type_{DRM_TYPE_NONE};
  bool is_initialized_{false};
  bool is_buffering_{false};
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_H_
