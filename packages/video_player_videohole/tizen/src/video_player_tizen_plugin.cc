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
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <shared_mutex>

#include "media_player.h"
#include "video_player.h"
#include "video_player_options.h"

namespace video_player_videohole_tizen {

class VideoPlayerTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar);

  // Get singleton instance for FFI access
  static VideoPlayerTizenPlugin *GetInstance() { return instance_; }

  VideoPlayerTizenPlugin(FlutterDesktopPluginRegistrarRef registrar_ref,
                         flutter::PluginRegistrar *plugin_registrar);
  virtual ~VideoPlayerTizenPlugin();

  std::optional<FlutterError> Initialize();
  ErrorOr<PlayerMessage> Create(const CreateMessage &msg);
  std::optional<FlutterError> Dispose(const PlayerMessage &msg);
  ErrorOr<DurationMessage> Duration(const PlayerMessage &msg);
  std::optional<FlutterError> SetLooping(const LoopingMessage &msg);
  std::optional<FlutterError> SetVolume(const VolumeMessage &msg);
  std::optional<FlutterError> SetPlaybackSpeed(
      const PlaybackSpeedMessage &msg);
  ErrorOr<TrackMessage> Track(const TrackTypeMessage &msg);
  ErrorOr<bool> SetTrackSelection(const SelectedTracksMessage &msg);
  std::optional<FlutterError> Play(const PlayerMessage &msg);
  ErrorOr<bool> SetDeactivate(const PlayerMessage &msg);
  ErrorOr<bool> SetActivate(const PlayerMessage &msg);
  ErrorOr<PositionMessage> Position(const PlayerMessage &msg);
  void SeekTo(
      const PositionMessage &msg,
      std::function<void(std::optional<FlutterError> reply)> result);
  std::optional<FlutterError> Pause(const PlayerMessage &msg);
  std::optional<FlutterError> SetMixWithOthers(
      const MixWithOthersMessage &msg);
  std::optional<FlutterError> SetDisplayGeometry(
      const GeometryMessage &msg);
  std::optional<FlutterError> Suspend(int64_t player_id);
  std::optional<FlutterError> Restore(int64_t player_id,
                                      const CreateMessage *msg,
                                      int64_t resume_time);
  ErrorOr<bool> SetDisplayRotate(const RotationMessage &msg);

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
  // VideoPlayerVideoholeApi removed - all methods migrated to FFI
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

// Helper function to extract a string value from JSON by key
// Returns true if the key was found and value was extracted
static bool ExtractStringValue(const std::string &json_str, 
                               const std::string &key,
                               std::string &out_value) {
  std::string search_key = "\"" + key + "\"";
  size_t pos = json_str.find(search_key);
  if (pos == std::string::npos) return false;
  
  pos = json_str.find(':', pos);
  if (pos == std::string::npos) return false;
  
  pos++;
  while (pos < json_str.length() && (isspace(json_str[pos]) || json_str[pos] == ':')) pos++;
  if (pos >= json_str.length() || json_str[pos] != '"') return false;
  
  pos++;  // skip opening quote
  size_t end = pos;
  while (end < json_str.length() && json_str[end] != '"') end++;
  
  out_value = json_str.substr(pos, end - pos);
  return true;
}

// Helper function to extract a nested object from JSON by key
// Returns the nested object JSON string (including braces) or empty string if not found
static std::string ExtractObjectValue(const std::string &json_str, 
                                       const std::string &key) {
  std::string search_key = "\"" + key + "\"";
  size_t pos = json_str.find(search_key);
  if (pos == std::string::npos) return "";
  
  pos = json_str.find(':', pos);
  if (pos == std::string::npos) return "";
  
  pos++;
  while (pos < json_str.length() && isspace(json_str[pos])) pos++;
  if (pos >= json_str.length() || json_str[pos] != '{') return "";
  
  int brace_count = 1;
  size_t start = pos;
  pos++;
  while (pos < json_str.length() && brace_count > 0) {
    if (json_str[pos] == '{') brace_count++;
    else if (json_str[pos] == '}') brace_count--;
    pos++;
  }
  return json_str.substr(start, pos - start);
}

