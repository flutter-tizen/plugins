// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message.h"

#include <flutter/basic_message_channel.h>
#include <flutter/standard_message_codec.h>

int64_t TextureMessage::getTextureId() const { return textureId_; }

void TextureMessage::setTextureId(int64_t textureId) { textureId_ = textureId; }

flutter::EncodableValue TextureMessage::toMap() {
  flutter::EncodableMap toMapResult = {{flutter::EncodableValue("textureId"),
                                        flutter::EncodableValue(textureId_)}};
  return flutter::EncodableValue(toMapResult);
}

TextureMessage TextureMessage::fromMap(const flutter::EncodableValue &value) {
  TextureMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        map[flutter::EncodableValue("textureId")];
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

flutter::EncodableValue CreateMessage::toMap() {
  flutter::EncodableMap toMapResult = {
      {flutter::EncodableValue("asset"), flutter::EncodableValue(asset_)},
      {flutter::EncodableValue("uri"), flutter::EncodableValue(uri_)},
      {flutter::EncodableValue("packageName"),
       flutter::EncodableValue(packageName_)},
      {flutter::EncodableValue("formatHint"),
       flutter::EncodableValue(formatHint_)}};
  return flutter::EncodableValue(toMapResult);
}

CreateMessage CreateMessage::fromMap(const flutter::EncodableValue &value) {
  CreateMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &asset = map[flutter::EncodableValue("asset")];
    if (std::holds_alternative<std::string>(asset)) {
      fromMapResult.setAsset(std::get<std::string>(asset));
    }

    flutter::EncodableValue &uri = map[flutter::EncodableValue("uri")];
    if (std::holds_alternative<std::string>(uri)) {
      fromMapResult.setUri(std::get<std::string>(uri));
    }

    flutter::EncodableValue &packageName =
        map[flutter::EncodableValue("packageName")];
    if (std::holds_alternative<std::string>(packageName)) {
      fromMapResult.setPackageName(std::get<std::string>(uri));
    }

    flutter::EncodableValue &formatHint =
        map[flutter::EncodableValue("formatHint")];
    if (std::holds_alternative<std::string>(formatHint)) {
      fromMapResult.setFormatHint(std::get<std::string>(formatHint));
    }
  }
  return fromMapResult;
}

int64_t LoopingMessage::getTextureId() const { return textureId_; }

void LoopingMessage::setTextureId(int64_t textureId) { textureId_ = textureId; }

bool LoopingMessage::getIsLooping() const { return isLooping_; }

void LoopingMessage::setIsLooping(bool isLooping) { isLooping_ = isLooping; }

flutter::EncodableValue LoopingMessage::toMap() {
  flutter::EncodableMap toMapResult = {{flutter::EncodableValue("textureId"),
                                        flutter::EncodableValue(textureId_)},
                                       {flutter::EncodableValue("isLooping"),
                                        flutter::EncodableValue(isLooping_)}};
  return flutter::EncodableValue(toMapResult);
}

LoopingMessage LoopingMessage::fromMap(const flutter::EncodableValue &value) {
  LoopingMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        map[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &isLooping =
        map[flutter::EncodableValue("isLooping")];
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
       flutter::EncodableValue(textureId_)},
      {flutter::EncodableValue("volume"), flutter::EncodableValue(volume_)}};
  return flutter::EncodableValue(toMapResult);
}

VolumeMessage VolumeMessage::fromMap(const flutter::EncodableValue &value) {
  VolumeMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        map[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &volume = map[flutter::EncodableValue("volume")];
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
       flutter::EncodableValue(textureId_)},
      {flutter::EncodableValue("speed"), flutter::EncodableValue(speed_)}};
  return flutter::EncodableValue(toMapResult);
}

