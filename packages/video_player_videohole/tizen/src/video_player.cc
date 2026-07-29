// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <sstream>

#include "log.h"

// For Dart_Port and Dart_PostCObject_DL
#include <dart_api_dl.h>

namespace video_player_videohole_tizen {

static int64_t player_index = 1;

// P0-2 fix: Per-player event port registration
// Each player has its own Dart port for event notifications
static std::map<int64_t, int64_t> g_player_dart_ports;
static std::mutex g_player_ports_mutex;

// P0-2 fix: Register Dart port for a specific player
void RegisterPlayerEventPort(int64_t player_id, int64_t dart_port) {
  std::lock_guard<std::mutex> lock(g_player_ports_mutex);
  g_player_dart_ports[player_id] = dart_port;
  LOG_INFO("[VideoPlayer] Registered port %lld for player %lld",
           static_cast<long long>(dart_port),
           static_cast<long long>(player_id));
}

// P0-2 fix: Unregister Dart port for a specific player
void UnregisterPlayerEventPort(int64_t player_id) {
  std::lock_guard<std::mutex> lock(g_player_ports_mutex);
  auto it = g_player_dart_ports.find(player_id);
  if (it != g_player_dart_ports.end()) {
    g_player_dart_ports.erase(it);
    LOG_INFO("[VideoPlayer] Unregistered port for player %lld",
             static_cast<long long>(player_id));
  }
}

// P1-2 fix: Unregister all player event ports (for hot restart cleanup)
void UnregisterAllPlayerEventPorts() {
  std::lock_guard<std::mutex> lock(g_player_ports_mutex);
  g_player_dart_ports.clear();
  LOG_INFO("[VideoPlayer] Unregistered all player ports");
}

// P0-2 fix: Post event to Dart using per-player port
void PostEventToDart(int64_t player_id, const std::string& event_json) {
  std::lock_guard<std::mutex> lock(g_player_ports_mutex);
  auto it = g_player_dart_ports.find(player_id);
  if (it == g_player_dart_ports.end()) {
    LOG_DEBUG(
        "[VideoPlayer] Port not registered for player %lld, dropping event",
        static_cast<long long>(player_id));
    return;
  }

  int64_t port = it->second;

  // TEMP_DEBUG: Add detailed logging for crash investigation
  std::string event_type = "unknown";
  if (event_json.find("\"initialized\"") != std::string::npos) {
    event_type = "initialized";
  } else if (event_json.find("\"bufferingStart\"") != std::string::npos) {
    event_type = "bufferingStart";
  } else if (event_json.find("\"bufferingUpdate\"") != std::string::npos) {
    event_type = "bufferingUpdate";
  } else if (event_json.find("\"bufferingEnd\"") != std::string::npos) {
    event_type = "bufferingEnd";
  } else if (event_json.find("\"completed\"") != std::string::npos) {
    event_type = "completed";
  } else if (event_json.find("\"subtitleUpdate\"") != std::string::npos) {
    event_type = "subtitleUpdate";
  } else if (event_json.find("\"error\"") != std::string::npos) {
    event_type = "error";
  }

  // LOG_INFO("[FFI_DEBUG] Pre-PostEvent: player_id=%lld, port=%lld,
  // event_type=%s, json_len=%zu",
  //          static_cast<long long>(player_id),
  //          static_cast<long long>(port),
  //          event_type.c_str(),
  //          event_json.length());

  // Log first 200 chars of JSON for debugging (truncated to avoid log spam)
  std::string json_preview = event_json.length() > 200
                                 ? event_json.substr(0, 200) + "..."
                                 : event_json;
  // LOG_INFO("[FFI_DEBUG] JSON preview: %s", json_preview.c_str());

  // Message format: [player_id, event_json]
  Dart_CObject player_id_obj;
  player_id_obj.type = Dart_CObject_kInt64;
  player_id_obj.value.as_int64 = player_id;

  // Dart_PostCObject_DL takes ownership of the string on success
  char* json_copy = strdup(event_json.c_str());
  // LOG_INFO("[FFI_DEBUG] strdup completed, json_copy=%p", json_copy);

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

  // LOG_INFO("[FFI_DEBUG] Calling Dart_PostCObject_DL...");
  bool result = Dart_PostCObject_DL(port, &message);
  // LOG_INFO("[FFI_DEBUG] Dart_PostCObject_DL returned: %d", result ? 1 : 0);

  if (!result) {
    LOG_ERROR(
        "[VideoPlayer] Failed to post event to Dart for player %lld. Freeing "
        "json_copy.",
        static_cast<long long>(player_id));
    free(json_copy);
  }
  // On success, Dart takes ownership of json_copy
}

// Legacy FFI event callback functions removed - use per-player port
// registration instead
void VideoPlayer::RegisterFFIEventCallback(DartPortEventCallback callback) {
  // Deprecated: use RegisterPlayerEventPort instead
}

DartPortEventCallback VideoPlayer::GetFFIEventCallback() {
  // Deprecated: use RegisterPlayerEventPort instead
  return nullptr;
}

// Helper function to convert EncodableValue to JSON string
static std::string EncodableValueToJson(const flutter::EncodableValue& value);

static std::string EncodableMapToJson(const flutter::EncodableMap& map) {
  std::ostringstream oss;
  oss << "{";
  bool first = true;
  for (const auto& [key, val] : map) {
    if (!first) oss << ",";
    first = false;

    // Key
    if (std::holds_alternative<std::string>(key)) {
      oss << "\"" << std::get<std::string>(key) << "\":";
    } else if (std::holds_alternative<int32_t>(key)) {
      oss << std::get<int32_t>(key) << ":";
    } else if (std::holds_alternative<int64_t>(key)) {
      oss << std::get<int64_t>(key) << ":";
    }

    // Value
    oss << EncodableValueToJson(val);
  }
  oss << "}";
  return oss.str();
}

static std::string EncodableListToJson(const flutter::EncodableList& list) {
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < list.size(); ++i) {
    if (i > 0) oss << ",";
    oss << EncodableValueToJson(list[i]);
  }
  oss << "]";
  return oss.str();
}

