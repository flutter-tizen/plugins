#include "message.h"

#include <flutter/basic_message_channel.h>
#include <flutter/standard_message_codec.h>

#include "log.h"

int64_t TextureMessage::getTextureId() const { return textureId_; }

void TextureMessage::setTextureId(int64_t textureId) { textureId_ = textureId; }

flutter::EncodableValue TextureMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("textureId"),
       flutter::EncodableValue((int64_t)textureId_)}};
  return flutter::EncodableValue(toMapResult);
}

TextureMessage TextureMessage::fromMap(const flutter::EncodableValue &value) {
  TextureMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        emap[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }
  }
  return fromMapResult;
}

std::string CreateMessage::getAsset() const { return asset_; }

void CreateMessage::setAsset(const std::string &asset) { asset_ = asset; }

std::string CreateMessage::getUri() const { return uri_; }

void CreateMessage::setUri(const std::string &uri) { uri_ = uri; }

std::string CreateMessage::getPackageName() const { return packageName_; }

void CreateMessage::setPackageName(const std::string &packageName) {
  packageName_ = packageName;
}

std::string CreateMessage::getFormatHint() const { return formatHint_; }

void CreateMessage::setFormatHint(const std::string &formatHint) {
  formatHint_ = formatHint;
}

flutter::EncodableMap CreateMessage::getHttpHeaders() const {
  return httpHeaders_;
}

void CreateMessage::setHttpHeaders(flutter::EncodableMap setterArg) {
  httpHeaders_ = setterArg;
}

flutter::EncodableMap CreateMessage::GetDrmConfigs() const {
  return drm_configs_;
}
void CreateMessage::SetDrmConfigs(flutter::EncodableMap configArg) {
  drm_configs_ = configArg;
}

flutter::EncodableValue CreateMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("asset"), flutter::EncodableValue(asset_)},
      {flutter::EncodableValue("uri"), flutter::EncodableValue(uri_)},
      {flutter::EncodableValue("packageName"),
       flutter::EncodableValue(packageName_)},
      {flutter::EncodableValue("formatHint"),
       flutter::EncodableValue(formatHint_)},
      {flutter::EncodableValue("httpHeaders"),
       flutter::EncodableValue(httpHeaders_)},
      {flutter::EncodableValue("drmConfigs"),
       flutter::EncodableValue(drm_configs_)},
  };

  return flutter::EncodableValue(toMapResult);
}

CreateMessage CreateMessage::fromMap(const flutter::EncodableValue &value) {
  CreateMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &asset = emap[flutter::EncodableValue("asset")];
    if (std::holds_alternative<std::string>(asset)) {
      fromMapResult.setAsset(std::get<std::string>(asset));
    }

    flutter::EncodableValue &uri = emap[flutter::EncodableValue("uri")];
    if (std::holds_alternative<std::string>(uri)) {
      fromMapResult.setUri(std::get<std::string>(uri));
    }

    flutter::EncodableValue &packageName =
        emap[flutter::EncodableValue("packageName")];
    if (std::holds_alternative<std::string>(packageName)) {
      fromMapResult.setPackageName(std::get<std::string>(uri));
    }

    flutter::EncodableValue &formatHint =
        emap[flutter::EncodableValue("formatHint")];
    if (std::holds_alternative<std::string>(formatHint)) {
      fromMapResult.setFormatHint(std::get<std::string>(formatHint));
    }

    flutter::EncodableValue &encodable_httpHeaders =
        emap[flutter::EncodableValue("httpHeaders")];
    if (std::holds_alternative<flutter::EncodableMap>(encodable_httpHeaders)) {
      fromMapResult.setHttpHeaders(
          std::get<flutter::EncodableMap>(encodable_httpHeaders));
    }

    flutter::EncodableValue &encodable_drm_configs =
        emap[flutter::EncodableValue("drmConfigs")];
    if (std::holds_alternative<flutter::EncodableMap>(encodable_drm_configs)) {
      fromMapResult.SetDrmConfigs(
          std::get<flutter::EncodableMap>(encodable_drm_configs));
    }
  }

  return fromMapResult;
}

int64_t LoopingMessage::getTextureId() const { return textureId_; }

void LoopingMessage::setTextureId(int64_t textureId) { textureId_ = textureId; }

bool LoopingMessage::getIsLooping() const { return isLooping_; }

void LoopingMessage::setIsLooping(bool isLooping) { isLooping_ = isLooping; }

flutter::EncodableValue LoopingMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("textureId"),
       flutter::EncodableValue((int64_t)textureId_)},
      {flutter::EncodableValue("isLooping"),
       flutter::EncodableValue(isLooping_)}};

  return flutter::EncodableValue(toMapResult);
}

LoopingMessage LoopingMessage::fromMap(const flutter::EncodableValue &value) {
  LoopingMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        emap[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &isLooping =
        emap[flutter::EncodableValue("isLooping")];
    if (std::holds_alternative<bool>(isLooping)) {
      fromMapResult.setIsLooping(std::get<bool>(isLooping));
    }
  }

  return fromMapResult;
}

