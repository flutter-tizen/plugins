// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <cstdint>
#include <map>

#include "messages.h"
#include "pending_call.h"
#include "video_player.h"
#include "video_player_options.h"

namespace {

class VideoPlayerTizenPlugin : public flutter::Plugin, public VideoPlayerApi {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar);

  VideoPlayerTizenPlugin(FlutterDesktopPluginRegistrarRef registrar_ref,
                         flutter::PluginRegistrar *plugin_registrar);
  virtual ~VideoPlayerTizenPlugin();

  std::optional<FlutterError> Initialize() override;
  ErrorOr<PlayerMessage> Create(const CreateMessage &msg) override;
  std::optional<FlutterError> Dispose(const PlayerMessage &msg) override;
  std::optional<FlutterError> SetLooping(const LoopingMessage &msg) override;
  std::optional<FlutterError> SetVolume(const VolumeMessage &msg) override;
  std::optional<FlutterError> SetPlaybackSpeed(
      const PlaybackSpeedMessage &msg) override;
  std::optional<FlutterError> Play(const PlayerMessage &msg) override;
  ErrorOr<PositionMessage> Position(const PlayerMessage &msg) override;
  std::optional<FlutterError> SeekTo(const PositionMessage &msg) override;
  std::optional<FlutterError> Pause(const PlayerMessage &msg) override;
  std::optional<FlutterError> SetMixWithOthers(
      const MixWithOthersMessage &msg) override;
  std::optional<FlutterError> SetDisplayGeometry(
      const GeometryMessage &msg) override;

  void SetLicenseData(void *response_data, size_t response_len,
                      int64_t player_id);

 private:
  void DisposeAllPlayers();

  FlutterDesktopPluginRegistrarRef registrar_ref_;
  flutter::PluginRegistrar *plugin_registrar_;
  VideoPlayerOptions options_;
  std::map<int64_t, std::unique_ptr<VideoPlayer>> players_;
};

std::unique_ptr<VideoPlayerTizenPlugin> plugin_;

void VideoPlayerTizenPlugin::RegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar) {
  plugin_ =
      std::make_unique<VideoPlayerTizenPlugin>(registrar_ref, plugin_registrar);
}

VideoPlayerTizenPlugin::VideoPlayerTizenPlugin(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar)
    : registrar_ref_(registrar_ref), plugin_registrar_(plugin_registrar) {
  VideoPlayerApi::SetUp(plugin_registrar->messenger(), this);
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
  return std::nullopt;
}

