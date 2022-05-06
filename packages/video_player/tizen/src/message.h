#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_MESSAGE_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_MESSAGE_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

#include <functional>
#include <string>

#include "video_player_error.h"

class TextureMessage {
 public:
  TextureMessage() : textureId_(0) {}
  ~TextureMessage() = default;
  TextureMessage(TextureMessage const &) = default;
  TextureMessage &operator=(TextureMessage const &) = default;

  int64_t getTextureId() const;
  void setTextureId(int64_t textureId);
  flutter::EncodableValue toMap();
  static TextureMessage fromMap(const flutter::EncodableValue &value);

 private:
  int64_t textureId_;
};

class CreateMessage {
 public:
  CreateMessage() = default;
  ~CreateMessage() = default;
  CreateMessage(CreateMessage const &) = default;
  CreateMessage &operator=(CreateMessage const &) = default;

  std::string getAsset() const;
  void setAsset(const std::string &asset);
  std::string getUri() const;
  void setUri(const std::string &uri);
  std::string getPackageName() const;
  void setPackageName(const std::string &packageName);
  std::string getFormatHint() const;
  void setFormatHint(const std::string &formatHint);
  flutter::EncodableValue toMap();
  static CreateMessage fromMap(const flutter::EncodableValue &value);

 private:
  std::string asset_;
  std::string uri_;
  std::string packageName_;
  std::string formatHint_;
};

class LoopingMessage {
 public:
  LoopingMessage() : textureId_(0), isLooping_(false) {}
  ~LoopingMessage() = default;
  LoopingMessage(LoopingMessage const &) = default;
  LoopingMessage &operator=(LoopingMessage const &) = default;

  int64_t getTextureId() const;
  void setTextureId(int64_t textureId);
  bool getIsLooping() const;
  void setIsLooping(bool isLooping);
  flutter::EncodableValue toMap();
  static LoopingMessage fromMap(const flutter::EncodableValue &value);

 private:
  int64_t textureId_;
  bool isLooping_;
};

class VolumeMessage {
 public:
  VolumeMessage() : textureId_(0), volume_(0.0) {}
  ~VolumeMessage() = default;
  VolumeMessage(VolumeMessage const &) = default;
  VolumeMessage &operator=(VolumeMessage const &) = default;

  int64_t getTextureId() const;
  void setTextureId(int64_t textureId);
  double getVolume() const;
  void setVolume(double volume);
  flutter::EncodableValue toMap();
  static VolumeMessage fromMap(const flutter::EncodableValue &value);

 private:
  int64_t textureId_;
  double volume_;
};

class PlaybackSpeedMessage {
 public:
  PlaybackSpeedMessage() : textureId_(0), speed_(1.0) {}
  ~PlaybackSpeedMessage() = default;
  PlaybackSpeedMessage(PlaybackSpeedMessage const &) = default;
  PlaybackSpeedMessage &operator=(PlaybackSpeedMessage const &) = default;

  int64_t getTextureId() const;
  void setTextureId(int64_t textureId);
  double getSpeed() const;
  void setSpeed(double speed);
  flutter::EncodableValue toMap();
  static PlaybackSpeedMessage fromMap(const flutter::EncodableValue &value);

 private:
  int64_t textureId_;
  double speed_;
};

class PositionMessage {
 public:
  PositionMessage() : textureId_(0), position_(0) {}
  ~PositionMessage() = default;
  PositionMessage(PositionMessage const &) = default;
  PositionMessage &operator=(PositionMessage const &) = default;

  int64_t getTextureId() const;
  void setTextureId(int64_t textureId);
  int64_t getPosition() const;
  void setPosition(int64_t position);
  flutter::EncodableValue toMap();
  static PositionMessage fromMap(const flutter::EncodableValue &value);

 private:
  int64_t textureId_;
  int64_t position_;
};

class MixWithOthersMessage {
 public:
  MixWithOthersMessage() : mixWithOthers_(false) {}
  ~MixWithOthersMessage() = default;
  MixWithOthersMessage(MixWithOthersMessage const &) = default;
  MixWithOthersMessage &operator=(MixWithOthersMessage const &) = default;

  bool getMixWithOthers() const;
  void setMixWithOthers(bool mixWithOthers);
  flutter::EncodableValue toMap();
  static MixWithOthersMessage fromMap(const flutter::EncodableValue &value);

 private:
  bool mixWithOthers_;
};

class VideoPlayerApi {
 public:
  using SeekCompletedCallback = std::function<void()>;

  virtual void initialize() = 0;
  virtual TextureMessage create(const CreateMessage &createMsg) = 0;
  virtual void dispose(const TextureMessage &textureMsg) = 0;
  virtual void setLooping(const LoopingMessage &loopingMsg) = 0;
  virtual void setVolume(const VolumeMessage &volumeMsg) = 0;
  virtual void setPlaybackSpeed(const PlaybackSpeedMessage &speedMsg) = 0;
  virtual void play(const TextureMessage &textureMsg) = 0;
  virtual void pause(const TextureMessage &textureMsg) = 0;
  virtual PositionMessage position(const TextureMessage &textureMsg) = 0;
  virtual void seekTo(const PositionMessage &positionMsg,
                      const SeekCompletedCallback &onSeekCompleted) = 0;
  virtual void setMixWithOthers(
      const MixWithOthersMessage &mixWithOthersMsg) = 0;

  static void SetUp(flutter::BinaryMessenger *messenger, VideoPlayerApi *api);
  static flutter::EncodableValue WrapError(const VideoPlayerError &error);
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_MESSAGE_H_
