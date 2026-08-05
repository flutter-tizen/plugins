// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <sstream>

#include "../third_party/json.hpp"
#include "log.h"

// For Dart_Port and Dart_PostCObject_DL
#include <dart_api_dl.h>

using nlohmann::json;

namespace video_player_videohole_tizen {

// Global Dart port for all player events
static int64_t g_dart_port = -1;
static std::mutex g_dart_port_mutex;

// Register global Dart port for all player events
void RegisterDartPort(int64_t dart_port) {
  std::lock_guard<std::mutex> lock(g_dart_port_mutex);
  g_dart_port = dart_port;
  LOG_INFO("[VideoPlayer] Registered global port %lld",
           static_cast<long long>(dart_port));
}

// Unregister global Dart port
void UnregisterDartPort() {
  std::lock_guard<std::mutex> lock(g_dart_port_mutex);
  g_dart_port = -1;
  LOG_INFO("[VideoPlayer] Unregistered global port");
}

// Post event to Dart using global port
// Note: Copy port under lock, then release lock BEFORE calling
// Dart_PostCObject_DL to avoid potential deadlock if Dart_PostCObject_DL blocks
// or callbacks
void PostEventToDart(int64_t player_id, const std::string& event_json) {
  int64_t port;
  {
    std::lock_guard<std::mutex> lock(g_dart_port_mutex);
    if (g_dart_port < 0) {
      LOG_ERROR("[VideoPlayer] Global port not registered, dropping event");
      return;
    }
    port = g_dart_port;
  }

  Dart_CObject player_id_obj;
  player_id_obj.type = Dart_CObject_kInt64;
  player_id_obj.value.as_int64 = player_id;

  char* json_copy = strdup(event_json.c_str());

  Dart_CObject event_json_obj;
  event_json_obj.type = Dart_CObject_kString;
  event_json_obj.value.as_string = json_copy;

  // Array elements (must be pointers)
  Dart_CObject* array_elements[2];
  array_elements[0] = &player_id_obj;
  array_elements[1] = &event_json_obj;

  // The message array
  Dart_CObject message;
  message.type = Dart_CObject_kArray;
  message.value.as_array.length = 2;
  message.value.as_array.values = array_elements;

  bool result = Dart_PostCObject_DL(port, &message);
  free(json_copy);

  if (!result) {
    LOG_ERROR(
        "[VideoPlayer] Failed to post event to Dart for player %lld. Freeing "
        "json_copy.",
        static_cast<long long>(player_id));
  }
}

// Fix P1 #4: Use nlohmann::json for proper string escaping

// Forward declaration
static json EncodableValueToJsonJson(const flutter::EncodableValue& value);

// Convert EncodableMap to nlohmann::json
static json EncodableMapToJsonJson(const flutter::EncodableMap& map) {
  json j = json::object();
  for (const auto& [key, val] : map) {
    std::string key_str;
    if (std::holds_alternative<std::string>(key)) {
      key_str = std::get<std::string>(key);
    } else if (std::holds_alternative<int32_t>(key)) {
      key_str = std::to_string(std::get<int32_t>(key));
    } else if (std::holds_alternative<int64_t>(key)) {
      key_str = std::to_string(std::get<int64_t>(key));
    }
    j[key_str] = EncodableValueToJsonJson(val);
  }
  return j;
}

// Convert EncodableList to nlohmann::json
static json EncodableListToJsonJson(const flutter::EncodableList& list) {
  json j = json::array();
  for (const auto& item : list) {
    j.push_back(EncodableValueToJsonJson(item));
  }
  return j;
}

// Convert EncodableValue to nlohmann::json (helper for nested types)
static json EncodableValueToJsonJson(const flutter::EncodableValue& value) {
  try {
    if (std::holds_alternative<bool>(value)) {
      return json(std::get<bool>(value));
    }
    if (std::holds_alternative<int32_t>(value)) {
      return json(std::get<int32_t>(value));
    }
    if (std::holds_alternative<int64_t>(value)) {
      return json(std::get<int64_t>(value));
    }
    if (std::holds_alternative<double>(value)) {
      return json(std::get<double>(value));
    }
    if (std::holds_alternative<std::string>(value)) {
      return json(std::get<std::string>(value));
    }
    if (std::holds_alternative<flutter::EncodableList>(value)) {
      return EncodableListToJsonJson(std::get<flutter::EncodableList>(value));
    }
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      return EncodableMapToJsonJson(std::get<flutter::EncodableMap>(value));
    }
    // Default for null/monostate or any unknown type
    return json(nullptr);
  } catch (const std::bad_variant_access& e) {
    return json(nullptr);
  }
}

