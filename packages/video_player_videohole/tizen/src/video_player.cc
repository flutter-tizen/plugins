// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <dlfcn.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <algorithm>

#include "log.h"
#include "pending_call.h"

static int64_t player_index = 1;

VideoPlayer::VideoPlayer(flutter::PluginRegistrar *plugin_registrar,
                         void *native_window)
    : plugin_registrar_(plugin_registrar), native_window_(native_window) {}

bool VideoPlayer::SetDisplay() {
  int x = 0, y = 0, width = 0, height = 0;
  void *ecore_lib_handle = dlopen("libecore_wl2.so.1", RTLD_LAZY);
  if (ecore_lib_handle) {
    FuncEcoreWl2WindowGeometryGet ecore_wl2_window_geometry_get =
        reinterpret_cast<FuncEcoreWl2WindowGeometryGet>(
            dlsym(ecore_lib_handle, "ecore_wl2_window_geometry_get"));
    if (ecore_wl2_window_geometry_get) {
      ecore_wl2_window_geometry_get(native_window_, &x, &y, &width, &height);
    } else {
      LOG_ERROR("[VideoPlayer] Symbol not found: %s", dlerror());
      dlclose(ecore_lib_handle);
      return false;
    }
    dlclose(ecore_lib_handle);
  } else {
    LOG_ERROR("[VideoPlayer] dlopen failed: %s", dlerror());
    return false;
  }

  void *player_lib_handle = dlopen("libcapi-media-player.so.0", RTLD_LAZY);
  if (player_lib_handle) {
    FuncPlayerSetEcoreWlDisplay player_set_ecore_wl_display =
        reinterpret_cast<FuncPlayerSetEcoreWlDisplay>(
            dlsym(player_lib_handle, "player_set_ecore_wl_display"));
    if (player_set_ecore_wl_display) {
      int ret =
          player_set_ecore_wl_display(player_, PLAYER_DISPLAY_TYPE_OVERLAY,
                                      native_window_, x, y, width, height);
      if (ret != PLAYER_ERROR_NONE) {
        LOG_ERROR("[VideoPlayer] player_set_ecore_wl_display failed: %s",
                  get_error_message(ret));
        dlclose(player_lib_handle);
        return false;
      }
    } else {
      LOG_ERROR("[VideoPlayer] Symbol not found: %s", dlerror());
      dlclose(ecore_lib_handle);
      return false;
    }
    dlclose(player_lib_handle);
  } else {
    LOG_ERROR("[VideoPlayer] dlopen failed: %s", dlerror());
    return false;
  }

  int ret = player_set_display_mode(player_, PLAYER_DISPLAY_MODE_DST_ROI);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_display_mode failed: %s",
              get_error_message(ret));
    return false;
  }
  return true;
}

int64_t VideoPlayer::Create(const std::string &uri, int drm_type,
                            const std::string &license_server_url) {
  LOG_INFO("[VideoPlayer] uri: %s, drm_type: %d", uri.c_str(), drm_type);

  player_id_ = player_index++;

  if (uri.empty()) {
    LOG_ERROR("[VideoPlayer] The uri must not be empty.");
    return -1;
  }

  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_create failed: %s", get_error_message(ret));
    return -1;
  }

  if (drm_type != DRM_TYPE_NONE) {
    drm_manager_ =
        std::make_unique<DrmManager>(drm_type, license_server_url, player_);
    drm_manager_->SetChallengeCallback(
        [this](const std::vector<uint8_t> &challenge) -> std::vector<uint8_t> {
          return OnLicenseChallenge(challenge);
        });

    if (!drm_manager_->InitializeDrmSession(uri)) {
      LOG_ERROR("[VideoPlayer] Failed to initialize the DRM session.");
      drm_manager_->ReleaseDrmSession();
    }
  }

  ret = player_set_uri(player_, uri.c_str());
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_uri failed: %s",
              get_error_message(ret));
    return -1;
  }

  if (!SetDisplay()) {
    LOG_ERROR("[VideoPlayer] Failed to set display.");
    return -1;
  }
  SetDisplayRoi(0, 0, 1, 1);

  ret = player_set_display_visible(player_, true);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_display_visible failed: %s",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_buffering_cb(player_, OnBuffering, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_buffering_cb failed: %s",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_completed_cb failed: %s",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_interrupted_cb(player_, OnInterrupted, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_interrupted_cb failed: %s",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_error_cb(player_, OnError, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_error_cb failed: %s",
              get_error_message(ret));
    return -1;
  }

  ret = player_prepare_async(player_, OnPrepared, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_prepare_async failed: %s",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_subtitle_updated_cb(player_, OnSubtitleUpdated, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_subtitle_updated_cb failed: %s",
              get_error_message(ret));
  }

  SetUpEventChannel(plugin_registrar_->messenger());

  return player_id_;
}

