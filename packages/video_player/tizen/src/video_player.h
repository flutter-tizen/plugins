// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_H_

#include <Ecore.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/texture_registrar.h>
#include <player.h>

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include "video_player_options.h"

typedef int (*ScreensaverResetTimeout)(void);
typedef int (*ScreensaverOverrideReset)(bool onoff);

class VideoPlayer {
 public:
  using SeekCompletedCallback = std::function<void()>;

  explicit VideoPlayer(flutter::PluginRegistrar *plugin_registrar,
                       flutter::TextureRegistrar *texture_registrar,
                       const std::string &uri, VideoPlayerOptions &options);
  ~VideoPlayer();

  void Play();
  void Pause();
  void SetLooping(bool is_looping);
  void SetVolume(double volume);
  void SetPlaybackSpeed(double speed);
  void SeekTo(int32_t position, SeekCompletedCallback callback);
  int32_t GetPosition();
  void Dispose();

  int64_t GetTextureId() { return texture_id_; }

 private:
  FlutterDesktopGpuSurfaceDescriptor *ObtainGpuSurface(size_t width,
                                                       size_t height);

  void SetUpEventChannel(flutter::BinaryMessenger *messenger);
  void Initialize();
  void SendInitialized();

  static void OnPrepared(void *data);
  static void OnBuffering(int percent, void *data);
  static void OnSeekCompleted(void *data);
  static void OnPlayCompleted(void *data);
  static void OnInterrupted(player_interrupted_code_e code, void *data);
  static void OnError(int error_code, void *data);
  static void OnVideoFrameDecoded(media_packet_h packet, void *data);
  static void ReleaseMediaPacket(void *packet);
  static Eina_Bool ScreenSaverBlock(void *data);

  void RequestRendering();
  void OnRenderingCompleted();

  media_packet_h current_media_packet_ = nullptr;
  media_packet_h previous_media_packet_ = nullptr;

  bool is_initialized_ = false;
  bool is_rendering_ = false;

  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;

  player_h player_ = nullptr;
  int64_t texture_id_ = -1;

  flutter::TextureRegistrar *texture_registrar_;
  std::unique_ptr<flutter::TextureVariant> texture_variant_;
  std::unique_ptr<FlutterDesktopGpuSurfaceDescriptor> gpu_surface_;
  std::mutex mutex_;
  std::queue<media_packet_h> packet_queue_;

  SeekCompletedCallback on_seek_completed_;
  Ecore_Timer *timer;
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_H_
