// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_tizen.h>

#include <condition_variable>
#include <mutex>

#include "dart_api_dl.c"
#include "log.h"
#include "messages.h"
#include "video_player.h"
#include "video_player_options.h"

class VideoPlayerTizenPlugin : public flutter::Plugin, public VideoPlayerApi {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar);
  VideoPlayerTizenPlugin(FlutterDesktopPluginRegistrarRef registrar_ref,
                         flutter::PluginRegistrar *plugin_registrar);
  virtual ~VideoPlayerTizenPlugin();
  std::optional<FlutterError> Initialize() override;
  ErrorOr<std::unique_ptr<PlayerMessage>> Create(
      const CreateMessage &createMsg) override;
  std::optional<FlutterError> Dispose(const PlayerMessage &playerMsg) override;
  std::optional<FlutterError> SetLooping(
      const LoopingMessage &loopingMsg) override;
  std::optional<FlutterError> SetVolume(
      const VolumeMessage &volumeMsg) override;
  std::optional<FlutterError> SetPlaybackSpeed(
      const PlaybackSpeedMessage &speedMsg) override;
  std::optional<FlutterError> Play(const PlayerMessage &playerMsg) override;
  std::optional<FlutterError> Pause(const PlayerMessage &playerMsg) override;
  ErrorOr<std::unique_ptr<PositionMessage>> Position(
      const PlayerMessage &playerMsg) override;
  std::optional<FlutterError> SeekTo(
      const PositionMessage &positionMsg) override;
  std::optional<FlutterError> SetMixWithOthers(
      const MixWithOthersMessage &mixWithOthersMsg) override;
  std::optional<FlutterError> SetDisplayRoi(
      const GeometryMessage &geometryMsg) override;

 private:
  void DisposeAllPlayers();
  FlutterDesktopPluginRegistrarRef registrar_ref_;
  VideoPlayerOptions options_;
  flutter::PluginRegistrar *plugin_registrar_;
  std::map<int64_t, std::unique_ptr<VideoPlayer>> players_;
};

void VideoPlayerTizenPlugin::RegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar) {
  auto plugin =
      std::make_unique<VideoPlayerTizenPlugin>(registrar_ref, plugin_registrar);
  plugin_registrar->AddPlugin(std::move(plugin));
}

VideoPlayerTizenPlugin::VideoPlayerTizenPlugin(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar)
    : registrar_ref_(registrar_ref), plugin_registrar_(plugin_registrar) {
  VideoPlayerApi::SetUp(plugin_registrar_->messenger(), this);
}

VideoPlayerTizenPlugin::~VideoPlayerTizenPlugin() { DisposeAllPlayers(); }

