// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/encodable_value.h>
#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <cinttypes>
#include <cstdint>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>

#include "media_player.h"
#include "messages.h"
#include "video_player.h"
#include "video_player_options.h"

namespace video_player_videohole_tizen {

class VideoPlayerTizenPlugin : public flutter::Plugin,
                               public VideoPlayerVideoholeApi {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar);

  // Get singleton instance for FFI access
  static VideoPlayerTizenPlugin *GetInstance() { return instance_; }

  VideoPlayerTizenPlugin(FlutterDesktopPluginRegistrarRef registrar_ref,
                         flutter::PluginRegistrar *plugin_registrar);
  virtual ~VideoPlayerTizenPlugin();

  std::optional<FlutterError> Initialize() override;
  ErrorOr<PlayerMessage> Create(const CreateMessage &msg) override;
  std::optional<FlutterError> Dispose(const PlayerMessage &msg) override;
  ErrorOr<DurationMessage> Duration(const PlayerMessage &msg) override;
  std::optional<FlutterError> SetLooping(const LoopingMessage &msg) override;
  std::optional<FlutterError> SetVolume(const VolumeMessage &msg) override;
  std::optional<FlutterError> SetPlaybackSpeed(
      const PlaybackSpeedMessage &msg) override;
  ErrorOr<TrackMessage> Track(const TrackTypeMessage &msg) override;
  ErrorOr<bool> SetTrackSelection(const SelectedTracksMessage &msg) override;
  std::optional<FlutterError> Play(const PlayerMessage &msg) override;
  ErrorOr<bool> SetDeactivate(const PlayerMessage &msg) override;
  ErrorOr<bool> SetActivate(const PlayerMessage &msg) override;
  ErrorOr<PositionMessage> Position(const PlayerMessage &msg) override;
  void SeekTo(
      const PositionMessage &msg,
      std::function<void(std::optional<FlutterError> reply)> result) override;
  std::optional<FlutterError> Pause(const PlayerMessage &msg) override;
  std::optional<FlutterError> SetMixWithOthers(
      const MixWithOthersMessage &msg) override;
  std::optional<FlutterError> SetDisplayGeometry(
      const GeometryMessage &msg) override;
  std::optional<FlutterError> Suspend(int64_t player_id) override;
  std::optional<FlutterError> Restore(int64_t player_id,
                                      const CreateMessage *msg,
                                      int64_t resume_time) override;
  ErrorOr<bool> SetDisplayRotate(const RotationMessage &msg) override;

  static VideoPlayer *FindPlayerById(int64_t player_id) {
    auto iter = players_.find(player_id);
    if (iter != players_.end()) {
      return iter->second.get();
    }
    return nullptr;
  }

 private:
  void DisposeAllPlayers();
  void ParseFromCreateMessage(const CreateMessage &msg, std::string &uri,
                              int64_t &drm_type,
                              std::string &license_server_url,
                              bool &prebuffer_mode,
                              flutter::EncodableMap &http_headers);

  FlutterDesktopPluginRegistrarRef registrar_ref_;
  flutter::PluginRegistrar *plugin_registrar_;
  VideoPlayerOptions options_;

  static inline std::map<int64_t, std::unique_ptr<VideoPlayer>> players_;
  static VideoPlayerTizenPlugin *instance_;
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
  instance_ = this;  // Set singleton instance for FFI access
  VideoPlayerVideoholeApi::SetUp(plugin_registrar->messenger(), this);
}

VideoPlayerTizenPlugin::~VideoPlayerTizenPlugin() { DisposeAllPlayers(); }

