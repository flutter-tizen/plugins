// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PLUS_PLAYER_H_
#define FLUTTER_PLUGIN_PLUS_PLAYER_H_

#include <flutter/plugin_registrar.h>

#include <memory>
#include <string>

#include "drm_manager.h"
#include "plusplayer/plusplayer.h"
#include "plusplayer/track.h"
#include "video_player.h"

class PlusPlayer : public VideoPlayer, public plusplayer::EventListener {
 public:
  explicit PlusPlayer(flutter::BinaryMessenger *messenger, void *native_window,
                      std::string &video_format);
  ~PlusPlayer();

  int64_t Create(const std::string &uri, int drm_type,
                 const std::string &license_server_url, bool is_prebuffer_mode,
                 flutter::EncodableMap &http_headers) override;
  void Dispose() override;

  void SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                     int32_t height) override;
  bool Play() override;
  bool Deactivate() override;
  bool Activate() override;
  bool Pause() override;
  bool SetLooping(bool is_looping) override;
  bool SetVolume(double volume) override;
  bool SetPlaybackSpeed(double speed) override;
  bool SeekTo(int64_t position, SeekCompletedCallback callback) override;
  int64_t GetPosition() override;
  int64_t GetDuration() override;
  void GetVideoSize(int32_t *width, int32_t *height) override;
  bool IsReady() override;
  flutter::EncodableList GetTrackInfo(std::string track_type) override;
  bool SetTrackSelection(int32_t track_id, std::string track_type) override;

 private:
  bool SetDisplay();
  bool SetDrm(const std::string &uri, int drm_type,
              const std::string &license_server_url);
  static bool OnLicenseAcquired(int *drm_handle, unsigned int length,
                                unsigned char *pssh_data, void *user_data);

  // Implementer of plusplayer::EventListener
  void OnPrepareDone(bool ret, UserData userdata) override;
  void OnBufferStatus(const int percent, UserData userdata) override;
  void OnSeekDone(UserData userdata) override;
  void OnEos(UserData userdata) override;
  void OnSubtitleData(std::unique_ptr<char[]> data, const int size,
                      const plusplayer::SubtitleType &type,
                      const uint64_t duration,
                      plusplayer::SubtitleAttrListPtr attr_list,
                      UserData userdata) override;
  void OnResourceConflicted(UserData userdata) override;
  void OnError(const plusplayer::ErrorType &error_code,
               UserData userdata) override;
  void OnErrorMsg(const plusplayer::ErrorType &error_code,
                  const char *error_msg, UserData userdata) override;
  void OnDrmInitData(int *drmhandle, unsigned int len, unsigned char *psshdata,
                     plusplayer::TrackType type, UserData userdata) override;
  void OnAdaptiveStreamingControlEvent(
      const plusplayer::StreamingMessageType &type,
      const plusplayer::MessageParam &msg, UserData userdata) override;
  void OnClosedCaptionData(std::unique_ptr<char[]> data, const int size,
                           UserData userdata) override;
  void OnCueEvent(const char *CueData, UserData userdata) override;
  void OnDateRangeEvent(const char *DateRangeData, UserData userdata) override;
  void OnStopReachEvent(bool StopReach, UserData userdata) override;
  void OnCueOutContEvent(const char *CueOutContData,
                         UserData userdata) override;
  void OnChangeSourceDone(bool ret, UserData userdata) override;
  void OnStateChangedToPlaying(UserData userdata) override;

  std::unique_ptr<plusplayer::PlusPlayer> player_;
  std::unique_ptr<DrmManager> drm_manager_;

  void *native_window_;
  std::string video_format_;
  bool is_buffering_ = false;
  bool is_prebuffer_mode_ = false;
  SeekCompletedCallback on_seek_completed_;
};

#endif  // FLUTTER_PLUGIN_PLUS_PLAYER_H_