PlaybackSpeedMessage PlaybackSpeedMessage::fromMap(
    const flutter::EncodableValue &value) {
  PlaybackSpeedMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        map[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &speed = map[flutter::EncodableValue("speed")];
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
  flutter::EncodableMap toMapResult = {{flutter::EncodableValue("textureId"),
                                        flutter::EncodableValue(textureId_)},
                                       {flutter::EncodableValue("position"),
                                        flutter::EncodableValue(position_)}};
  return flutter::EncodableValue(toMapResult);
}

PositionMessage PositionMessage::fromMap(const flutter::EncodableValue &value) {
  PositionMessage fromMapResult;
  if (std::holds_alternative<flutter::EncodableMap>(value)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &textureId =
        map[flutter::EncodableValue("textureId")];
    if (std::holds_alternative<int32_t>(textureId) ||
        std::holds_alternative<int64_t>(textureId)) {
      fromMapResult.setTextureId(textureId.LongValue());
    }

    flutter::EncodableValue &position =
        map[flutter::EncodableValue("position")];
    if (std::holds_alternative<int32_t>(position) ||
        std::holds_alternative<int64_t>(position)) {
      fromMapResult.setPosition(position.LongValue());
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
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(value);
    flutter::EncodableValue &mixWithOthers =
        map[flutter::EncodableValue("mixWithOthers")];
    if (std::holds_alternative<bool>(mixWithOthers)) {
      fromMapResult.setMixWithOthers(std::get<bool>(mixWithOthers));
    }
  }
  return fromMapResult;
}

void VideoPlayerApi::SetUp(flutter::BinaryMessenger *messenger,
                           VideoPlayerApi *api) {
  auto init_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.initialize",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    init_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          flutter::EncodableMap wrapped;
          try {
            api->initialize();
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto create_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.create",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    create_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          CreateMessage input = CreateMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            TextureMessage output = api->create(input);
            wrapped.emplace(flutter::EncodableValue("result"), output.toMap());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto dispose_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.dispose",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    dispose_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->dispose(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto looping_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.setLooping",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    looping_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          LoopingMessage input = LoopingMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setLooping(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto volume_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.setVolume",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    volume_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          VolumeMessage input = VolumeMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setVolume(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto speed_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.setPlaybackSpeed",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    speed_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          PlaybackSpeedMessage input = PlaybackSpeedMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setPlaybackSpeed(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto play_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.play",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    play_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->play(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto position_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.position",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    position_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            PositionMessage output = api->position(input);
            wrapped.emplace(flutter::EncodableValue("result"), output.toMap());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto seek_to_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.seekTo",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    seek_to_channel->SetMessageHandler(
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
          } catch (const VideoPlayerError &error) {
            flutter::EncodableMap wrapped = {
                {flutter::EncodableValue("error"),
                 VideoPlayerApi::WrapError(error)}};
            reply(flutter::EncodableValue(wrapped));
          }
        });
  }

  auto pause_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.pause",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    pause_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          TextureMessage input = TextureMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->pause(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }

  auto mix_channel =
      std::make_unique<flutter::BasicMessageChannel<flutter::EncodableValue>>(
          messenger, "dev.flutter.pigeon.VideoPlayerApi.setMixWithOthers",
          &flutter::StandardMessageCodec::GetInstance());
  if (api != nullptr) {
    mix_channel->SetMessageHandler(
        [api](const flutter::EncodableValue &message,
              flutter::MessageReply<flutter::EncodableValue> reply) {
          MixWithOthersMessage input = MixWithOthersMessage::fromMap(message);
          flutter::EncodableMap wrapped;
          try {
            api->setMixWithOthers(input);
            wrapped.emplace(flutter::EncodableValue("result"),
                            flutter::EncodableValue());
          } catch (const VideoPlayerError &error) {
            wrapped.emplace(flutter::EncodableValue("error"),
                            VideoPlayerApi::WrapError(error));
          }
          reply(flutter::EncodableValue(wrapped));
        });
  }
}

flutter::EncodableValue VideoPlayerApi::WrapError(
    const VideoPlayerError &error) {
  flutter::EncodableMap wrapped = {
      {flutter::EncodableValue("message"),
       flutter::EncodableValue(error.getMessage())},
      {flutter::EncodableValue("code"),
       flutter::EncodableValue(error.getCode())},
      {flutter::EncodableValue("details"), flutter::EncodableValue()}};
  return flutter::EncodableValue(wrapped);
}