int64_t VolumeMessage::getTextureId() const { return textureId_; }

void VolumeMessage::setTextureId(int64_t textureId) { textureId_ = textureId; }

double VolumeMessage::getVolume() const { return volume_; }

void VolumeMessage::setVolume(double volume) { volume_ = volume; }

flutter::EncodableValue VolumeMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("textureId"),
       flutter::EncodableValue((int64_t)textureId_)},
      {flutter::EncodableValue("volume"), flutter::EncodableValue(volume_)}};

  return flutter::EncodableValue(toMapResult);
}

VolumeMessage VolumeMessage::fromMap(const flutter::EncodableValue &value) {
  VolumeMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        emap[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &volume = emap[flutter::EncodableValue("volume")];
    if (std::holds_alternative<double>(volume)) {
      fromMapResult.setVolume(std::get<double>(volume));
    }
  }

  return fromMapResult;
}

int64_t PlaybackSpeedMessage::getTextureId() const { return textureId_; }

void PlaybackSpeedMessage::setTextureId(int64_t textureId) {
  textureId_ = textureId;
}

double PlaybackSpeedMessage::getSpeed() const { return speed_; }

void PlaybackSpeedMessage::setSpeed(double speed) { speed_ = speed; }

flutter::EncodableValue PlaybackSpeedMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("textureId"),
       flutter::EncodableValue((int64_t)textureId_)},
      {flutter::EncodableValue("speed"), flutter::EncodableValue(speed_)}};

  return flutter::EncodableValue(toMapResult);
}

PlaybackSpeedMessage PlaybackSpeedMessage::fromMap(
    const flutter::EncodableValue &value) {
  PlaybackSpeedMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        emap[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &speed = emap[flutter::EncodableValue("speed")];
    if (std::holds_alternative<double>(speed)) {
      fromMapResult.setSpeed(std::get<double>(speed));
    }
  }

  return fromMapResult;
}

int64_t PositionMessage::getTextureId() const { return textureId_; }

void PositionMessage::setTextureId(int64_t textureId) {
  textureId_ = textureId;
}

int64_t PositionMessage::getPosition() const { return position_; }

void PositionMessage::setPosition(int64_t position) { position_ = position; }

flutter::EncodableValue PositionMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("textureId"),
       flutter::EncodableValue((int64_t)textureId_)},
      {flutter::EncodableValue("position"),
       flutter::EncodableValue((int64_t)position_)}};

  return flutter::EncodableValue(toMapResult);
}

PositionMessage PositionMessage::fromMap(const flutter::EncodableValue &value) {
  PositionMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        emap[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &position =
        emap[flutter::EncodableValue("position")];
    if (std::holds_alternative<int32_t>(position) ||
        std::holds_alternative<int64_t>(position)) {
      fromMapResult.setPosition(position.LongValue());
    }
  }

  return fromMapResult;
}

int64_t GeometryMessage::getTextureId() const { return textureId_; }
void GeometryMessage::setTextureId(int64_t textureId) {
  textureId_ = textureId;
}
int GeometryMessage::getX() const { return x_; }
void GeometryMessage::setX(int x) { x_ = x; }
int GeometryMessage::getY() const { return y_; }
void GeometryMessage::setY(int y) { y_ = y; }
int GeometryMessage::getW() const { return w_; }
void GeometryMessage::setW(int w) { w_ = w; }
int GeometryMessage::getH() const { return h_; }
void GeometryMessage::setH(int h) { h_ = h; }

flutter::EncodableValue GeometryMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("textureId"),
       flutter::EncodableValue((int64_t)textureId_)},
      {flutter::EncodableValue("x"), flutter::EncodableValue(x_)},
      {flutter::EncodableValue("y"), flutter::EncodableValue(y_)},
      {flutter::EncodableValue("w"), flutter::EncodableValue(w_)},
      {flutter::EncodableValue("h"), flutter::EncodableValue(h_)}};

  return flutter::EncodableValue(toMapResult);
}

GeometryMessage GeometryMessage::fromMap(const flutter::EncodableValue &value) {
  GeometryMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        emap[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &x = emap[flutter::EncodableValue("x")];
    if (std::holds_alternative<int>(x)) {
      fromMapResult.setX(std::get<int>(x));
    }

    flutter::EncodableValue &y = emap[flutter::EncodableValue("y")];
    if (std::holds_alternative<int>(y)) {
      fromMapResult.setY(std::get<int>(y));
    }

    flutter::EncodableValue &w = emap[flutter::EncodableValue("w")];
    if (std::holds_alternative<int>(w)) {
      fromMapResult.setW(std::get<int>(w));
    }

    flutter::EncodableValue &h = emap[flutter::EncodableValue("h")];
    if (std::holds_alternative<int>(h)) {
      fromMapResult.setH(std::get<int>(h));
    }
  }

  return fromMapResult;
}

int64_t BufferingConfigMessage::GetTextureId() const { return textureId_; }
void BufferingConfigMessage::SetTextureId(int64_t texture_id) {
  textureId_ = texture_id;
}
std::string BufferingConfigMessage::GetOption() const { return option_; }
void BufferingConfigMessage::SetOption(std::string &option) {
  option_ = option;
}
void BufferingConfigMessage::SetAmount(int amount) { amount_ = amount; }
int BufferingConfigMessage::GetAmount() const { return amount_; }