// Unified function to parse CreateMessage from JSON string
// JSON format: {"uri":"...","asset":"...","packageName":"...","formatHint":"...",
//               "httpHeaders":{...},"drmConfigs":{...},"playerOptions":{...}}
static video_player_videohole_tizen::CreateMessage ParseCreateMessage(
    const std::string &json_str) {
  video_player_videohole_tizen::CreateMessage msg;
  
  if (json_str.empty() || json_str == "{}") {
    return msg;
  }
  
  // Extract string fields using helper function
  std::string value;
  if (ExtractStringValue(json_str, "uri", value)) {
    msg.set_uri(value);
  }
  if (ExtractStringValue(json_str, "asset", value)) {
    msg.set_asset(value);
  }
  if (ExtractStringValue(json_str, "packageName", value)) {
    msg.set_package_name(value);
  }
  if (ExtractStringValue(json_str, "formatHint", value)) {
    msg.set_format_hint(value);
  }
  
  // Extract nested object fields using helper function
  std::string nested_json;
  
  // httpHeaders
  nested_json = ExtractObjectValue(json_str, "httpHeaders");
  if (!nested_json.empty()) {
    flutter::EncodableMap headers = ParseJsonMap(nested_json);
    if (!headers.empty()) {
      msg.set_http_headers(headers);
    }
  }
  
  // playerOptions
  nested_json = ExtractObjectValue(json_str, "playerOptions");
  if (!nested_json.empty()) {
    flutter::EncodableMap options = ParseJsonMap(nested_json);
    if (!options.empty()) {
      msg.set_player_options(options);
    }
  }
  
  // drmConfigs (special handling for drmType and licenseServerUrl)
  nested_json = ExtractObjectValue(json_str, "drmConfigs");
  if (!nested_json.empty()) {
    int64_t drm_type = 0;
    std::string license_url;
    
    // Extract drmType (number value)
    size_t drm_pos = nested_json.find("\"drmType\"");
    if (drm_pos != std::string::npos) {
      drm_pos = nested_json.find(':', drm_pos);
      if (drm_pos != std::string::npos) {
        drm_pos++;
        while (drm_pos < nested_json.length() && (isspace(nested_json[drm_pos]) || nested_json[drm_pos] == ':')) drm_pos++;
        size_t end = drm_pos;
        while (end < nested_json.length() && nested_json[end] != ',' && nested_json[end] != '}' && nested_json[end] != '"') end++;
        std::string val = nested_json.substr(drm_pos, end - drm_pos);
        while (!val.empty() && (val.back() == '"' || isspace(val.back()))) val.pop_back();
        try {
          drm_type = std::stoll(val);
        } catch (...) {
          if (val == "1") drm_type = 1;
          else if (val == "2") drm_type = 2;
        }
      }
    }
    
    // Extract licenseServerUrl using helper function
    ExtractStringValue(nested_json, "licenseServerUrl", license_url);
    
    flutter::EncodableMap drm_map;
    drm_map[flutter::EncodableValue("drmType")] = flutter::EncodableValue(drm_type);
    if (!license_url.empty()) {
      drm_map[flutter::EncodableValue("licenseServerUrl")] = flutter::EncodableValue(license_url);
    }
    msg.set_drm_configs(drm_map);
  }
  
  return msg;
}

// FFI create function - takes a single JSON string for all create parameters
// JSON format: {"uri":"...","asset":"...","packageName":"...","formatHint":"...",
//               "httpHeaders":{...},"drmConfigs":{...},"playerOptions":{...}}
// Returns: player_id on success, -1 on error
int64_t ffi_create(const char *create_message_json) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  video_player_videohole_tizen::CreateMessage msg;
  if (create_message_json != nullptr && strlen(create_message_json) > 0) {
    msg = ParseCreateMessage(std::string(create_message_json));
  }
  
  auto result = plugin->Create(msg);
  if (result.has_error()) {
    return -1;
  }
  return result.value().player_id();
}