static std::string EncodableValueToJson(const flutter::EncodableValue& value) {
  // Use holds_alternative with try-catch to safely check types
  // This avoids static_assert errors and bad_variant_access exceptions
  try {
    if (std::holds_alternative<bool>(value)) {
      return std::get<bool>(value) ? "true" : "false";
    }
    if (std::holds_alternative<int32_t>(value)) {
      return std::to_string(std::get<int32_t>(value));
    }
    if (std::holds_alternative<int64_t>(value)) {
      return std::to_string(std::get<int64_t>(value));
    }
    if (std::holds_alternative<double>(value)) {
      return std::to_string(std::get<double>(value));
    }
    if (std::holds_alternative<std::string>(value)) {
      std::ostringstream oss;
      oss << "\"" << std::get<std::string>(value) << "\"";
      return oss.str();
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
      return EncodableListToJson(std::get<flutter::EncodableList>(value));
    }
    if (std::holds_alternative<flutter::EncodableMap>(value)) {
      return EncodableMapToJson(std::get<flutter::EncodableMap>(value));
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

// Convert event to JSON and call FFI callback if registered
static void NotifyFFIEventCallback(int64_t player_id,
                                   const flutter::EncodableValue& event_value) {
  DartPortEventCallback callback = VideoPlayer::GetFFIEventCallback();
  if (callback) {
    std::string event_json = EncodableValueToJson(event_value);
    // Pass strdup-allocated string to callback
    callback(player_id, strdup(event_json.c_str()));
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

  // LOG_INFO("[VideoPlayer] ExecuteSinkEvents: player_id=%lld, queue_size=%zu",
  //          static_cast<long long>(player_id_),
  //          encodable_event_queue_.size());

  std::lock_guard<std::mutex> lock(queue_mutex_);
  int event_count = 0;
  while (!encodable_event_queue_.empty()) {
    const flutter::EncodableValue& event = encodable_event_queue_.front();

    // Send to FFI using Dart_PostCObject_DL
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

    encodable_event_queue_.pop();
  }

  LOG_INFO("[VideoPlayer] ExecuteSinkEvents: sent %d events", event_count);

  while (!error_event_queue_.empty()) {
    const auto& error = error_event_queue_.front();
    // Send error event via FFI
    flutter::EncodableMap error_map = {
        {flutter::EncodableValue("event"), flutter::EncodableValue("error")},
        {flutter::EncodableValue("code"), flutter::EncodableValue(error.first)},
        {flutter::EncodableValue("message"),
         flutter::EncodableValue(error.second)},
    };
    PushEvent(flutter::EncodableValue(error_map));
    error_event_queue_.pop();
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
