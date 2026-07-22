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
int ffi_restore(int64_t player_id, const char* create_message_json,
                int64_t resume_time);
int ffi_set_activate(int64_t player_id);
int ffi_set_deactivate(int64_t player_id);
int ffi_set_mix_with_others(bool mix_with_others);

// FFI event port functions
int ffi_initialize_api_dl(void* data);
void ffi_register_event_port(int64_t port);
void ffi_unregister_event_port();

#ifdef __cplusplus
}  // extern "C"

// ===== Message types for C++ usage =====

namespace video_player_videohole_tizen {

// Error type
class FlutterError {
 public:
  explicit FlutterError(const std::string& code) : code_(code) {}
  explicit FlutterError(const std::string& code, const std::string& message)
      : code_(code), message_(message) {}
  explicit FlutterError(const std::string& code, const std::string& message,
                        const flutter::EncodableValue& details)
      : code_(code), message_(message), details_(details) {}

  const std::string& code() const { return code_; }
  const std::string& message() const { return message_; }
  const flutter::EncodableValue& details() const { return details_; }

 private:
  std::string code_;
  std::string message_;
  flutter::EncodableValue details_;
};

// ErrorOr template type
template <class T>
class ErrorOr {
 public:
  ErrorOr(const T& rhs) : v_(rhs) {}
  ErrorOr(const T&& rhs) : v_(std::move(rhs)) {}
  ErrorOr(const FlutterError& rhs) : v_(rhs) {}
  ErrorOr(const FlutterError&& rhs) : v_(std::move(rhs)) {}

  bool has_error() const { return std::holds_alternative<FlutterError>(v_); }
  const T& value() const { return std::get<T>(v_); };
  const FlutterError& error() const { return std::get<FlutterError>(v_); };

 private:
  friend class VideoPlayerVideoholeApi;
  ErrorOr() = default;
  T TakeValue() && { return std::get<T>(std::move(v_)); }

  std::variant<T, FlutterError> v_;
};

// PlayerMessage - player identifier
class PlayerMessage {
 public:
  explicit PlayerMessage(int64_t id) : player_id_(id) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

 private:
  int64_t player_id_;
};

// LoopingMessage - looping state
class LoopingMessage {
 public:
  LoopingMessage(int64_t id, bool looping)
      : player_id_(id), is_looping_(looping) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  bool is_looping() const { return is_looping_; }
  void set_is_looping(bool value) { is_looping_ = value; }

 private:
  int64_t player_id_;
  bool is_looping_;
};

// VolumeMessage - volume level
class VolumeMessage {
 public:
  VolumeMessage(int64_t id, double vol) : player_id_(id), volume_(vol) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  double volume() const { return volume_; }
  void set_volume(double value) { volume_ = value; }

 private:
  int64_t player_id_;
  double volume_;
};

// PlaybackSpeedMessage - playback speed
class PlaybackSpeedMessage {
 public:
  PlaybackSpeedMessage(int64_t id, double speed)
      : player_id_(id), speed_(speed) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  double speed() const { return speed_; }
  void set_speed(double value) { speed_ = value; }

 private:
  int64_t player_id_;
  double speed_;
};

// TrackMessage - track information
class TrackMessage {
 public:
  TrackMessage(int64_t id, const flutter::EncodableList& tracks)
      : player_id_(id), tracks_(tracks) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  const flutter::EncodableList& tracks() const { return tracks_; }
  void set_tracks(const flutter::EncodableList& value) { tracks_ = value; }

 private:
  int64_t player_id_;
  flutter::EncodableList tracks_;
};

// TrackTypeMessage - track type identifier
class TrackTypeMessage {
 public:
  TrackTypeMessage(int64_t id, const std::string& type)
      : player_id_(id), track_type_(type) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  const std::string& track_type() const { return track_type_; }
  void set_track_type(const std::string& value) { track_type_ = value; }

 private:
  int64_t player_id_;
  std::string track_type_;
};

// SelectedTracksMessage - selected track info
class SelectedTracksMessage {
 public:
  SelectedTracksMessage(int64_t id, int64_t track_id, const std::string& type)
      : player_id_(id), track_id_(track_id), track_type_(type) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  int64_t track_id() const { return track_id_; }
  void set_track_id(int64_t value) { track_id_ = value; }

  const std::string& track_type() const { return track_type_; }
  void set_track_type(const std::string& value) { track_type_ = value; }

 private:
  int64_t player_id_;
  int64_t track_id_;
  std::string track_type_;
};

// PositionMessage - playback position
class PositionMessage {
 public:
  PositionMessage(int64_t id, int64_t pos) : player_id_(id), position_(pos) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  int64_t position() const { return position_; }
  void set_position(int64_t value) { position_ = value; }

 private:
  int64_t player_id_;
  int64_t position_;
};

// DurationMessage - duration range
class DurationMessage {
 public:
  explicit DurationMessage(int64_t id) : player_id_(id) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  const std::optional<flutter::EncodableList>& duration_range() const {
    return duration_range_;
  }
  void set_duration_range(const flutter::EncodableList& value) {
    duration_range_ = value;
  }

 private:
  int64_t player_id_;
  std::optional<flutter::EncodableList> duration_range_;
};

// GeometryMessage - display geometry (ROI)
class GeometryMessage {
 public:
  GeometryMessage(int64_t id, int32_t x, int32_t y, int32_t w, int32_t h)
      : player_id_(id), x_(x), y_(y), width_(w), height_(h) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  int32_t x() const { return x_; }
  void set_x(int32_t value) { x_ = value; }

  int32_t y() const { return y_; }
  void set_y(int32_t value) { y_ = value; }

  int32_t width() const { return width_; }
  void set_width(int32_t value) { width_ = value; }

  int32_t height() const { return height_; }
  void set_height(int32_t value) { height_ = value; }

 private:
  int64_t player_id_;
  int32_t x_, y_, width_, height_;
};

// RotationMessage - display rotation
class RotationMessage {
 public:
  RotationMessage(int64_t id, int32_t r) : player_id_(id), rotation_(r) {}

  int64_t player_id() const { return player_id_; }
  void set_player_id(int64_t value) { player_id_ = value; }

  int32_t rotation() const { return rotation_; }
  void set_rotation(int32_t value) { rotation_ = value; }

 private:
  int64_t player_id_;
  int32_t rotation_;
};

// MixWithOthersMessage - mix with others setting
class MixWithOthersMessage {
 public:
  explicit MixWithOthersMessage(bool m) : mix_with_others_(m) {}

  bool mix_with_others() const { return mix_with_others_; }
  void set_mix_with_others(bool value) { mix_with_others_ = value; }

 private:
  bool mix_with_others_;
};

// CreateMessage - player creation parameters
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
