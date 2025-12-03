// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PLUS_PLAYER_H_
#define FLUTTER_PLUGIN_PLUS_PLAYER_H_

#include <flutter/plugin_registrar.h>

#include <memory>
#include <string>

#include "device_proxy.h"
#include "drm_manager.h"
#include "messages.h"
#include "plus_player_util.h"
#include "plusplayer_capi/plusplayer_capi.h"
#include "video_player.h"

namespace video_player_avplay_tizen {

class PlusPlayer : public VideoPlayer {
 public:
  explicit PlusPlayer(flutter::BinaryMessenger *messenger,
                      FlutterDesktopViewRef flutter_view);
  ~PlusPlayer();

  int64_t Create(const std::string &uri,
                 const CreateMessage &create_message) override;
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
  std::pair<int64_t, int64_t> GetDuration() override;
  void GetVideoSize(int32_t *width, int32_t *height) override;
  bool IsReady() override;
  flutter::EncodableList GetTrackInfo(std::string track_type) override;
  bool SetTrackSelection(int32_t track_id, std::string track_type) override;
  std::string GetStreamingProperty(
      const std::string &streaming_property_type) override;
  bool SetBufferConfig(const std::string &key, int64_t value) override;
  void SetStreamingProperty(const std::string &type,
                            const std::string &value) override;
  bool SetDisplayRotate(int64_t rotation) override;
  bool SetDisplayMode(int64_t display_mode) override;
  bool Suspend() override;
  bool Restore(const CreateMessage *restore_message,
               int64_t resume_time) override;
  bool SetData(const flutter::EncodableMap &data) override;
  flutter::EncodableMap GetData(const flutter::EncodableList &data) override;
  bool UpdateDashToken(const std::string &dashToken) override;
  flutter::EncodableList GetActiveTrackInfo() override;

 private:
  bool GetMemento(PlayerMemento *memento);
  std::string GetExtraStreamingProperty(
      const std::string &streaming_property_type);
  bool IsLive();
  std::pair<int64_t, int64_t> GetLiveDuration();
  void PreSet(const CreateMessage &create_message);
  bool SetDisplay();
  bool SetAppId();
  bool SetDrm(const std::string &uri, int drm_type,
              const std::string &license_server_url);
  void UnregisterListener();
  void RegisterListener();
  bool StopAndClose();
  bool RestorePlayer(const CreateMessage *restore_message, int64_t resume_time);

  // Helper methods for SetStreamingProperty
  bool IsDashFormat() const;
  bool IsDashOnlyProperty(const std::string &type) const;
  void SetPropertyInternal(const std::string &type, const std::string &value);

  static bool OnLicenseAcquired(int *drm_handle, unsigned int length,
                                unsigned char *pssh_data, void *user_data);
  static void OnPrepareDone(bool ret, void *user_data);
  static void OnBufferStatus(int percent, void *user_data);
  static void OnSeekDone(void *user_data);
  static void OnEos(void *user_data);
  static void OnSubtitleData(const plusplayer_subtitle_type_e type,
                             const uint64_t duration_in_ms, const char *data,
                             const int size,
                             plusplayer_subtitle_attr_s *attr_list,
                             int attr_size, void *userdata);
  static void OnResourceConflicted(void *user_data);
  static void OnError(plusplayer_error_type_e error_type, void *user_data);
  static void OnErrorMsg(plusplayer_error_type_e error_type,
                         const char *error_msg, void *user_data);

  static void OnDrmInitData(Plusplayer_DrmHandle *drm_andle, unsigned int len,
                            unsigned char *pssh_data,
                            plusplayer_track_type_e type, void *user_data);

  static void OnAdaptiveStreamingControlEvent(
      plusplayer_streaming_message_type_e message_type,
      plusplayer_message_param_s *param, void *user_data);
  static void OnStateChangedToPlaying(void *user_data);
  static void OnADEventFromDash(const char *ad_data, void *user_data);

  plusplayer_h player_ = nullptr;
  std::unique_ptr<DrmManager> drm_manager_;
  bool is_buffering_ = false;
  bool is_prebuffer_mode_ = false;
  SeekCompletedCallback on_seek_completed_;
  std::unique_ptr<PlayerMemento> memento_ = nullptr;
  std::string url_;
  std::unique_ptr<DeviceProxy> device_proxy_ = nullptr;
  CreateMessage create_message_;
  plusplayer_geometry_s current_display_roi_{0, 0, 1, 1};
};

}  // namespace video_player_avplay_tizen

#endif  // FLUTTER_PLUGIN_PLUS_PLAYER_H_
