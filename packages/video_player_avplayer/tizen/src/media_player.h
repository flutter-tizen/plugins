// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_MEDIA_PLAYER_H_
#define FLUTTER_PLUGIN_MEDIA_PLAYER_H_

#include <flutter/plugin_registrar.h>

#include "drm_manager.h"
#include "media_player_proxy.h"
#include "video_player.h"
#include <memory>
#include <string>
#include <utility>

class MediaPlayer : public VideoPlayer {
 public:
  explicit MediaPlayer(flutter::BinaryMessenger *messenger,
                       void *native_window);
  ~MediaPlayer();

  int64_t Create(const std::string &uri, int drm_type,
                 const std::string &license_server_url,
                 bool is_prebuffer_mode) override;
  void Dispose() override;

  void SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                     int32_t height) override;
  void Play() override;
  void Pause() override;
  void SetLooping(bool is_looping) override;
  void SetVolume(double volume) override;
  void SetPlaybackSpeed(double speed) override;
  void SeekTo(int32_t position, SeekCompletedCallback callback) override;
  int32_t GetPosition() override;
  int32_t GetDuration() override;
  void GetVideoSize(int32_t *width, int32_t *height) override;
  bool isReady() override;
  flutter::EncodableList getTrackInfo(int32_t track_type) override;
  bool SetTrackSelection(int32_t track_id, int32_t track_type) override;

 private:
  bool SetDisplay();
  bool SetDrm(const std::string &uri, int drm_type,
              const std::string &license_server_url);

  static void OnPrepared(void *user_data);
  static void OnBuffering(int percent, void *user_data);
  static void OnSeekCompleted(void *user_data);
  static void OnPlayCompleted(void *user_data);
  static void OnInterrupted(player_interrupted_code_e code, void *user_data);
  static void OnError(int error_code, void *user_data);
  static void OnSubtitleUpdated(unsigned long duration, char *text,
                                void *user_data);
  static bool OnDrmSecurityInitComplete(int *drm_handle, unsigned int length,
                                        unsigned char *pssh_data,
                                        void *user_data);
  static int OnDrmUpdatePsshData(drm_init_data_type init_type, void *data,
                                 int data_length, void *user_data);

  player_h player_ = nullptr;
  std::unique_ptr<MediaPlayerProxy> media_player_proxy_ = nullptr;
  std::unique_ptr<DrmManager> drm_manager_;
  bool is_buffering_ = false;
  void *native_window_ = nullptr;
  SeekCompletedCallback on_seek_completed_;
};

#endif  // FLUTTER_PLUGIN_MEDIA_PLAYER_H_