// Convert EncodableValue to JSON string with proper escaping
static std::string EncodableValueToJson(const flutter::EncodableValue& value) {
  try {
    if (std::holds_alternative<bool>(value)) {
      return json(std::get<bool>(value)).dump();
    }
    if (std::holds_alternative<int32_t>(value)) {
      return json(std::get<int32_t>(value)).dump();
    }
    if (std::holds_alternative<int64_t>(value)) {
      return json(std::get<int64_t>(value)).dump();
    }
    if (std::holds_alternative<double>(value)) {
      return json(std::get<double>(value)).dump();
    }
    if (std::holds_alternative<std::string>(value)) {
      // nlohmann::json handles string escaping automatically
      return json(std::get<std::string>(value)).dump();
    }
    if (std::holds_alternative<std::vector<uint8_t>>(value)) {
      return "[]";
    }
    if (std::holds_alternative<std::vector<int32_t>>(value)) {
      return "[]";
    }
    if (std::holds_alternative<std::vector<int64_t>>(value)) {
      return "[]";
    }
    if (std::holds_alternative<std::vector<double>>(value)) {
      return "[]";
    }
    if (std::holds_alternative<flutter::EncodableList>(value)) {
      return EncodableListToJsonJson(std::get<flutter::EncodableList>(value))
          .dump();
    }
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      return EncodableMapToJsonJson(std::get<flutter::EncodableMap>(value))
          .dump();
    }
    if (std::holds_alternative<std::vector<float>>(value)) {
      return "[]";
    }
    // Default for null/monostate or any unknown type
    return "null";
  } catch (const std::bad_variant_access& e) {
    return "null";
  }
}

VideoPlayer::VideoPlayer(flutter::BinaryMessenger* messenger,
                         FlutterDesktopViewRef flutter_view)
    : player_id_(-1),  // Will be set in SetUpEventChannel.
      ecore_wl2_window_proxy_(std::make_unique<EcoreWl2WindowProxy>()),
      binary_messenger_(messenger),
      flutter_view_(flutter_view) {
  // Initialize GMainContext and event dispatch state
  main_context_ = std::unique_ptr<GMainContext, GMainContextDeleter>(
      g_main_context_ref_thread_default());
  event_dispatch_state_ = std::make_shared<VideoPlayer::EventDispatchState>();
  event_dispatch_state_->player = this;
}

VideoPlayer::~VideoPlayer() {
  // Mark event dispatch state as disposed and cancel pending event source
  if (event_dispatch_state_) {
    std::lock_guard<std::mutex> lock(event_dispatch_state_->mutex);
    event_dispatch_state_->disposed = true;
    event_dispatch_state_->player = nullptr;

    if (event_dispatch_state_->pending_source_id != 0) {
      g_source_remove(event_dispatch_state_->pending_source_id);
      event_dispatch_state_->pending_source_id = 0;
    }
  }

  main_context_.reset();
}

void VideoPlayer::ResetEventDispatchState() {
  if (event_dispatch_state_) {
    std::lock_guard<std::mutex> lock(event_dispatch_state_->mutex);
    event_dispatch_state_->disposed = false;
    event_dispatch_state_->player = this;
    event_dispatch_state_->pending_source_id = 0;
    LOG_INFO("[VideoPlayer] ResetEventDispatchState: player=%p, this=%p",
             event_dispatch_state_->player, this);
  }
}

bool VideoPlayer::IsDisposed() const {
  if (event_dispatch_state_) {
    std::lock_guard<std::mutex> lock(event_dispatch_state_->mutex);
    return event_dispatch_state_->disposed;
  }
  return true;  // If no event_dispatch_state_, consider as disposed
}

void VideoPlayer::ExecuteSinkEvents() {
  // Double-check: make sure we're not disposed
  if (event_dispatch_state_) {
    std::lock_guard<std::mutex> state_lock(event_dispatch_state_->mutex);
    if (event_dispatch_state_->disposed) {
      LOG_ERROR("[VideoPlayer] ExecuteSinkEvents: disposed, dropping events");
      return;
    }
  }

  // Step 1: Collect regular events while holding the lock
  std::vector<flutter::EncodableValue> regular_events;
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!encodable_event_queue_.empty()) {
      regular_events.push_back(encodable_event_queue_.front());
      encodable_event_queue_.pop();
    }
  }

  // Step 2: Send regular events (no lock needed)
  int event_count = 0;
  for (const auto& event : regular_events) {
    std::string event_json = EncodableValueToJson(event);

    // Extract event type for logging
    std::string event_type = "unknown";
    if (event_json.find("\"event\":\"initialized\"") != std::string::npos) {
      event_type = "initialized";
    } else if (event_json.find("\"event\":\"bufferingStart\"") !=
               std::string::npos) {
      event_type = "bufferingStart";
    } else if (event_json.find("\"event\":\"bufferingUpdate\"") !=
               std::string::npos) {
      event_type = "bufferingUpdate";
    } else if (event_json.find("\"event\":\"bufferingEnd\"") !=
               std::string::npos) {
      event_type = "bufferingEnd";
    } else if (event_json.find("\"event\":\"completed\"") !=
               std::string::npos) {
      event_type = "completed";
    }

    LOG_INFO(
        "[VideoPlayer] Sending event #%d: type=%s, player_id=%lld, "
        "json_len=%zu",
        event_count, event_type.c_str(), static_cast<long long>(player_id_),
        event_json.length());

    PostEventToDart(player_id_, event_json);
    event_count++;
  }

  LOG_INFO("[VideoPlayer] ExecuteSinkEvents: sent %d events", event_count);

  // Step 3: Collect error events while holding the lock
  std::vector<std::pair<std::string, std::string>> error_events;
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!error_event_queue_.empty()) {
      error_events.push_back(error_event_queue_.front());
      error_event_queue_.pop();
    }
  }

  // Step 4: Send error events directly using PostEventToDart (more efficient
  // than PushEvent)
  for (const auto& error : error_events) {
    flutter::EncodableMap error_map = {
        {flutter::EncodableValue("event"), flutter::EncodableValue("error")},
        {flutter::EncodableValue("code"), flutter::EncodableValue(error.first)},
        {flutter::EncodableValue("message"),
         flutter::EncodableValue(error.second)},
    };
    std::string error_json =
        EncodableValueToJson(flutter::EncodableValue(error_map));
    PostEventToDart(player_id_, error_json);
  }
}

