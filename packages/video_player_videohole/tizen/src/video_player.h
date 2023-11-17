// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_H_

#include <Ecore.h>
#include <dart_api_dl.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include <player.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "drm_manager.h"
#include "video_player_options.h"

#define MAX_STRING_NAME_LEN 255
#define MMPLAYER_FOUR_CC_LEN 14
#define PLAYER_LANG_NAME_SIZE 10

typedef struct {
  char fourCC[MMPLAYER_FOUR_CC_LEN + 1]; /**< codec fourcc */
  char name[MAX_STRING_NAME_LEN]; /**< name: video/audio, it maybe not exit in
                                     some track*/
  /*dynamic infos in hls,ss,dash streams*/
  int width;    /**< resolution width */
  int height;   /**< resolution height */
  int bit_rate; /**< bitrate in bps */
} player_video_track_info_v2;

typedef struct {
  char fourCC[MMPLAYER_FOUR_CC_LEN + 1]; /**< codec fourcc */
  char language[PLAYER_LANG_NAME_SIZE];  /**< language info*/
  /*dynamic infos in hls,ss,dash streams*/
  int sample_rate; /**< sample rate in this track*/
  int channel;     /**< channel in this track*/
  int bit_rate;    /**< bitrate  in this track*/
} player_audio_track_info_v2;

typedef struct {
  char fourCC[MMPLAYER_FOUR_CC_LEN + 1]; /**< codec fourcc */
  char language[PLAYER_LANG_NAME_SIZE];  /**< language info*/
  int subtitle_type; /**< text subtitle = 0, picture subtitle = 1 */
} player_subtitle_track_info_v2;

typedef void (*FuncEcoreWl2WindowGeometryGet)(void *window, int *x, int *y,
                                              int *width, int *height);
typedef int (*FuncPlayerSetEcoreWlDisplay)(player_h player,
                                           player_display_type_e type,
                                           void *ecore_wl_window, int x, int y,
                                           int width, int height);
typedef int (*FuncPlayerGetTrackCountV2)(player_h player,
                                         player_stream_type_e type,
                                         int *pcount);
typedef int (*FuncPlayerGetVideoTrackInfoV2)(
    player_h player, int index, player_video_track_info_v2 **track_info);
typedef int (*FuncPlayerGetAudioTrackInfoV2)(
    player_h player, int index, player_audio_track_info_v2 **track_info);
typedef int (*FuncPlayerGetSubtitleTrackInfoV2)(
    player_h player, int index, player_subtitle_track_info_v2 **track_info);

class VideoPlayer {
 public:
  using SeekCompletedCallback = std::function<void()>;

  explicit VideoPlayer(flutter::PluginRegistrar *plugin_registrar,
                       void *native_window);
  ~VideoPlayer();

  int64_t Create(const std::string &uri, int drm_type,
                 const std::string &license_server_url,
                 flutter::EncodableMap &http_headers);
  void Dispose();

  void SetDisplayRoi(int32_t x, int32_t y, int32_t width, int32_t height);
  void Play();
  void Pause();
  void SetLooping(bool is_looping);
  void SetVolume(double volume);
  void SetPlaybackSpeed(double speed);
  void SeekTo(int32_t position, SeekCompletedCallback callback);
  int32_t GetPosition();
  flutter::EncodableList getTrackInfo(int32_t track_type);
  void SetTrackSelection(int32_t track_id, int32_t track_type);

  void RegisterSendPort(Dart_Port send_port) { send_port_ = send_port; }

 private:
  void SendPendingEvents();
  void PushEvent(const flutter::EncodableValue &encodable_value);
  void SendError(const std::string &error_code,
                 const std::string &error_message);
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

  Ecore_Pipe *sink_event_pipe_ = nullptr;
  std::mutex queue_mutex_;
  std::queue<flutter::EncodableValue> encodable_event_queue_;
  std::queue<std::pair<std::string, std::string>> error_event_queue_;
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_H_
