// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <cinttypes>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <variant>

#include "ffi_messages.h"
#include "media_player.h"
#include "video_player.h"
#include "video_player_options.h"

namespace video_player_videohole_tizen {

// ===== Global State =====
// Player registry - manages all player instances using shared_ptr for thread safety
static std::map<int64_t, std::shared_ptr<VideoPlayer>> g_players;
static std::shared_mutex g_players_mutex;  // Read-write lock for better concurrency

// Global resources needed for player creation
static FlutterDesktopPluginRegistrarRef g_registrar_ref = nullptr;
static flutter::PluginRegistrar* g_plugin_registrar = nullptr;
static VideoPlayerOptions g_options;

// Helper function to get player by ID (returns shared_ptr, caller holds reference)
static std::shared_ptr<VideoPlayer> GetPlayer(int64_t player_id) {
  std::shared_lock<std::shared_mutex> lock(g_players_mutex);  // Read lock
  auto iter = g_players.find(player_id);
  if (iter != g_players.end()) {
    return iter->second;  // Copy shared_ptr, reference count +1
  }
  return nullptr;
}

// Helper function to parse simple JSON-like string to flutter::EncodableMap
static flutter::EncodableMap ParseJsonMap(const std::string& json_str) {
  flutter::EncodableMap result;
  if (json_str.empty() || json_str == "{}") {
    return result;
  }

  std::string content = json_str;
  if (content.front() == '{') content = content.substr(1);
  if (content.back() == '}') content.pop_back();

  size_t pos = 0;
  while (pos < content.length()) {
    while (pos < content.length() &&
           (isspace(content[pos]) || content[pos] == ',')) {
      pos++;
    }
    if (pos >= content.length()) break;

    if (content[pos] != '"') break;
    pos++;
    size_t key_start = pos;
    while (pos < content.length() && content[pos] != '"') pos++;
    std::string key = content.substr(key_start, pos - key_start);
    pos++;

    while (pos < content.length() &&
           (isspace(content[pos]) || content[pos] == ':')) {
      pos++;
    }
    if (pos >= content.length()) break;

    std::string value;
    if (content[pos] == '"') {
      pos++;
      size_t val_start = pos;
      while (pos < content.length() && content[pos] != '"') pos++;
      value = content.substr(val_start, pos - val_start);
      pos++;
      result[flutter::EncodableValue(key)] = flutter::EncodableValue(value);
    } else if (content[pos] == '{') {
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
      size_t val_start = pos;
      while (pos < content.length() && content[pos] != ',' &&
             content[pos] != '}') {
        pos++;
      }
      value = content.substr(val_start, pos - val_start);
      while (!value.empty() && isspace(value.back())) value.pop_back();

      if (value == "true") {
        result[flutter::EncodableValue(key)] = flutter::EncodableValue(true);
      } else if (value == "false") {
        result[flutter::EncodableValue(key)] = flutter::EncodableValue(false);
      } else {
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
static bool ExtractStringValue(const std::string& json_str,
                               const std::string& key, std::string& out_value) {
  std::string search_key = "\"" + key + "\"";
  size_t pos = json_str.find(search_key);
  if (pos == std::string::npos) return false;

  pos = json_str.find(':', pos);
  if (pos == std::string::npos) return false;

  pos++;
  while (pos < json_str.length() &&
         (isspace(json_str[pos]) || json_str[pos] == ':'))
    pos++;
  if (pos >= json_str.length() || json_str[pos] != '"') return false;

  pos++;
  size_t end = pos;
  while (end < json_str.length() && json_str[end] != '"') end++;

  out_value = json_str.substr(pos, end - pos);
  return true;
}

// Helper function to extract a nested object from JSON by key
static std::string ExtractObjectValue(const std::string& json_str,
                                      const std::string& key) {
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
    if (json_str[pos] == '{')
      brace_count++;
    else if (json_str[pos] == '}')
      brace_count--;
    pos++;
  }
  return json_str.substr(start, pos - start);
}

// Unified function to parse CreateMessage from JSON string
static video_player_videohole_tizen::CreateMessage ParseCreateMessage(
    const std::string& json_str) {
  using video_player_videohole_tizen::CreateMessage;
  using flutter::EncodableMap;
  using flutter::EncodableValue;

  video_player_videohole_tizen::CreateMessage msg;

  if (json_str.empty() || json_str == "{}") {
    return msg;
  }

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

  std::string nested_json;

  nested_json = ExtractObjectValue(json_str, "httpHeaders");
  if (!nested_json.empty()) {
    flutter::EncodableMap headers = ParseJsonMap(nested_json);
    if (!headers.empty()) {
      msg.set_http_headers(headers);
    }
  }

  nested_json = ExtractObjectValue(json_str, "playerOptions");
  if (!nested_json.empty()) {
    flutter::EncodableMap options = ParseJsonMap(nested_json);
    if (!options.empty()) {
      msg.set_player_options(options);
    }
  }

  nested_json = ExtractObjectValue(json_str, "drmConfigs");
  if (!nested_json.empty()) {
    int64_t drm_type = 0;
    std::string license_url;

    size_t drm_pos = nested_json.find("\"drmType\"");
    if (drm_pos != std::string::npos) {
      drm_pos = nested_json.find(':', drm_pos);
      if (drm_pos != std::string::npos) {
        drm_pos++;
        while (drm_pos < nested_json.length() &&
               (isspace(nested_json[drm_pos]) || nested_json[drm_pos] == ':'))
          drm_pos++;
        size_t end = drm_pos;
        while (end < nested_json.length() && nested_json[end] != ',' &&
               nested_json[end] != '}' && nested_json[end] != '"')
          end++;
        std::string val = nested_json.substr(drm_pos, end - drm_pos);
        while (!val.empty() && (val.back() == '"' || isspace(val.back())))
          val.pop_back();
        try {
          drm_type = std::stoll(val);
        } catch (...) {
          if (val == "1")
            drm_type = 1;
          else if (val == "2")
            drm_type = 2;
        }
      }
    }

    ExtractStringValue(nested_json, "licenseServerUrl", license_url);

    flutter::EncodableMap drm_map;
    drm_map[flutter::EncodableValue("drmType")] =
        flutter::EncodableValue(drm_type);
    if (!license_url.empty()) {
      drm_map[flutter::EncodableValue("licenseServerUrl")] =
          flutter::EncodableValue(license_url);
    }
    msg.set_drm_configs(drm_map);
  }

  return msg;
}

// ===== C interface for plugin registration =====

extern "C" {

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar_ref) {
  auto* plugin_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar_ref);

  // Store global references for FFI functions
  g_registrar_ref = registrar_ref;
  g_plugin_registrar = plugin_registrar;
}

}  // extern "C"

}  // namespace video_player_videohole_tizen

// ===== FFI Implementation =====

#include <dart_api_dl.h>
#include <stdlib.h>
#include <string.h>

using video_player_videohole_tizen::CreateMessage;
using video_player_videohole_tizen::GetPlayer;
using video_player_videohole_tizen::g_dart_port;
using video_player_videohole_tizen::g_dart_port_mutex;
using video_player_videohole_tizen::g_options;
using video_player_videohole_tizen::g_plugin_registrar;
using video_player_videohole_tizen::g_players;
using video_player_videohole_tizen::g_players_mutex;
using video_player_videohole_tizen::g_registrar_ref;
using video_player_videohole_tizen::ParseCreateMessage;
using video_player_videohole_tizen::VideoPlayer;

extern "C" {

int ffi_initialize() {
  std::unique_lock<std::shared_mutex> lock(g_players_mutex);
  g_players.clear();
  return 0;
}

int64_t ffi_create(const char* create_message_json) {
  if (g_registrar_ref == nullptr || g_plugin_registrar == nullptr) {
    return -1;
  }

  FlutterDesktopViewRef view = FlutterDesktopPluginRegistrarGetView(g_registrar_ref);
  if (!view) {
    return -1;
  }

  CreateMessage msg;
  if (create_message_json != nullptr && strlen(create_message_json) > 0) {
    msg = ParseCreateMessage(std::string(create_message_json));
  }

  std::string uri;
  if (msg.asset() && !msg.asset()->empty()) {
    char* res_path = app_get_resource_path();
    if (res_path) {
      uri = uri + res_path + "flutter_assets/" + *msg.asset();
      free(res_path);
    } else {
      return -1;
    }
  } else if (msg.uri() && !msg.uri()->empty()) {
    uri = *msg.uri();
  } else {
    return -1;
  }

  auto player = std::make_unique<video_player_videohole_tizen::MediaPlayer>(
      g_plugin_registrar->messenger(), view);
  int64_t player_id = player->Create(uri, msg);
  if (player_id == -1) {
    return -1;
  }

  std::unique_lock<std::shared_mutex> lock(g_players_mutex);
  g_players[player_id] = std::move(player);
  return player_id;
}

int ffi_dispose(int64_t player_id) {
  std::unique_lock<std::shared_mutex> lock(g_players_mutex);
  auto iter = g_players.find(player_id);
  if (iter != g_players.end()) {
    iter->second->Dispose();
    g_players.erase(iter);
  }
  return 0;
}

int ffi_play(int64_t player_id) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->Play() ? 0 : -1;
}

int ffi_pause(int64_t player_id) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->Pause() ? 0 : -1;
}