flutter::EncodableValue BufferingConfigMessage::ToMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("textureId"),
       flutter::EncodableValue((int64_t)textureId_)},
      {flutter::EncodableValue("bufferOption"),
       flutter::EncodableValue(option_)},
      {flutter::EncodableValue("amount"), flutter::EncodableValue(amount_)}};

  return flutter::EncodableValue(toMapResult);
}
BufferingConfigMessage BufferingConfigMessage::FromMap(
    const flutter::EncodableValue &value) {
  BufferingConfigMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        emap[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.SetTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &bufferOption =
        emap[flutter::EncodableValue("bufferOption")];
    if (std::holds_alternative<std::string>(bufferOption)) {
      fromMapResult.SetOption(std::get<std::string>(bufferOption));
    }

    flutter::EncodableValue &amount = emap[flutter::EncodableValue("amount")];
    if (std::holds_alternative<int>(amount)) {
      fromMapResult.SetAmount(std::get<int>(amount));
    }
  }
  return fromMapResult;
}

bool MixWithOthersMessage::getMixWithOthers() const { return mixWithOthers_; }

void MixWithOthersMessage::setMixWithOthers(bool mixWithOthers) {
  mixWithOthers_ = mixWithOthers;
}

flutter::EncodableValue MixWithOthersMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("mixWithOthers"),
       flutter::EncodableValue(mixWithOthers_)}};

  return flutter::EncodableValue(toMapResult);
}

MixWithOthersMessage MixWithOthersMessage::fromMap(
    const flutter::EncodableValue &value) {
  MixWithOthersMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap emap = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &mixWithOthers =
        emap[flutter::EncodableValue("mixWithOthers")];
    if (std::holds_alternative<bool>(mixWithOthers)) {
      fromMapResult.setMixWithOthers(std::get<bool>(mixWithOthers));
    }
  }

  return fromMapResult;
}

void VideoPlayerApi::setup(flutter::BinaryMessenger *binaryMessenger,
                           VideoPlayerApi *api) {
  auto initChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.initialize",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    initChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          flutter::EncodableMap wrapped;
          try {
            api->initialize();
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto createChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.create",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    createChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          CreateMessage input = CreateMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            TextureMessage output = api->create(input);
            wrapped.emplace(flutter::EncodableValue("result"), output.toMap());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto disposeChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.dispose",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    disposeChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->dispose(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto loopingChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.setLooping",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    loopingChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          LoopingMessage input = LoopingMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setLooping(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto volumeChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.setVolume",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    volumeChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          VolumeMessage input = VolumeMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setVolume(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto speedChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.setPlaybackSpeed",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    speedChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          PlaybackSpeedMessage input = PlaybackSpeedMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setPlaybackSpeed(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto playChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.play",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    playChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->play(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto positionChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.position",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    positionChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            PositionMessage output = api->position(input);
            wrapped.emplace(flutter::EncodableValue("result"), output.toMap());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto seekToChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.seekTo",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    seekToChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          PositionMessage input = PositionMessage::fromMap(message);
          try {
            api->seekTo(input, [reply]() {
              flutter::EncodableMap wrapped = {
                  {flutter::EncodableValue("result"),
                   flutter::EncodableValue()}};
              reply(flutter::EncodableValue(wrapped));
            });
          } catch (const VideoPlayerError &e) {
            flutter::EncodableMap error = {{flutter::EncodableValue("error"),
                                            VideoPlayerApi::wrapError(e)}};
            reply(flutter::EncodableValue(error));
          }
        });
  }

  auto pauseChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.pause",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    pauseChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->pause(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto mixChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.setMixWithOthers",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    mixChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          MixWithOthersMessage input = MixWithOthersMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setMixWithOthers(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto roiChannel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger, "dev.flutter.pigeon.VideoPlayerApi.setDisplayRoi",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    roiChannel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          GeometryMessage input = GeometryMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setDisplayRoi(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto buffering_config_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          binaryMessenger,
          "dev.flutter.pigeon.VideoPlayerApi.setBufferingConfig",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    buffering_config_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          BufferingConfigMessage input =
              BufferingConfigMessage::FromMap(message);
          flutter::EncodableMap wrapped;
          try {
            bool ret = api->setBufferingConfig(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue(ret));
          } catch (const VideoPlayerError &e) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::wrapError(e));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }
}

flutter::EncodableValue VideoPlayerApi::wrapError(
    const VideoPlayerError &error) {
  flutter::EncodableMap wrapped = {
      {flutter::EncodableValue("message"),
       flutter::EncodableValue(error.GetMessage())},
      {flutter::EncodableValue("code"),
       flutter::EncodableValue(error.GetCode())},
      {flutter::EncodableValue("details"), flutter::EncodableValue()}};
  return flutter::EncodableValue(wrapped);
}