void VideoPlayerTizenPlugin::DisposeAllPlayers() {
  auto iter = players_.begin();
  while (iter != players_.end()) {
    iter->second->Dispose();
    iter++;
  }
  players_.clear();
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Initialize() {
  DisposeAllPlayers();
  return {};
}

ErrorOr<std::unique_ptr<PlayerMessage>> VideoPlayerTizenPlugin::Create(
    const CreateMessage &createMsg) {
  std::unique_ptr<VideoPlayer> player =
      std::make_unique<VideoPlayer>(registrar_ref_, createMsg);
  set_license_cb_ = MyCallbackBlocking;
  if (set_license_cb_ != nullptr) {
    player->SetDrmLicense(set_license_cb_);
  }
  std::unique_ptr<PlayerMessage> player_message =
      std::make_unique<PlayerMessage>();
  int64_t playerId = player->Create();
  if (playerId != -1) {
    players_[playerId] = std::move(player);
    player_message->set_player_id(playerId);
  }
  return ErrorOr<PlayerMessage>::MakeWithUniquePtr(std::move(player_message));
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Dispose(
    const PlayerMessage &playerMsg) {
  auto iter = players_.find(playerMsg.player_id());
  if (iter != players_.end()) {
    iter->second->Dispose();
    players_.erase(iter);
  }
  return {};
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetLooping(
    const LoopingMessage &loopingMsg) {
  auto iter = players_.find(loopingMsg.player_id());
  if (iter != players_.end()) {
    iter->second->SetLooping(loopingMsg.is_looping());
  }
  return {};
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetVolume(
    const VolumeMessage &volumeMsg) {
  auto iter = players_.find(volumeMsg.player_id());
  if (iter != players_.end()) {
    iter->second->SetVolume(volumeMsg.volume());
  }
  return {};
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetPlaybackSpeed(
    const PlaybackSpeedMessage &speedMsg) {
  auto iter = players_.find(speedMsg.player_id());
  if (iter != players_.end()) {
    iter->second->SetPlaybackSpeed(speedMsg.speed());
  }
  return {};
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Play(
    const PlayerMessage &playerMsg) {
  auto iter = players_.find(playerMsg.player_id());
  if (iter != players_.end()) {
    iter->second->Play();
  }
  return {};
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Pause(
    const PlayerMessage &playerMsg) {
  auto iter = players_.find(playerMsg.player_id());
  if (iter != players_.end()) {
    iter->second->Pause();
  }
  return {};
}

ErrorOr<std::unique_ptr<PositionMessage>> VideoPlayerTizenPlugin::Position(
    const PlayerMessage &playerMsg) {
  std::unique_ptr<PositionMessage> result = std::make_unique<PositionMessage>();
  auto iter = players_.find(playerMsg.player_id());
  if (iter != players_.end()) {
    result->set_player_id(playerMsg.player_id());
    result->set_position(iter->second->GetPosition());
  }
  return ErrorOr<PositionMessage>::MakeWithUniquePtr(std::move(result));
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SeekTo(
    const PositionMessage &positionMsg) {
  auto iter = players_.find(positionMsg.player_id());
  if (iter != players_.end()) {
    iter->second->SeekTo(positionMsg.position());
  }
  return {};
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetDisplayRoi(
    const GeometryMessage &geometryMsg) {
  auto iter = players_.find(geometryMsg.player_id());
  if (iter != players_.end()) {
    iter->second->SetDisplayRoi(geometryMsg.x(), geometryMsg.y(),
                                geometryMsg.w(), geometryMsg.h());
  }
  return {};
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetMixWithOthers(
    const MixWithOthersMessage &mixWithOthersMsg) {
  options_.SetMixWithOthers(mixWithOthersMsg.mix_with_others());
  return {};
}

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}

// Call Dart function through FFI
intptr_t InitDartApiDL(void *data) { return Dart_InitializeApiDL(data); }
Dart_Port send_port_;

void RegisterSendPort(Dart_Port send_port) { send_port_ = send_port; }

void GetLicense(FuncLicenseCB callback) { license_cb_ = callback; }

void ExecuteCallback(CallbackWrapper *wrapper_ptr) {
  const CallbackWrapper wrapper = *wrapper_ptr;
  wrapper();
  delete wrapper_ptr;
  LOG_INFO("ExecuteCallback done");
}

void NotifyDart(CallbackWrapper *wrapper) {
  intptr_t wrapper_intptr = reinterpret_cast<intptr_t>(wrapper);

  Dart_CObject ptr_obj;
  ptr_obj.type = Dart_CObject_kInt64;
  ptr_obj.value.as_int64 = wrapper_intptr;

  bool posted = Dart_PostCObject_DL(send_port_, &ptr_obj);
  if (!posted) {
    LOG_ERROR("Dart_PostCObject_DL(): message %d not posted", wrapper_intptr);
  }
  LOG_INFO("after calling Dart_PostCObject_DL()");
}

uint8_t MyCallbackBlocking(char *challenge) {
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  uint8_t result;
  auto callback = license_cb_;
  std::condition_variable cv;
  bool notified = false;
  const CallbackWrapper wrapper = [&]() {
    result = callback(challenge);
    LOG_INFO("Notify result ready");
    notified = true;
    cv.notify_one();
  };
  CallbackWrapper *wrapper_ptr = new CallbackWrapper(wrapper);
  NotifyDart(wrapper_ptr);
  LOG_INFO("Waiting for result");
  while (!notified) {
    cv.wait(lock);
  }
  LOG_INFO("Received result");
  return result;
}
