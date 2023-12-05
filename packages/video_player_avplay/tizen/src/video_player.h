// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_H_

#include <Ecore.h>
#include <dart_api_dl.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter_tizen.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <utility>

#include "ecore_wl2_window_proxy.h"

class VideoPlayer {
 public:
  using SeekCompletedCallback = std::function<void()>;

  explicit VideoPlayer(flutter::BinaryMessenger *messenger,
                       FlutterDesktopViewRef flutter_view);
  VideoPlayer(const VideoPlayer &) = delete;
  VideoPlayer &operator=(const VideoPlayer &) = delete;
  virtual ~VideoPlayer();

  virtual int64_t Create(const std::string &uri, int drm_type,
                         const std::string &license_server_url,
                         bool is_prebuffer_mode,
                         flutter::EncodableMap &http_headers) = 0;
  virtual void Dispose() = 0;

  virtual void SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                             int32_t height) = 0;
  virtual bool Play() = 0;
  virtual bool Deactivate() { return false; };
  virtual bool Activate() { return false; };
  virtual bool Pause() = 0;
  virtual bool SetLooping(bool is_looping) = 0;
  virtual bool SetVolume(double volume) = 0;
  virtual bool SetPlaybackSpeed(double speed) = 0;
  virtual bool SeekTo(int64_t position, SeekCompletedCallback callback) = 0;
  virtual int64_t GetPosition() = 0;
  virtual int64_t GetDuration() = 0;
  virtual bool IsReady() = 0;
  virtual flutter::EncodableList GetTrackInfo(std::string track_type) = 0;
  virtual bool SetTrackSelection(int32_t track_id, std::string track_type) = 0;

 protected:
  virtual void GetVideoSize(int32_t *width, int32_t *height) = 0;
  void *GetWindowHandle();
  int64_t SetUpEventChannel();
  void SendInitialized();
  void SendBufferingStart();
  void SendBufferingUpdate(int32_t value);
  void SendBufferingEnd();
  void SendSubtitleUpdate(int32_t duration, const std::string &text);
  void SendPlayCompleted();
  void SendError(const std::string &error_code,
                 const std::string &error_message);

  std::mutex queue_mutex_;
  std::unique_ptr<EcoreWl2WindowProxy> ecore_wl2_window_proxy_ = nullptr;
  flutter::BinaryMessenger *binary_messenger_;
  bool is_initialized_ = false;
  FlutterDesktopViewRef flutter_view_;

 private:
  void ExecuteSinkEvents();
  void PushEvent(flutter::EncodableValue encodable_value);

  std::queue<flutter::EncodableValue> encodable_event_queue_;
  std::queue<std::pair<std::string, std::string>> error_event_queue_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
  Ecore_Pipe *sink_event_pipe_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_H_