int ffi_seek_to(int64_t player_id, int64_t position_ms) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  player->SeekTo(position_ms, []() -> void {
    // Seek completed callback - events are sent via FFI event port
  });
  return 0;
}

int64_t ffi_get_position(int64_t player_id) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->GetPosition();
}

const char* ffi_get_duration(int64_t player_id) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return strdup("-1");
  }

  auto duration_pair = player->GetDuration();
  std::string duration_json = "{\"start\":" + std::to_string(duration_pair.first) +
                              ",\"end\":" + std::to_string(duration_pair.second) + "}";
  return strdup(duration_json.c_str());
}

int ffi_set_volume(int64_t player_id, double volume) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->SetVolume(volume) ? 0 : -1;
}

int ffi_set_playback_speed(int64_t player_id, double speed) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->SetPlaybackSpeed(speed) ? 0 : -1;
}

int ffi_set_looping(int64_t player_id, bool is_looping) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->SetLooping(is_looping) ? 0 : -1;
}

const char* ffi_get_track_info(int64_t player_id, const char* track_type) {
  auto player = GetPlayer(player_id);
  if (!player || track_type == nullptr) {
    return nullptr;
  }

  auto tracks = player->GetTrackInfo(std::string(track_type));
  std::string track_info_json = "{\"playerId\":" + std::to_string(player_id) +
                                ",\"tracks\":[";

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

int ffi_set_track_selection(int64_t player_id, int64_t track_id,
                            const char* track_type) {
  auto player = GetPlayer(player_id);
  if (!player || track_type == nullptr) {
    return -1;
  }
  return player->SetTrackSelection(track_id, std::string(track_type)) ? 0 : -1;
}

int ffi_set_display_geometry(int64_t player_id, int32_t x, int32_t y,
                             int32_t width, int32_t height) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  player->SetDisplayRoi(x, y, width, height);
  return 0;
}