ErrorOr<PlayerMessage> VideoPlayerTizenPlugin::Create(
    const CreateMessage &msg) {
  FlutterDesktopViewRef flutter_view =
      FlutterDesktopPluginRegistrarGetView(registrar_ref_);
  if (!flutter_view) {
    return FlutterError("Operation failed", "Could not get a Flutter view.");
  }
  void *native_window = FlutterDesktopViewGetNativeHandle(flutter_view);
  if (!native_window) {
    return FlutterError("Operation failed",
                        "Could not get a native window handle.");
  }
  std::unique_ptr<VideoPlayer> player =
      std::make_unique<VideoPlayer>(plugin_registrar_, native_window, msg);
  player->GetChallengeData(ChallengeCb);

  int64_t player_id = player->Create();
  if (player_id == -1) {
    return FlutterError("Operation failed", "Failed to create a player.");
  }
  players_[player_id] = std::move(player);

  PlayerMessage result;
  result.set_player_id(player_id);
  return result;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Dispose(
    const PlayerMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter != players_.end()) {
    iter->second->Dispose();
    players_.erase(iter);
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetLooping(
    const LoopingMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  iter->second->SetLooping(msg.is_looping());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetVolume(
    const VolumeMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  iter->second->SetVolume(msg.volume());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetPlaybackSpeed(
    const PlaybackSpeedMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  iter->second->SetPlaybackSpeed(msg.speed());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Play(
    const PlayerMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  iter->second->Play();

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Pause(
    const PlayerMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  iter->second->Pause();

  return std::nullopt;
}

ErrorOr<PositionMessage> VideoPlayerTizenPlugin::Position(
    const PlayerMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }

  PositionMessage result;
  result.set_player_id(msg.player_id());
  result.set_position(iter->second->GetPosition());
  return result;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SeekTo(
    const PositionMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  iter->second->SeekTo(msg.position());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetDisplayGeometry(
    const GeometryMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  iter->second->SetDisplayRoi(msg.x(), msg.y(), msg.width(), msg.height());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetMixWithOthers(
    const MixWithOthersMessage &msg) {
  options_.SetMixWithOthers(msg.mix_with_others());
  return std::nullopt;
}

void VideoPlayerTizenPlugin::SetLicenseData(void *response_data,
                                            size_t response_len,
                                            int64_t player_id) {
  auto iter = players_.find(player_id);
  if (iter != players_.end()) {
    iter->second->SetLicenseData(response_data, response_len);
  }
}

std::map<int64_t, Dart_Port> send_ports_;

}  // namespace

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}

intptr_t VideoPlayerTizenPluginInitDartApi(void *data) {
  return Dart_InitializeApiDL(data);
}

void VideoPlayerTizenPluginRegisterSendPort(int64_t player_id,
                                            Dart_Port send_port) {
  send_ports_[player_id] = send_port;
}

intptr_t ChallengeCb(uint8_t *challenge_data, size_t challenge_len,
                     int64_t player_id) {
  auto iter = send_ports_.find(player_id);
  if (iter == send_ports_.end()) {
    return 0;
  }
  Dart_Port send_port = iter->second;

  const char *methodname = "onLicenseChallenge";
  intptr_t result = 0;
  size_t request_length = challenge_len;
  void *request_buffer = malloc(request_length);
  memcpy(request_buffer, challenge_data, challenge_len);

  void *response_buffer = nullptr;
  size_t response_length = 0;

  PendingCall pending_call(&response_buffer, &response_length);

  Dart_CObject c_send_port;
  c_send_port.type = Dart_CObject_kSendPort;
  c_send_port.value.as_send_port.id = pending_call.port();
  c_send_port.value.as_send_port.origin_id = ILLEGAL_PORT;

  Dart_CObject c_pending_call;
  c_pending_call.type = Dart_CObject_kInt64;
  c_pending_call.value.as_int64 = reinterpret_cast<int64_t>(&pending_call);

  Dart_CObject c_method_name;
  c_method_name.type = Dart_CObject_kString;
  c_method_name.value.as_string = const_cast<char *>(methodname);

  Dart_CObject c_request_data;
  c_request_data.type = Dart_CObject_kExternalTypedData;
  c_request_data.value.as_external_typed_data.type = Dart_TypedData_kUint8;
  c_request_data.value.as_external_typed_data.length = request_length;
  c_request_data.value.as_external_typed_data.data =
      static_cast<uint8_t *>(request_buffer);
  c_request_data.value.as_external_typed_data.peer = request_buffer;
  c_request_data.value.as_external_typed_data.callback =
      [](void *isolate_callback_data, void *peer) { free(peer); };

  Dart_CObject *c_request_arr[] = {
      &c_send_port,
      &c_pending_call,
      &c_method_name,
      &c_request_data,
  };
  Dart_CObject c_request;
  c_request.type = Dart_CObject_kArray;
  c_request.value.as_array.values = c_request_arr;
  c_request.value.as_array.length =
      sizeof(c_request_arr) / sizeof(c_request_arr[0]);

  pending_call.PostAndWait(send_port, &c_request);
  LOG_INFO("Received result");

  plugin_->SetLicenseData(response_buffer, response_length, player_id);
  if (response_length != 0) {
    result = 1;
  }

  return result;
}