// FFI dispose function - releases player resources
// Returns: 0 on success, -1 on error
// Safe to call multiple times with same player_id
int ffi_dispose(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  // Check if player exists before trying to dispose
  // This prevents crash when dispose is called multiple times
  auto player = video_player_videohole_tizen::VideoPlayerTizenPlugin::FindPlayerById(player_id);
  if (player == nullptr) {
    // Player already disposed or never existed - return success silently
    return 0;
  }

  auto result = plugin->Dispose(
      video_player_videohole_tizen::PlayerMessage(player_id));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI play function - starts or resumes playback
// Returns: 0 on success, -1 on error
// Note: Checks if player exists before calling to avoid crash on disposed player
int ffi_play(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  // Check if player exists before calling - avoid crash on disposed player
  auto player = video_player_videohole_tizen::VideoPlayerTizenPlugin::FindPlayerById(player_id);
  if (player == nullptr) {
    return -1;  // Player already disposed or never existed
  }

  auto result =
      plugin->Play(video_player_videohole_tizen::PlayerMessage(player_id));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI pause function - pauses playback
// Returns: 0 on success, -1 on error
// Note: Checks if player exists before calling to avoid crash on disposed player
int ffi_pause(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  // Check if player exists before calling - avoid crash on disposed player
  auto player = video_player_videohole_tizen::VideoPlayerTizenPlugin::FindPlayerById(player_id);
  if (player == nullptr) {
    return -1;  // Player already disposed or never existed
  }

  auto result =
      plugin->Pause(video_player_videohole_tizen::PlayerMessage(player_id));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI seek_to function - seeks to a specific position (in milliseconds)
// Returns: 0 on success, -1 on error
// Note: This is a fire-and-forget operation. The seek completion is notified
// through the event channel, not through return value.
int ffi_seek_to(int64_t player_id, int64_t position_ms) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  // Call SeekTo asynchronously - the result will be notified through event channel
  // We don't block here to avoid freezing the UI thread
  plugin->SeekTo(
      video_player_videohole_tizen::PositionMessage(player_id, position_ms),
      [](std::optional<video_player_videohole_tizen::FlutterError> result) {
        // Callback is invoked when seek completes
        // The event channel will notify Flutter about the seek completion
        if (result.has_value()) {
          // Log error but don't block - Flutter will handle it through events
        }
      });

  // Return immediately to allow UI to refresh
  return 0;  // Success (actual result will be notified through event channel)
}

// FFI get_position function - gets current playback position in milliseconds
// Returns: position in milliseconds (>= 0) on success, -1 on error
// Note: Checks if player exists before calling to avoid crash on disposed player
int64_t ffi_get_position(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  // Check if player exists before calling - avoid crash on disposed player
  auto player = video_player_videohole_tizen::VideoPlayerTizenPlugin::FindPlayerById(player_id);
  if (player == nullptr) {
    return -1;  // Player already disposed or never existed
  }

  auto result =
      plugin->Position(video_player_videohole_tizen::PlayerMessage(player_id));
  if (result.has_error()) {
    return -1;
  }
  return result.value().position();
}

// FFI get_duration function - gets video duration range in milliseconds
// Returns: JSON string with start and end values, or "-1" on error
// Format: {"start": <start_ms>, "end": <end_ms>}
// For live streams, start may be non-zero (e.g., {"start": 1000, "end": 5000})
// Note: Returns a malloc-allocated string that the caller must free
const char* ffi_get_duration(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return strdup("-1");  // Plugin not initialized
  }

  auto result =
      plugin->Duration(video_player_videohole_tizen::PlayerMessage(player_id));
  if (result.has_error()) {
    return strdup("-1");
  }
  
  // Return both start and end of duration range as JSON
  // Use strdup to allocate memory that persists after this function returns
  // The Dart caller is responsible for freeing this memory
  const auto& duration_range = result.value().duration_range();
  std::string duration_json;
  if (duration_range && duration_range->size() >= 2) {
    int64_t start = (*duration_range)[0].IsNull() ? 0 : 
                    std::get<int64_t>((*duration_range)[0]);
    int64_t end = (*duration_range)[1].IsNull() ? 0 : 
                  std::get<int64_t>((*duration_range)[1]);
    duration_json = "{\"start\":" + std::to_string(start) + 
                    ",\"end\":" + std::to_string(end) + "}";
  } else {
    duration_json = "{\"start\":0,\"end\":0}";
  }
  return strdup(duration_json.c_str());
}

// FFI set_volume function - sets playback volume (0.0 to 1.0)
// Returns: 0 on success, -1 on error
int ffi_set_volume(int64_t player_id, double volume) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetVolume(
      video_player_videohole_tizen::VolumeMessage(player_id, volume));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI set_playback_speed function - sets playback speed
// Returns: 0 on success, -1 on error
int ffi_set_playback_speed(int64_t player_id, double speed) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetPlaybackSpeed(
      video_player_videohole_tizen::PlaybackSpeedMessage(player_id, speed));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI set_looping function - enables or disables looping
// Returns: 0 on success, -1 on error
int ffi_set_looping(int64_t player_id, bool is_looping) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetLooping(
      video_player_videohole_tizen::LoopingMessage(player_id, is_looping));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI set_display_geometry function - sets the display geometry (ROI)
// Returns: 0 on success, -1 on error
int ffi_set_display_geometry(int64_t player_id, int32_t x, int32_t y, int32_t width, int32_t height) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetDisplayGeometry(
      video_player_videohole_tizen::GeometryMessage(player_id, x, y, width, height));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI set_display_rotate function - sets display rotation
// Returns: 0 on success, -1 on error
int ffi_set_display_rotate(int64_t player_id, int32_t rotation) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetDisplayRotate(
      video_player_videohole_tizen::RotationMessage(player_id, rotation));
  if (result.has_error()) {
    return -1;
  }
  return result.value() ? 0 : -1;
}