int ffi_set_display_rotate(int64_t player_id, int32_t rotation) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->SetDisplayRotate(rotation) ? 0 : -1;
}

int ffi_suspend(int64_t player_id) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->Suspend() ? 0 : -1;
}

int ffi_restore(int64_t player_id, const char* create_message_json,
                int64_t resume_time) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }

  CreateMessage msg;
  if (create_message_json != nullptr && strlen(create_message_json) > 0) {
    msg = ParseCreateMessage(std::string(create_message_json));
  }

  return player->Restore(&msg, resume_time) ? 0 : -1;
}

int ffi_set_activate(int64_t player_id) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->Activate() ? 0 : -1;
}

int ffi_set_deactivate(int64_t player_id) {
  auto player = GetPlayer(player_id);
  if (!player) {
    return -1;
  }
  return player->Deactivate() ? 0 : -1;
}

int ffi_set_mix_with_others(bool mix_with_others) {
  g_options.SetMixWithOthers(mix_with_others);
  return 0;
}

// FFI event port functions
// Note: g_dart_port and g_dart_port_mutex are declared in video_player.h
static bool g_dart_api_dl_initialized = false;

int ffi_initialize_api_dl(void* data) {
  if (!g_dart_api_dl_initialized) {
    if (Dart_InitializeApiDL(data) == 0) {
      g_dart_api_dl_initialized = true;
      return 0;
    }
    return -1;
  }
  return 0;
}

void ffi_register_event_port(int64_t port) {
  std::lock_guard<std::mutex> lock(g_dart_port_mutex);
  g_dart_port = port;
}

void ffi_unregister_event_port() {
  std::lock_guard<std::mutex> lock(g_dart_port_mutex);
  g_dart_port = 0;
}

}  // extern "C"