void VideoPlayer::SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                                int32_t height) {
  int ret = player_set_display_roi_area(player_, x, y, width, height);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_display_roi_area failed: %s",
              get_error_message(ret));
  }
}

VideoPlayer::~VideoPlayer() {
  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
  Dispose();
}

void VideoPlayer::Play() {
  LOG_INFO("[VideoPlayer] Player starting.");

  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] Unable to get player state.");
    return;
  }
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[VideoPlayer] Player not ready.");
    return;
  }
  if (state == PLAYER_STATE_PLAYING) {
    LOG_INFO("[VideoPlayer] Player already playing.");
    return;
  }

  ret = player_start(player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_start failed: %s", get_error_message(ret));
  }
}

void VideoPlayer::Pause() {
  LOG_INFO("[VideoPlayer] Player pausing.");

  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] Unable to get player state.");
    return;
  }
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[VideoPlayer] Player not ready.");
    return;
  }
  if (state != PLAYER_STATE_PLAYING) {
    LOG_INFO("[VideoPlayer] Player not playing.");
    return;
  }

  ret = player_pause(player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_pause failed: %s", get_error_message(ret));
  }
}

void VideoPlayer::SetLooping(bool is_looping) {
  LOG_INFO("[VideoPlayer] is_looping: %d", is_looping);

  int ret = player_set_looping(player_, is_looping);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_looping failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SetVolume(double volume) {
  LOG_INFO("[VideoPlayer] volume: %f", volume);

  int ret = player_set_volume(player_, volume, volume);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_volume failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("[VideoPlayer] speed: %f", speed);

  int ret = player_set_playback_rate(player_, speed);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_playback_rate failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SeekTo(int32_t position, SeekCompletedCallback callback) {
  LOG_INFO("[VideoPlayer] position: %d", position);

  on_seek_completed_ = std::move(callback);
  int ret =
      player_set_play_position(player_, position, true, OnSeekCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_play_position failed: %s",
              get_error_message(ret));
  }
}

int32_t VideoPlayer::GetPosition() {
  int position = 0;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_get_play_position failed: %s",
              get_error_message(ret));
  }
  return position;
}

void VideoPlayer::Dispose() {
  LOG_INFO("[VideoPlayer] Player disposing.");

  is_initialized_ = false;
  event_sink_ = nullptr;
  event_channel_->SetStreamHandler(nullptr);

  if (player_) {
    player_unprepare(player_);
    player_unset_buffering_cb(player_);
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_destroy(player_);
    player_ = nullptr;
  }
}

void VideoPlayer::SetUpEventChannel(flutter::BinaryMessenger *messenger) {
  std::string channel_name =
      "tizen/video_player/video_events_" + std::to_string(player_id_);
  auto channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          messenger, channel_name,
          &flutter::StandardMethodCodec::GetInstance());
  auto handler = std::make_unique<
      flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
      [&](const flutter::EncodableValue *arguments,
          std::unique_ptr<flutter::EventSink<>> &&events)
          -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = std::move(events);
        Initialize();
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = nullptr;
        return nullptr;
      });
  channel->SetStreamHandler(std::move(handler));

  event_channel_ = std::move(channel);
}

