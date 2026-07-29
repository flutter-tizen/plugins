// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// FFI API header for video_player_tizen
// This file contains all FFI function declarations and message types

#ifndef FFI_MESSAGES_H_
#define FFI_MESSAGES_H_

#include <flutter/binary_messenger.h>
#include <flutter/encodable_value.h>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>

#ifdef __cplusplus
extern "C" {
#endif

// ===== FFI function declarations =====

int ffi_initialize();
int64_t ffi_create(const char* create_message_json);
int ffi_dispose(int64_t player_id);
int ffi_play(int64_t player_id);
int ffi_pause(int64_t player_id);
int ffi_seek_to(int64_t player_id, int64_t position_ms);
int64_t ffi_get_position(int64_t player_id);
const char* ffi_get_duration(int64_t player_id);
int ffi_set_volume(int64_t player_id, double volume);
int ffi_set_playback_speed(int64_t player_id, double speed);
int ffi_set_looping(int64_t player_id, bool is_looping);
const char* ffi_get_track_info(int64_t player_id, const char* track_type);
int ffi_set_track_selection(int64_t player_id, int64_t track_id,
                            const char* track_type);
int ffi_set_display_geometry(int64_t player_id, int32_t x, int32_t y,
                             int32_t width, int32_t height);
int ffi_set_display_rotate(int64_t player_id, int32_t rotation);
int ffi_suspend(int64_t player_id);
// P0-3 fix: restore returns int (0 on success, -1 on failure)
// Player ID remains unchanged after restore
int ffi_restore(int64_t player_id, const char* create_message_json,
                int64_t resume_time);
int ffi_set_activate(int64_t player_id);
int ffi_set_deactivate(int64_t player_id);
int ffi_set_mix_with_others(bool mix_with_others);

// FFI event port functions
int ffi_initialize_api_dl(void* data);
void ffi_register_event_port(int64_t port);
void ffi_unregister_event_port();

// P0-1 fix: FFI string memory management
void ffi_free_string(char* ptr);

// P0-2 fix: Per-player event port registration
void ffi_register_player_event_port(int64_t player_id, int64_t port);
void ffi_unregister_player_event_port(int64_t player_id);

#ifdef __cplusplus
}  // extern "C"

// ===== Message types for C++ usage =====

namespace video_player_videohole_tizen {

// CreateMessage - player creation parameters (used by FFI create/restore)
class CreateMessage {
 public:
  CreateMessage() = default;

  const std::optional<std::string>& asset() const { return asset_; }
  void set_asset(const std::string& value) { asset_ = value; }

  const std::optional<std::string>& uri() const { return uri_; }
  void set_uri(const std::string& value) { uri_ = value; }

  const std::optional<std::string>& package_name() const {
    return package_name_;
  }
  void set_package_name(const std::string& value) { package_name_ = value; }

  const std::optional<std::string>& format_hint() const { return format_hint_; }
  void set_format_hint(const std::string& value) { format_hint_ = value; }

  const flutter::EncodableMap& http_headers() const { return http_headers_; }
  void set_http_headers(const flutter::EncodableMap& value) {
    http_headers_ = value;
  }

  const flutter::EncodableMap& drm_configs() const { return drm_configs_; }
  void set_drm_configs(const flutter::EncodableMap& value) {
    drm_configs_ = value;
  }

  const flutter::EncodableMap& player_options() const {
    return player_options_;
  }
  void set_player_options(const flutter::EncodableMap& value) {
    player_options_ = value;
  }

 private:
  std::optional<std::string> asset_;
  std::optional<std::string> uri_;
  std::optional<std::string> package_name_;
  std::optional<std::string> format_hint_;
  flutter::EncodableMap http_headers_;
  flutter::EncodableMap drm_configs_;
  flutter::EncodableMap player_options_;
};

}  // namespace video_player_videohole_tizen

#endif  // __cplusplus
#endif  // FFI_MESSAGES_H_
