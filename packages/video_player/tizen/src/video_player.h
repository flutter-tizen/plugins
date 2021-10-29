#ifndef VIDEO_PLAYER_H_
#define VIDEO_PLAYER_H_

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include <player.h>

#include <mutex>
#include <string>

#include "video_player_options.h"

using SeekCompletedCb = std::function<void()>;

class VideoPlayer {
 public:
  VideoPlayer(flutter::PluginRegistrar *plugin_registrar,
              flutter::TextureRegistrar *texture_registrar,
              const std::string &uri, VideoPlayerOptions &options);
  ~VideoPlayer();

  long getTextureId();
  void play();
  void pause();
  void setLooping(bool is_looping);
  void setVolume(double volume);
  void setPlaybackSpeed(double speed);
  void seekTo(int position,
              const SeekCompletedCb &seek_completed_cb);  // milliseconds
  int getPosition();                                      // milliseconds
  void dispose();

 private:
  void initialize();
  void setupEventChannel(flutter::BinaryMessenger *messenger);
  void sendInitialized();
  void sendBufferingStart();
  void sendBufferingUpdate(int position);  // milliseconds
  void sendBufferingEnd();
  FlutterDesktopGpuBuffer *ObtainGpuBuffer(size_t width, size_t height);
  void Destruct(void *buffer);

  static void onPrepared(void *data);
  static void onBuffering(int percent, void *data);
  static void onSeekCompleted(void *data);
  static void onPlayCompleted(void *data);
  static void onInterrupted(player_interrupted_code_e code, void *data);
  static void onErrorOccurred(int code, void *data);
  static void onVideoFrameDecoded(media_packet_h packet, void *data);

  bool is_initialized_;
  player_h player_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
  long texture_id_;
  flutter::TextureRegistrar *texture_registrar_;
  std::unique_ptr<flutter::TextureVariant> texture_variant_;
  std::unique_ptr<FlutterDesktopGpuBuffer> flutter_desktop_gpu_buffer_;
  std::mutex mutex_;
  media_packet_h media_packet_ = nullptr;
  SeekCompletedCb on_seek_completed_;
};

#endif  // VIDEO_PLAYER_H_
