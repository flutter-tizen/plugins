// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_H_

#include <Ecore.h>
#include <dart_api_dl.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <utility>

#include "ecore_wl2_window_proxy.h"

class VideoPlayer {
 public:
  using SeekCompletedCallback = std::function<void()>;

  explicit VideoPlayer(flutter::BinaryMessenger *messenger);
  VideoPlayer(const VideoPlayer &) = delete;
  VideoPlayer &operator=(const VideoPlayer &) = delete;
  virtual ~VideoPlayer();

  virtual int64_t Create(const std::string &uri, int drm_type,
                         const std::string &license_server_url,
                         bool is_prebuffer_mode) = 0;
  virtual void Dispose() = 0;

  virtual void SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                             int32_t height) = 0;
  virtual void Play() = 0;
  virtual bool SetDeactivate() { return false; };
  virtual bool SetActivate() { return false; };
  virtual void Pause() = 0;
  virtual void SetLooping(bool is_looping) = 0;
  virtual void SetVolume(double volume) = 0;
  virtual void SetPlaybackSpeed(double speed) = 0;
  virtual void SeekTo(int64_t position, SeekCompletedCallback callback) = 0;
  virtual int64_t GetPosition() = 0;
  virtual int64_t GetDuration() = 0;
  virtual bool isReady() = 0;
  virtual flutter::EncodableList getTrackInfo(int32_t track_type) = 0;
  virtual bool SetTrackSelection(int32_t track_id, int32_t track_type) = 0;
  // send port is used for drm
  void RegisterSendPort(Dart_Port send_port) { send_port_ = send_port; }

 protected:
  virtual void GetVideoSize(int32_t *width, int32_t *height) = 0;
  int64_t SetUpEventChannel();
  void SendInitialized();
  void SendBufferingStart();
  void SendBufferingUpdate(int32_t value);
  void SendBufferingEnd();
  void SendSubtitleUpdate(int32_t duration, const std::string &text);
  void SendPlayCompleted();
  void SendError(const std::string &error_code,
                 const std::string &error_message);

  void OnLicenseChallenge(const void *challenge, unsigned long challenge_len,
                          void **response, unsigned long *response_len);
  bool is_initialized_ = false;
  std::mutex queue_mutex_;
  std::unique_ptr<EcoreWl2WindowProxy> ecore_wl2_window_proxy_ = nullptr;

 private:
  void ExecuteSinkEvents();
  void PushEvent(flutter::EncodableValue encodable_value);
  std::queue<flutter::EncodableValue> encodable_event_queue_;
  std::queue<std::pair<std::string, std::string>> error_event_queue_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
  flutter::BinaryMessenger *binary_messenger_;
  Dart_Port send_port_;
  Ecore_Pipe *sink_event_pipe_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_H_
