// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_stt_plugin.h"

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <glib.h>
#include <stt.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <utility>

namespace {

using EncodableValue = flutter::EncodableValue;
using EncodableMap = flutter::EncodableMap;
using EncodableList = flutter::EncodableList;
using EventSink = flutter::EventSink<EncodableValue>;
using StreamHandlerError = flutter::StreamHandlerError<EncodableValue>;

constexpr char kMethodChannelName[] = "tizen_stt";
constexpr char kEventChannelName[] = "tizen_stt/events";

std::string ErrorName(int error) {
  switch (error) {
    case STT_ERROR_NONE:
      return "STT_ERROR_NONE";
    case STT_ERROR_OUT_OF_MEMORY:
      return "STT_ERROR_OUT_OF_MEMORY";
    case STT_ERROR_IO_ERROR:
      return "STT_ERROR_IO_ERROR";
    case STT_ERROR_INVALID_PARAMETER:
      return "STT_ERROR_INVALID_PARAMETER";
    case STT_ERROR_TIMED_OUT:
      return "STT_ERROR_TIMED_OUT";
    case STT_ERROR_RECORDER_BUSY:
      return "STT_ERROR_RECORDER_BUSY";
    case STT_ERROR_OUT_OF_NETWORK:
      return "STT_ERROR_OUT_OF_NETWORK";
    case STT_ERROR_PERMISSION_DENIED:
      return "STT_ERROR_PERMISSION_DENIED";
    case STT_ERROR_NOT_SUPPORTED:
      return "STT_ERROR_NOT_SUPPORTED";
    case STT_ERROR_INVALID_STATE:
      return "STT_ERROR_INVALID_STATE";
    case STT_ERROR_INVALID_LANGUAGE:
      return "STT_ERROR_INVALID_LANGUAGE";
    case STT_ERROR_ENGINE_NOT_FOUND:
      return "STT_ERROR_ENGINE_NOT_FOUND";
    case STT_ERROR_OPERATION_FAILED:
      return "STT_ERROR_OPERATION_FAILED";
    case STT_ERROR_NOT_SUPPORTED_FEATURE:
      return "STT_ERROR_NOT_SUPPORTED_FEATURE";
    case STT_ERROR_RECORDING_TIMED_OUT:
      return "STT_ERROR_RECORDING_TIMED_OUT";
    case STT_ERROR_NO_SPEECH:
      return "STT_ERROR_NO_SPEECH";
    case STT_ERROR_IN_PROGRESS_TO_READY:
      return "STT_ERROR_IN_PROGRESS_TO_READY";
    case STT_ERROR_IN_PROGRESS_TO_RECORDING:
      return "STT_ERROR_IN_PROGRESS_TO_RECORDING";
    case STT_ERROR_IN_PROGRESS_TO_PROCESSING:
      return "STT_ERROR_IN_PROGRESS_TO_PROCESSING";
    case STT_ERROR_SERVICE_RESET:
      return "STT_ERROR_SERVICE_RESET";
    default:
      return "STT_ERROR_" + std::to_string(error);
  }
}

std::string StateName(stt_state_e state) {
  switch (state) {
    case STT_STATE_CREATED:
      return "created";
    case STT_STATE_READY:
      return "ready";
    case STT_STATE_RECORDING:
      return "recording";
    case STT_STATE_PROCESSING:
      return "processing";
    default:
      return "unknown";
  }
}

std::string ResultEventName(stt_result_event_e event) {
  switch (event) {
    case STT_RESULT_EVENT_FINAL_RESULT:
      return "final";
    case STT_RESULT_EVENT_PARTIAL_RESULT:
      return "partial";
    case STT_RESULT_EVENT_ERROR:
      return "error";
    default:
      return "unknown";
  }
}

EncodableMap ErrorMap(int error, const std::string& operation) {
  return EncodableMap{
      {EncodableValue("operation"), EncodableValue(operation)},
      {EncodableValue("code"), EncodableValue(error)},
      {EncodableValue("name"), EncodableValue(ErrorName(error))},
  };
}

EncodableMap StateMap(stt_state_e state) {
  return EncodableMap{
      {EncodableValue("code"), EncodableValue(static_cast<int>(state))},
      {EncodableValue("name"), EncodableValue(StateName(state))},
  };
}

bool CollectLanguage(stt_h stt, const char* language, void* user_data) {
  auto* languages = static_cast<EncodableList*>(user_data);
  languages->push_back(EncodableValue(language ? language : ""));
  return true;
}

class TizenSttPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<TizenSttPlugin>();
    auto* plugin_pointer = plugin.get();