void VideoPlayer::ScheduleSendPendingEvents() {
  std::lock_guard<std::mutex> lock(event_dispatch_state_->mutex);

  // Check conditions and deduplicate
  if (!main_context_ || !event_dispatch_state_ ||
      event_dispatch_state_->disposed ||
      event_dispatch_state_->pending_source_id != 0) {
    return;
  }

  auto* state = new std::shared_ptr<EventDispatchState>(event_dispatch_state_);

  GSource* source = g_idle_source_new();

  // CRITICAL: Set callback BEFORE attaching to main context!
  g_source_set_callback(
      source,
      [](gpointer data) -> gboolean {
        auto state = static_cast<std::shared_ptr<EventDispatchState>*>(data);
        VideoPlayer* player = nullptr;
        {
          std::lock_guard<std::mutex> lock((*state)->mutex);
          if (!(*state)->disposed && (*state)->player) {
            (*state)->pending_source_id = 0;
            player = (*state)->player;
          }
        }
        if (player) {
          player->ExecuteSinkEvents();
        }
        return G_SOURCE_REMOVE;
      },
      state,
      [](gpointer data) {
        delete static_cast<std::shared_ptr<EventDispatchState>*>(data);
      });

  event_dispatch_state_->pending_source_id =
      g_source_attach(source, main_context_.get());
  g_source_unref(source);
}

void VideoPlayer::PushEvent(flutter::EncodableValue encodable_value) {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    encodable_event_queue_.push(encodable_value);
  }
  ScheduleSendPendingEvents();
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_) {
    int32_t width = 0, height = 0;
    GetVideoSize(&width, &height);
    is_initialized_ = true;
    // Use GetDuration() to get the duration
    auto duration = GetDuration();
    flutter::EncodableList duration_range{
        flutter::EncodableValue(duration.first),
        flutter::EncodableValue(duration.second)};
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("initialized")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration_range)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)},
    };
    PushEvent(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendBufferingStart() {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("bufferingStart")},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendBufferingUpdate(int32_t value) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("bufferingUpdate")},
      {flutter::EncodableValue("value"), flutter::EncodableValue(value)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendBufferingEnd() {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("bufferingEnd")},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendSubtitleUpdate(int32_t duration,
                                     const std::string& text) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("subtitleUpdate")},
      {flutter::EncodableValue("duration"), flutter::EncodableValue(duration)},
      {flutter::EncodableValue("text"), flutter::EncodableValue(text)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendPlayCompleted() {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"), flutter::EncodableValue("completed")},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendIsPlayingState(bool is_playing) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("isPlayingStateUpdate")},
      {flutter::EncodableValue("isPlaying"),
       flutter::EncodableValue(is_playing)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendRestored() {
  if (is_restored_) {
    is_restored_ = false;
    int32_t width = 0, height = 0;
    GetVideoSize(&width, &height);
    auto duration = GetDuration();
    flutter::EncodableList duration_range{
        flutter::EncodableValue(duration.first),
        flutter::EncodableValue(duration.second)};

    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"), flutter::EncodableValue("restored")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration_range)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)},
    };
    PushEvent(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendError(const std::string& error_code,
                            const std::string& error_message) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  error_event_queue_.push(std::make_pair(error_code, error_message));
  ScheduleSendPendingEvents();
}

void* VideoPlayer::GetWindowHandle() {
  return FlutterDesktopViewGetNativeHandle(flutter_view_);
}

}  // namespace video_player_videohole_tizen