void VideoPlayer::Initialize() {
  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer] Player state: %d", state);
    if (state == PLAYER_STATE_READY && !is_initialized_) {
      SendInitialized();
    }
  } else {
    LOG_ERROR("[VideoPlayer] player_get_state failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_ && !is_interrupted_ && event_sink_) {
    int duration = 0;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      event_sink_->Error("player_get_duration failed", get_error_message(ret));
      return;
    }
    LOG_INFO("[VideoPlayer] Video duration: %d", duration);

    int width = 0, height = 0;
    ret = player_get_video_size(player_, &width, &height);
    if (ret != PLAYER_ERROR_NONE) {
      event_sink_->Error("player_get_video_size failed",
                         get_error_message(ret));
      return;
    }
    LOG_INFO("[VideoPlayer] Video width: %d, height: %d", width, height);

    player_display_rotation_e rotation = PLAYER_DISPLAY_ROTATION_NONE;
    ret = player_get_display_rotation(player_, &rotation);
    if (ret != PLAYER_ERROR_NONE) {
      event_sink_->Error("player_get_display_rotation failed",
                         get_error_message(ret));
    } else {
      if (rotation == PLAYER_DISPLAY_ROTATION_90 ||
          rotation == PLAYER_DISPLAY_ROTATION_270) {
        std::swap(width, height);
      }
    }

    is_initialized_ = true;
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("initialized")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)},
    };
    event_sink_->Success(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendBufferingStart() {
  if (event_sink_) {
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingStart")},
    };
    event_sink_->Success(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendBufferingUpdate(int32_t value) {
  if (event_sink_) {
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingUpdate")},
        {flutter::EncodableValue("value"), flutter::EncodableValue(value)},
    };
    event_sink_->Success(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendBufferingEnd() {
  if (event_sink_) {
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingEnd")},
    };
    event_sink_->Success(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendSubtitleUpdate(int32_t duration,
                                     const std::string &text) {
  if (event_sink_) {
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("subtitleUpdate")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration)},
        {flutter::EncodableValue("text"), flutter::EncodableValue(text)},
    };
    event_sink_->Success(flutter::EncodableValue(result));
  }
}

void VideoPlayer::OnSubtitleUpdated(unsigned long duration, char *text,
                                    void *data) {
  LOG_INFO("[VideoPlayer] duration: %ld, text: %s", duration, text);

  VideoPlayer *player = static_cast<VideoPlayer *>(data);
  player->SendSubtitleUpdate(duration, std::string(text));
}

void VideoPlayer::OnPrepared(void *data) {
  LOG_INFO("[VideoPlayer] Player prepared.");

  VideoPlayer *player = static_cast<VideoPlayer *>(data);
  if (!player->is_initialized_) {
    player->SendInitialized();
  }
}

void VideoPlayer::OnBuffering(int percent, void *data) {
  LOG_INFO("[VideoPlayer] percent: %d", percent);

  VideoPlayer *player = static_cast<VideoPlayer *>(data);
  if (percent == 100) {
    player->SendBufferingEnd();
    player->is_buffering_ = false;
  } else if (!player->is_buffering_ && percent <= 5) {
    player->SendBufferingStart();
    player->is_buffering_ = true;
  } else {
    player->SendBufferingUpdate(percent);
  }
}

void VideoPlayer::OnSeekCompleted(void *data) {
  LOG_INFO("[VideoPlayer] Seek completed.");

  VideoPlayer *player = static_cast<VideoPlayer *>(data);
  if (player->on_seek_completed_) {
    player->on_seek_completed_();
    player->on_seek_completed_ = nullptr;
  }
}

void VideoPlayer::OnPlayCompleted(void *data) {
  LOG_INFO("[VideoPlayer] Play completed.");

  VideoPlayer *player = static_cast<VideoPlayer *>(data);
  if (player->event_sink_) {
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("completed")},
    };
    player->event_sink_->Success(flutter::EncodableValue(result));
  }
  player->Pause();
}

void VideoPlayer::OnError(int error_code, void *data) {
  LOG_ERROR("[VideoPlayer] Error code: %d (%s)", error_code,
            get_error_message(error_code));

  VideoPlayer *player = static_cast<VideoPlayer *>(data);
  if (player->event_sink_) {
    player->event_sink_->Error(
        "Player error", std::string("Error: ") + get_error_message(error_code));
  }
}

void VideoPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  LOG_ERROR("[VideoPlayer] Interrupt code: %d", code);

  VideoPlayer *player = static_cast<VideoPlayer *>(data);
  player->is_interrupted_ = true;
  if (player->event_sink_) {
    player->event_sink_->Error("Player interrupted",
                               "Video player has been interrupted.");
  }
}

std::vector<uint8_t> VideoPlayer::OnLicenseChallenge(
    const std::vector<uint8_t> &challenge) {
  const char *method_name = "onLicenseChallenge";
  size_t request_length = challenge.size();
  void *request_buffer = malloc(request_length);
  memcpy(request_buffer, challenge.data(), challenge.size());

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
  c_method_name.value.as_string = const_cast<char *>(method_name);

  Dart_CObject c_request_data;
  c_request_data.type = Dart_CObject_kExternalTypedData;
  c_request_data.value.as_external_typed_data.type = Dart_TypedData_kUint8;
  c_request_data.value.as_external_typed_data.length = request_length;
  c_request_data.value.as_external_typed_data.data =
      static_cast<uint8_t *>(request_buffer);
  c_request_data.value.as_external_typed_data.peer = request_buffer;
  c_request_data.value.as_external_typed_data.callback =
      [](void *isolate_callback_data, void *peer) { free(peer); };

  Dart_CObject *c_request_arr[] = {&c_send_port, &c_pending_call,
                                   &c_method_name, &c_request_data};
  Dart_CObject c_request;
  c_request.type = Dart_CObject_kArray;
  c_request.value.as_array.values = c_request_arr;
  c_request.value.as_array.length =
      sizeof(c_request_arr) / sizeof(c_request_arr[0]);

  pending_call.PostAndWait(send_port_, &c_request);
  LOG_INFO("[ffi] Received result (size: %d)", response_length);

  return std::vector<uint8_t>(
      static_cast<uint8_t *>(response_buffer),
      static_cast<uint8_t *>(response_buffer) + response_length);
}