void VideoPlayerTizenPlugin::DisposeAllPlayers() {
  for (const auto &[id, player] : players_) {
    player->Dispose();
  }
  players_.clear();
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Initialize() {
  DisposeAllPlayers();
  return std::nullopt;
}

ErrorOr<PlayerMessage> VideoPlayerTizenPlugin::Create(
    const CreateMessage &msg) {
  if (!FlutterDesktopPluginRegistrarGetView(registrar_ref_)) {
    return FlutterError("Operation failed", "Could not get a Flutter view.");
  }
  std::string uri;

  if (msg.asset() && !msg.asset()->empty()) {
    char *res_path = app_get_resource_path();
    if (res_path) {
      uri = uri + res_path + "flutter_assets/" + *msg.asset();
      free(res_path);
    } else {
      return FlutterError("Internal error", "Failed to get resource path.");
    }
  } else if (msg.uri() && !msg.uri()->empty()) {
    uri = *msg.uri();
  } else {
    return FlutterError("Invalid argument", "Either asset or uri must be set.");
  }

  auto player = std::make_unique<MediaPlayer>(
      plugin_registrar_->messenger(),
      FlutterDesktopPluginRegistrarGetView(registrar_ref_));
  int64_t player_id = player->Create(uri, msg);
  if (player_id == -1) {
    return FlutterError("Operation failed", "Failed to create a player.");
  }
  players_[player_id] = std::move(player);
  PlayerMessage result(player_id);
  return result;
}

ErrorOr<DurationMessage> VideoPlayerTizenPlugin::Duration(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  DurationMessage result(msg.player_id());
  auto duration_pair = player->GetDuration();
  flutter::EncodableList duration_range{
      flutter::EncodableValue(duration_pair.first),
      flutter::EncodableValue(duration_pair.second)};
  result.set_duration_range(duration_range);
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
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->SetLooping(msg.is_looping())) {
    return FlutterError("SetLooping", "Player set looping failed");
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetVolume(
    const VolumeMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->SetVolume(msg.volume())) {
    return FlutterError("SetVolume", "Player set volume failed");
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetPlaybackSpeed(
    const PlaybackSpeedMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->SetPlaybackSpeed(msg.speed())) {
    return FlutterError("SetPlaybackSpeed", "Player set playback speed failed");
  }
  return std::nullopt;
}

ErrorOr<TrackMessage> VideoPlayerTizenPlugin::Track(
    const TrackTypeMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());

  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }

  TrackMessage result(msg.player_id(), player->GetTrackInfo(msg.track_type()));
  return result;
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetTrackSelection(
    const SelectedTracksMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->SetTrackSelection(msg.track_id(), msg.track_type());
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Play(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->Play()) {
    return FlutterError("Play", "Player play failed");
  }
  return std::nullopt;
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetDeactivate(const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->Deactivate();
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetActivate(const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->Activate();
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Pause(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->Pause()) {
    return FlutterError("Pause", "Player pause failed");
  }
  return std::nullopt;
}

ErrorOr<PositionMessage> VideoPlayerTizenPlugin::Position(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  PositionMessage result(msg.player_id(), player->GetPosition());
  return result;
}

void VideoPlayerTizenPlugin::SeekTo(
    const PositionMessage &msg,
    std::function<void(std::optional<FlutterError> reply)> result) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    result(FlutterError("Invalid argument", "Player not found"));
    return;
  }
  if (!player->SeekTo(msg.position(),
                      [result]() -> void { result(std::nullopt); })) {
    result(FlutterError("SeekTo", "Player seek to failed"));
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetDisplayGeometry(
    const GeometryMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  player->SetDisplayRoi(msg.x(), msg.y(), msg.width(), msg.height());
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetMixWithOthers(
    const MixWithOthersMessage &msg) {
  options_.SetMixWithOthers(msg.mix_with_others());
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Suspend(int64_t player_id) {
  VideoPlayer *player = FindPlayerById(player_id);
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->Suspend()) {
    return FlutterError("Operation failed", "Player suspend error");
  }
  return std::nullopt;
}
std::optional<FlutterError> VideoPlayerTizenPlugin::Restore(
    int64_t player_id, const CreateMessage *msg, int64_t resume_time) {
  VideoPlayer *player = FindPlayerById(player_id);
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }

  if (!player->Restore(msg, resume_time)) {
    return FlutterError("Operation failed", "Player restore error");
  }
  return std::nullopt;
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetDisplayRotate(
    const RotationMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->SetDisplayRotate(msg.rotation());
}

}  // namespace video_player_videohole_tizen

// Static instance definition
namespace video_player_videohole_tizen {
VideoPlayerTizenPlugin *VideoPlayerTizenPlugin::instance_ = nullptr;
}  // namespace video_player_videohole_tizen

// FFI exports for gradual migration
extern "C" {

int ffi_initialize() {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->Initialize();
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// Helper function to parse simple JSON-like string to flutter::EncodableMap
// Format: {"key1":"value1","key2":"value2"}
static flutter::EncodableMap ParseJsonMap(const std::string &json_str) {
  flutter::EncodableMap result;
  if (json_str.empty() || json_str == "{}") {
    return result;
  }

  // Simple parser for flat key-value pairs
  std::string content = json_str;
  // Remove outer braces
  if (content.front() == '{') content = content.substr(1);
  if (content.back() == '}') content.pop_back();

  // Parse key-value pairs
  size_t pos = 0;
  while (pos < content.length()) {
    // Skip whitespace and commas
    while (pos < content.length() &&
           (isspace(content[pos]) || content[pos] == ',')) {
      pos++;
    }
    if (pos >= content.length()) break;

    // Find key
    if (content[pos] != '"') break;
    pos++;  // skip opening quote
    size_t key_start = pos;
    while (pos < content.length() && content[pos] != '"') pos++;
    std::string key = content.substr(key_start, pos - key_start);
    pos++;  // skip closing quote

    // Skip colon and whitespace
    while (pos < content.length() &&
           (isspace(content[pos]) || content[pos] == ':')) {
      pos++;
    }
    if (pos >= content.length()) break;

    // Find value
    std::string value;
    if (content[pos] == '"') {
      // String value
      pos++;  // skip opening quote
      size_t val_start = pos;
      while (pos < content.length() && content[pos] != '"') pos++;
      value = content.substr(val_start, pos - val_start);
      pos++;  // skip closing quote
      result[flutter::EncodableValue(key)] = flutter::EncodableValue(value);
    } else if (content[pos] == '{') {
      // Nested object - skip for now, treat as empty map
      int brace_count = 1;
      pos++;
      while (pos < content.length() && brace_count > 0) {
        if (content[pos] == '{')
          brace_count++;
        else if (content[pos] == '}')
          brace_count--;
        pos++;
      }
      result[flutter::EncodableValue(key)] = flutter::EncodableMap();
    } else {
      // Number or boolean
      size_t val_start = pos;
      while (pos < content.length() && content[pos] != ',' &&
             content[pos] != '}') {
        pos++;
      }
      value = content.substr(val_start, pos - val_start);
      // Trim whitespace
      while (!value.empty() && isspace(value.back())) value.pop_back();

      if (value == "true") {
        result[flutter::EncodableValue(key)] = flutter::EncodableValue(true);
      } else if (value == "false") {
        result[flutter::EncodableValue(key)] = flutter::EncodableValue(false);
      } else {
        // Try as number
        try {
          if (value.find('.') != std::string::npos) {
            result[flutter::EncodableValue(key)] =
                flutter::EncodableValue(std::stod(value));
          } else {
            result[flutter::EncodableValue(key)] =
                flutter::EncodableValue(std::stoll(value));
          }
        } catch (...) {
          result[flutter::EncodableValue(key)] = flutter::EncodableValue(value);
        }
      }
    }
  }

  return result;
}

// Helper function to parse JSON for drm_configs
// Format: {"drmType":1,"licenseServerUrl":"http://..."}
// Note: prebufferMode is not currently supported by DrmConfigs class
//
// Dart DrmType enum: none=0, playready=1, widevine=2
// DrmManager::DrmType enum: DRM_TYPE_NONE=0, DRM_TYPE_PLAYREADAY=1,
// DRM_TYPE_WIDEVINECDM=2 The enum values are the same, no mapping needed

static void ParseDrmConfigs(const std::string &json_str, int64_t &drm_type,
                            std::string &license_server_url) {
  drm_type = 0;
  license_server_url.clear();

  if (json_str.empty() || json_str == "{}") {
    return;
  }

  // Simple extraction
  size_t pos;

  // Extract drmType
  pos = json_str.find("\"drmType\"");
  if (pos != std::string::npos) {
    pos = json_str.find(':', pos);
    if (pos != std::string::npos) {
      pos++;
      while (pos < json_str.length() &&
             (isspace(json_str[pos]) || json_str[pos] == ':'))
        pos++;

      // Skip any leading quotes (shouldn't be there for numbers, but just in
      // case)
      while (pos < json_str.length() &&
             (json_str[pos] == '"' || isspace(json_str[pos])))
        pos++;

      size_t end = pos;
      while (end < json_str.length() && json_str[end] != ',' &&
             json_str[end] != '}' && json_str[end] != '"')
        end++;
      std::string val = json_str.substr(pos, end - pos);

      // Trim trailing whitespace and quotes
      while (!val.empty() && (val.back() == '"' || isspace(val.back())))
        val.pop_back();

      try {
        drm_type = std::stoll(val);
      } catch (const std::exception &e) {
        // Fallback: try direct comparison
        if (val == "1") {
          drm_type = 1;  // PlayReady
        } else if (val == "2") {
          drm_type = 2;  // Widevine
        }
      }
    }
  }

  // Extract licenseServerUrl - handle both string and null values
  pos = json_str.find("\"licenseServerUrl\"");
  if (pos != std::string::npos) {
    pos = json_str.find(':', pos);
    if (pos != std::string::npos) {
      pos++;
      while (pos < json_str.length() &&
             (isspace(json_str[pos]) || json_str[pos] == ':'))
        pos++;

      // Check for null value
      if (pos < json_str.length() && json_str.substr(pos, 4) == "null") {
        license_server_url.clear();
      } else if (pos < json_str.length() && json_str[pos] == '"') {
        // String value
        pos++;
        size_t end = pos;
        while (end < json_str.length() && json_str[end] != '"') end++;
        license_server_url = json_str.substr(pos, end - pos);
      }
    }
  }
}

int64_t ffi_create(const char *uri, const char *asset, const char *package_name,
                   const char *format_hint, const char *http_headers_json,
                   const char *drm_configs_json,
                   const char *player_options_json) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  // Build CreateMessage from FFI parameters
  video_player_videohole_tizen::CreateMessage msg;

  // Set basic fields
  if (uri != nullptr && strlen(uri) > 0) {
    msg.set_uri(std::string(uri));
  }
  if (asset != nullptr && strlen(asset) > 0) {
    msg.set_asset(std::string(asset));
  }
  if (package_name != nullptr && strlen(package_name) > 0) {
    msg.set_package_name(std::string(package_name));
  }
  if (format_hint != nullptr && strlen(format_hint) > 0) {
    msg.set_format_hint(std::string(format_hint));
  }

  // Parse http_headers JSON
  if (http_headers_json != nullptr && strlen(http_headers_json) > 0) {
    flutter::EncodableMap headers =
        ParseJsonMap(std::string(http_headers_json));
    if (!headers.empty()) {
      msg.set_http_headers(headers);
    }
  }

  // Parse drm_configs JSON
  if (drm_configs_json != nullptr && strlen(drm_configs_json) > 0) {
    int64_t drm_type = 0;
    std::string license_url;
    ParseDrmConfigs(std::string(drm_configs_json), drm_type, license_url);

    flutter::EncodableMap drm_map;
    drm_map[flutter::EncodableValue("drmType")] =
        flutter::EncodableValue(drm_type);
    if (!license_url.empty()) {
      drm_map[flutter::EncodableValue("licenseServerUrl")] =
          flutter::EncodableValue(license_url);
    }
    msg.set_drm_configs(drm_map);
  }

  // Parse player_options JSON
  if (player_options_json != nullptr && strlen(player_options_json) > 0) {
    flutter::EncodableMap options =
        ParseJsonMap(std::string(player_options_json));
    if (!options.empty()) {
      msg.set_player_options(options);
    }
  }

  // Use the plugin's Create method which handles asset resolution and player
  // creation
  auto result = plugin->Create(msg);
  if (result.has_error()) {
    return -1;
  }
  return result.value().player_id();
}

}  // extern "C"

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  video_player_videohole_tizen::VideoPlayerTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