    auto method_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), kMethodChannelName,
            &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });
    plugin->method_channel_ = std::move(method_channel);

    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), kEventChannelName,
            &flutter::StandardMethodCodec::GetInstance());
    auto handler =
        std::make_unique<flutter::StreamHandlerFunctions<EncodableValue>>(
            [plugin_pointer](const EncodableValue* arguments,
                             std::unique_ptr<EventSink>&& events)
                -> std::unique_ptr<StreamHandlerError> {
              plugin_pointer->event_sink_ = std::move(events);
              return nullptr;
            },
            [plugin_pointer](const EncodableValue* arguments)
                -> std::unique_ptr<StreamHandlerError> {
              plugin_pointer->event_sink_.reset();
              return nullptr;
            });
    event_channel->SetStreamHandler(std::move(handler));
    plugin->event_channel_ = std::move(event_channel);

    registrar->AddPlugin(std::move(plugin));
  }

  ~TizenSttPlugin() override {
    DisposeStt();
    ClearEvents();
  }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto& method_name = method_call.method_name();
    if (method_name == "initialize") {
      if (initialize_result_) {
        ReplyError(std::move(result), STT_ERROR_IN_PROGRESS_TO_READY,
                   method_name);
        return;
      }
      int ret = Initialize();
      stt_state_e state = STT_STATE_CREATED;
      if (ret == STT_ERROR_NONE) ret = stt_get_state(stt_, &state);
      if (ret != STT_ERROR_NONE) {
        DisposeStt();
        ReplyError(std::move(result), ret, method_name);
      } else if (state == STT_STATE_CREATED) {
        initialize_result_ = std::move(result);
        initialize_timer_id_ = g_timeout_add(
            10000,
            [](gpointer data) -> gboolean {
              auto* self = static_cast<TizenSttPlugin*>(data);
              self->initialize_timer_id_ = 0;
              self->CompleteInitialize(STT_ERROR_TIMED_OUT);
              self->DisposeStt();
              return G_SOURCE_REMOVE;
            },
            this);
      } else {
        ReplyStatus(std::move(result));
      }
      return;
    }
    if (method_name == "getLanguages") {
      EncodableList languages;
      int ret = stt_ ? stt_foreach_supported_languages(stt_, CollectLanguage,
                                                       &languages)
                     : STT_ERROR_INVALID_STATE;
      if (ret != STT_ERROR_NONE) {
        ReplyError(std::move(result), ret, method_name);
      } else {
        result->Success(EncodableValue(languages));
      }
      return;
    }
    if (method_name == "start" || method_name == "stop" ||
        method_name == "cancel") {
      int ret = method_name == "start"  ? Start(method_call.arguments())
                : method_name == "stop" ? Stop()
                                        : Cancel();
      if (ret != STT_ERROR_NONE) {
        ReplyError(std::move(result), ret, method_name);
      } else {
        ReplyStatus(std::move(result));
      }
      return;
    }
    if (method_name == "dispose") {
      int ret = DisposeStt();
      if (ret != STT_ERROR_NONE) {
        ReplyError(std::move(result), ret, "stt_destroy");
        return;
      }
      result->Success();
      return;
    }
    if (method_name == "getState") {
      ReplyStatus(std::move(result));
      return;
    }
    result->NotImplemented();
  }

  int EnsureHandle() {
    if (stt_) {
      return STT_ERROR_NONE;
    }

    int ret = stt_create(&stt_);
    if (ret != STT_ERROR_NONE) {
      stt_ = nullptr;
      return ret;
    }

    ret = stt_set_state_changed_cb(stt_, OnStateChanged, this);
    if (ret == STT_ERROR_NONE)
      ret = stt_set_recognition_result_cb(stt_, OnRecognitionResult, this);
    if (ret == STT_ERROR_NONE) ret = stt_set_error_cb(stt_, OnError, this);
    if (ret != STT_ERROR_NONE) return ret;
    return STT_ERROR_NONE;
  }

  int Initialize() {
    int ret = EnsureHandle();
    if (ret != STT_ERROR_NONE) {
      return ret;
    }

    stt_state_e state = STT_STATE_CREATED;
    ret = stt_get_state(stt_, &state);
    if (ret != STT_ERROR_NONE) {
      return ret;
    }
    if (state == STT_STATE_CREATED) {
      ret = stt_prepare(stt_);
      if (ret != STT_ERROR_NONE) {
        return ret;
      }
    }
    return STT_ERROR_NONE;
  }

  void CompleteInitialize(int error) {
    if (initialize_timer_id_) {
      g_source_remove(initialize_timer_id_);
      initialize_timer_id_ = 0;
    }
    if (!initialize_result_) return;
    auto result = std::move(initialize_result_);
    if (error == STT_ERROR_NONE) {
      ReplyStatus(std::move(result));
    } else {
      ReplyError(std::move(result), error, "initialize");
    }
  }

  int Start(const EncodableValue* arguments) {
    if (!stt_) return STT_ERROR_INVALID_STATE;
    if (!arguments || !std::holds_alternative<EncodableMap>(*arguments)) {
      return STT_ERROR_INVALID_PARAMETER;
    }
    const auto& args = std::get<EncodableMap>(*arguments);
    auto language = args.find(EncodableValue("language"));
    auto type = args.find(EncodableValue("type"));
    if (language == args.end() || type == args.end() ||
        (!std::holds_alternative<std::monostate>(language->second) &&
         !std::holds_alternative<std::string>(language->second)) ||
        !std::holds_alternative<std::string>(type->second)) {
      return STT_ERROR_INVALID_PARAMETER;
    }
    const auto* language_value = std::get_if<std::string>(&language->second);
    const auto& type_value = std::get<std::string>(type->second);
    if ((language_value && (language_value->empty() ||
                            language_value->find('\0') != std::string::npos)) ||
        type_value.empty() || type_value.find('\0') != std::string::npos) {
      return STT_ERROR_INVALID_PARAMETER;
    }
    return stt_start(stt_, language_value ? language_value->c_str() : nullptr,
                     type_value.c_str());
  }

  int Stop() {
    if (!stt_) {
      return STT_ERROR_INVALID_STATE;
    }
    return stt_stop(stt_);
  }

  int Cancel() {
    if (!stt_) return STT_ERROR_NONE;
    if (initialize_result_) {
      return DisposeStt();
    }
    stt_state_e state;
    int ret = stt_get_state(stt_, &state);
    if (ret != STT_ERROR_NONE) return ret;
    if (state == STT_STATE_CREATED || state == STT_STATE_READY)
      return STT_ERROR_NONE;
    return stt_cancel(stt_);
  }

  int DisposeStt() {
    CompleteInitialize(STT_ERROR_INVALID_STATE);
    if (stt_) {
      int ret = stt_destroy(stt_);
      if (ret != STT_ERROR_NONE) return ret;
      stt_ = nullptr;
    }
    ClearEvents();
    return STT_ERROR_NONE;
  }

  void ReplyStatus(
      std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
    EncodableMap state_map{
        {EncodableValue("code"), EncodableValue(-1)},
        {EncodableValue("name"), EncodableValue("notCreated")},
    };
    if (stt_) {
      stt_state_e state;
      int ret = stt_get_state(stt_, &state);
      if (ret != STT_ERROR_NONE) {
        ReplyError(std::move(result), ret, "stt_get_state");
        return;
      }
      state_map = StateMap(state);
    }
    result->Success(EncodableValue(EncodableMap{
        {EncodableValue("state"), EncodableValue(state_map)},
    }));
  }

  void ReplyError(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result,
      int error, const std::string& operation) {
    result->Error(ErrorName(error), operation + " failed: " + ErrorName(error),
                  EncodableValue(ErrorMap(error, operation)));
  }

  void EmitError(int error, const std::string& operation) {
    EncodableMap map = ErrorMap(error, operation);
    map[EncodableValue("type")] = EncodableValue("error");
    EmitEvent(map);
  }

  void EmitEvent(EncodableMap map) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    event_queue_.push(EncodableValue(std::move(map)));
    if (!event_source_id_) {
      event_source_id_ = g_idle_add_full(
          G_PRIORITY_DEFAULT,
          [](gpointer data) -> gboolean {
            static_cast<TizenSttPlugin*>(data)->DrainEvents();
            return G_SOURCE_REMOVE;
          },
          this, nullptr);
    }
  }

  void ClearEvents() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (event_source_id_) {
      g_source_remove(event_source_id_);
      event_source_id_ = 0;
    }
    std::queue<EncodableValue>().swap(event_queue_);
  }

  void DrainEvents() {
    std::queue<EncodableValue> pending;
    {
      std::lock_guard<std::mutex> lock(queue_mutex_);
      event_source_id_ = 0;
      std::swap(pending, event_queue_);
    }
    while (!pending.empty()) {
      const auto& map = std::get<EncodableMap>(pending.front());
      const auto& type = std::get<std::string>(map.at(EncodableValue("type")));
      if (initialize_result_ && type == "state" &&
          std::get<EncodableMap>(map.at(EncodableValue("current")))
                  .at(EncodableValue("name")) == EncodableValue("ready")) {
        CompleteInitialize(STT_ERROR_NONE);
      } else if (initialize_result_ && type == "error") {
        CompleteInitialize(std::get<int32_t>(map.at(EncodableValue("code"))));
        DisposeStt();
      }
      if (event_sink_) {
        event_sink_->Success(pending.front());
      }
      pending.pop();
    }
  }

  static void OnStateChanged(stt_h stt, stt_state_e previous,
                             stt_state_e current, void* user_data) {
    auto* self = static_cast<TizenSttPlugin*>(user_data);
    self->EmitEvent(EncodableMap{
        {EncodableValue("type"), EncodableValue("state")},
        {EncodableValue("previous"), EncodableValue(StateMap(previous))},
        {EncodableValue("current"), EncodableValue(StateMap(current))},
    });
  }

  static void OnRecognitionResult(stt_h stt, stt_result_event_e event,
                                  const char** data, int data_count,
                                  const char* msg, void* user_data) {
    auto* self = static_cast<TizenSttPlugin*>(user_data);
    EncodableList texts;
    for (int i = 0; i < data_count; ++i) {
      texts.push_back(EncodableValue(data && data[i] ? data[i] : ""));
    }
    std::string text;
    if (!texts.empty() && std::holds_alternative<std::string>(texts.front())) {
      text = std::get<std::string>(texts.front());
    }
    self->EmitEvent(EncodableMap{
        {EncodableValue("type"), EncodableValue("result")},
        {EncodableValue("event"), EncodableValue(ResultEventName(event))},
        {EncodableValue("text"), EncodableValue(text)},
        {EncodableValue("texts"), EncodableValue(texts)},
        {EncodableValue("message"), EncodableValue(msg ? msg : "")},
    });
  }

  static void OnError(stt_h stt, stt_error_e reason, void* user_data) {
    auto* self = static_cast<TizenSttPlugin*>(user_data);
    self->EmitError(reason, "stt_error_cb");
  }

  stt_h stt_ = nullptr;
  guint initialize_timer_id_ = 0;
  std::unique_ptr<flutter::MethodResult<EncodableValue>> initialize_result_;
  guint event_source_id_ = 0;
  std::mutex queue_mutex_;
  std::queue<EncodableValue> event_queue_;
  std::unique_ptr<EventSink> event_sink_;
  std::unique_ptr<flutter::MethodChannel<EncodableValue>> method_channel_;
  std::unique_ptr<flutter::EventChannel<EncodableValue>> event_channel_;
};

}  // namespace

void TizenSttPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenSttPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