// FFI suspend function - suspends the player
// Returns: 0 on success, -1 on error
int ffi_suspend(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->Suspend(player_id);
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI restore function - restores a suspended player
// create_message_json: JSON string of CreateMessage or empty/null for using saved state
// Returns: 0 on success, -1 on error
int ffi_restore(int64_t player_id, const char* create_message_json, int64_t resume_time) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  // Use unified ParseCreateMessage function to parse JSON
  video_player_videohole_tizen::CreateMessage msg;
  if (create_message_json != nullptr && strlen(create_message_json) > 0) {
    msg = ParseCreateMessage(std::string(create_message_json));
  }
  
  auto result = plugin->Restore(player_id, &msg, resume_time);
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

// FFI set_activate function - activates the player
// Returns: 0 on success, -1 on error
int ffi_set_activate(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetActivate(
      video_player_videohole_tizen::PlayerMessage(player_id));
  if (result.has_error()) {
    return -1;
  }
  return result.value() ? 0 : -1;
}

// FFI set_deactivate function - deactivates the player
// Returns: 0 on success, -1 on error
int ffi_set_deactivate(int64_t player_id) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetDeactivate(
      video_player_videohole_tizen::PlayerMessage(player_id));
  if (result.has_error()) {
    return -1;
  }
  return result.value() ? 0 : -1;
}

// FFI get_track_info function - gets track information for a specific track type
// Returns: JSON string of track info on success, nullptr on error
// Format: {"playerId": <id>, "tracks": [{"trackId": <id>, ...}, ...]}
// Note: Returns a malloc-allocated string that the caller must free
const char* ffi_get_track_info(int64_t player_id, const char* track_type) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr || track_type == nullptr) {
    return nullptr;
  }

  auto result = plugin->Track(
      video_player_videohole_tizen::TrackTypeMessage(player_id, std::string(track_type)));
  if (result.has_error()) {
    return nullptr;
  }

  // Convert TrackMessage to JSON string
  // Use strdup to allocate memory that persists after this function returns
  // The Dart caller is responsible for freeing this memory
  std::string track_info_json = "{\"playerId\":" + std::to_string(result.value().player_id()) + 
                                ",\"tracks\":[";
  
  const auto& tracks = result.value().tracks();
  for (size_t i = 0; i < tracks.size(); ++i) {
    if (i > 0) track_info_json += ",";
    
    const auto& track_map = std::get<flutter::EncodableMap>(tracks[i]);
    track_info_json += "{";
    
    bool first = true;
    for (const auto& [key, value] : track_map) {
      if (!first) track_info_json += ",";
      first = false;
      
      const std::string* key_str = std::get_if<std::string>(&key);
      if (!key_str) continue;
      
      track_info_json += "\"" + *key_str + "\":";
      
      if (std::holds_alternative<int32_t>(value)) {
        track_info_json += std::to_string(std::get<int32_t>(value));
      } else if (std::holds_alternative<int64_t>(value)) {
        track_info_json += std::to_string(std::get<int64_t>(value));
      } else if (std::holds_alternative<double>(value)) {
        track_info_json += std::to_string(std::get<double>(value));
      } else if (std::holds_alternative<std::string>(value)) {
        track_info_json += "\"" + std::get<std::string>(value) + "\"";
      } else if (std::holds_alternative<bool>(value)) {
        track_info_json += std::get<bool>(value) ? "true" : "false";
      }
    }
    
    track_info_json += "}";
  }
  
  track_info_json += "]}";
  return strdup(track_info_json.c_str());
}

// FFI set_track_selection function - sets the selected track
// Returns: 0 on success, -1 on error
int ffi_set_track_selection(int64_t player_id, int64_t track_id, const char* track_type) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr || track_type == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetTrackSelection(
      video_player_videohole_tizen::SelectedTracksMessage(player_id, track_id, std::string(track_type)));
  if (result.has_error()) {
    return -1;
  }
  return result.value() ? 0 : -1;
}

// FFI set_mix_with_others function - sets whether to mix audio with other players
// Returns: 0 on success, -1 on error
int ffi_set_mix_with_others(bool mix_with_others) {
  auto *plugin =
      video_player_videohole_tizen::VideoPlayerTizenPlugin::GetInstance();
  if (plugin == nullptr) {
    return -1;  // Plugin not initialized
  }

  auto result = plugin->SetMixWithOthers(
      video_player_videohole_tizen::MixWithOthersMessage(mix_with_others));
  if (result.has_value()) {
    // Error occurred
    return -1;
  }
  return 0;  // Success
}

}  // extern "C"

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  video_player_videohole_tizen::VideoPlayerTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
