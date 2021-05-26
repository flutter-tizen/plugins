#ifndef VIDEO_PLAYER_H_
#define VIDEO_PLAYER_H_

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/plugin_registrar.h>
#include <player.h>

#include <string>
#include <mutex>

#include "video_player_options.h"

class VideoPlayer {
 public:
  VideoPlayer(flutter::PluginRegistrar *pluginRegistrar,
              flutter::TextureRegistrar *textureRegistrar,
              const std::string &uri, VideoPlayerOptions &options);
  ~VideoPlayer();

  long getTextureId();
  void play();
  void pause();
  void setLooping(bool isLooping);
  void setVolume(double volume);
  void setPlaybackSpeed(double speed);
  void seekTo(int position);  // milliseconds
  int getPosition();          // milliseconds
  void dispose();

 private:
  void initialize();
  void setupEventChannel(flutter::BinaryMessenger *messenger);
  void sendInitialized();
  void sendBufferingStart();
  void sendBufferingUpdate(int position);  // milliseconds
  void sendBufferingEnd();
  FlutterDesktopGpuBuffer *CopyGpuBuffer(size_t width, size_t height);
  void Destruction(void* buffer);

  static void onPrepared(void *data);
  static void onBuffering(int percent, void *data);
  static void onPlayCompleted(void *data);
  static void onInterrupted(player_interrupted_code_e code, void *data);
  static void onErrorOccurred(int code, void *data);
  static void onVideoFrameDecoded(media_packet_h packet, void *data);

  bool isInitialized_;
  player_h player_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> eventChannel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> eventSink_;
  long textureId_;
  flutter::TextureRegistrar *textureRegistrar_;
  std::unique_ptr<flutter::TextureVariant> textureVariant_;
  std::unique_ptr<FlutterDesktopGpuBuffer> flutterDesttopGpuBuffer_;
  std::mutex mutex_;
  media_packet_h mediaPacket_ = nullptr;
};

#endif  // VIDEO_PLAYER_H_
