// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_tts_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "log.h"
#include "text_to_speech.h"

namespace {

typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;

class FlutterTtsTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<FlutterTtsTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  FlutterTtsTizenPlugin(flutter::PluginRegistrar *registrar) {
    channel_ = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "flutter_tts",
        &flutter::StandardMethodCodec::GetInstance());
    channel_->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });

    tts_ = std::make_unique<TextToSpeech>();
    if (!tts_->Initialize()) {
      tts_ = nullptr;
      return;
    }
    tts_->SetStateChanagedCallback([this](TtsState previous, TtsState current) {
      std::unique_ptr<flutter::EncodableValue> value =
          std::make_unique<flutter::EncodableValue>(true);
      if (current == TtsState::kPlaying) {
        if (previous == TtsState::kReady) {
          channel_->InvokeMethod("speak.onStart", std::move(value));
        } else if (previous == TtsState::kPaused) {
          channel_->InvokeMethod("speak.onContinue", std::move(value));
        }
      } else if (current == TtsState::kPaused) {
        channel_->InvokeMethod("speak.onPause", std::move(value));
      } else if (current == TtsState::kReady) {
        if (previous == TtsState::kPlaying || previous == TtsState::kPaused) {
          // The utterance ID is not zero while speaking and pausing.
          if (tts_->GetUttId()) {
            channel_->InvokeMethod("speak.onCancel", std::move(value));
            HandleAwaitSpeakCompletion(flutter::EncodableValue(0));
          }
        }
      }
    });
    tts_->SetUtteranceCompletedCallback([this](int32_t utt_id) {
      std::unique_ptr<flutter::EncodableValue> args =
          std::make_unique<flutter::EncodableValue>(true);
      channel_->InvokeMethod("speak.onComplete", std::move(args));
      HandleAwaitSpeakCompletion(flutter::EncodableValue(1));
    });
  }

  virtual ~FlutterTtsTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<FlMethodResult> result) {
    // Keep the return values consistent with the Android implementation.
    // The Dart side of this plugin expects a return value of 1 on success and
    // 0 on failure, but not result->Error() as other plugins normally expect.
    const auto method_name = method_call.method_name();
    const auto &arguments = *method_call.arguments();

    result_ = std::move(result);

    if (!tts_) {
      result_->Error("Operation failed", "TTS initialization failed.");
      return;
    }

    if (method_name == "awaitSpeakCompletion") {
      OnAwaitSpeakCompletion(arguments);
    } else if (method_name == "speak") {
      OnSpeak(arguments);
    } else if (method_name == "stop") {
      OnStop();
    } else if (method_name == "pause") {
      OnPause();
    } else if (method_name == "setSpeechRate") {
      OnSetSpeechRate(arguments);
    } else if (method_name == "setLanguage") {
      OnSetLanguage(arguments);
    } else if (method_name == "getLanguages") {
      OnGetLanguage();
    } else if (method_name == "getDefaultVoice") {
      OnGetDefaultVoice();
    } else if (method_name == "getMaxSpeechInputLength") {
      OnGetMaxSpeechInputLength();
    } else if (method_name == "setVolume") {
      OnSetVolume(arguments);
    } else {
      result_->NotImplemented();
    }
  }

  void OnAwaitSpeakCompletion(const flutter::EncodableValue &arguments) {
    if (std::holds_alternative<bool>(arguments)) {
      await_speak_completion_ = std::get<bool>(arguments);
      SendResult(flutter::EncodableValue(1));
      return;
    }
    SendResult(flutter::EncodableValue(0));
  }

  void OnSpeak(const flutter::EncodableValue &arguments) {
    std::optional<TtsState> state = tts_->GetState();
    if (!state.has_value() || state == TtsState::kPlaying) {
      if (state.has_value() && state == TtsState::kPlaying) {
        LOG_ERROR("You cannot speak again while speaking.");
      }
      SendResult(flutter::EncodableValue(0));
      return;
    }

    if (std::holds_alternative<std::string>(arguments)) {
      std::string text = std::get<std::string>(arguments);
      if (!tts_->AddText(text)) {
        SendResult(flutter::EncodableValue(0));
        return;
      }
    }

    if (tts_->Speak()) {
      if (await_speak_completion_ && !result_for_await_speak_completion_) {
        result_for_await_speak_completion_ = std::move(result_);
      } else {
        SendResult(flutter::EncodableValue(1));
      }
    } else {
      SendResult(flutter::EncodableValue(0));
    }
  }

  void OnStop() {
    if (tts_->Stop()) {
      SendResult(flutter::EncodableValue(1));
    } else {
      SendResult(flutter::EncodableValue(0));
    }
  }

  void OnPause() {
    if (tts_->Pause()) {
      SendResult(flutter::EncodableValue(1));
    } else {
      SendResult(flutter::EncodableValue(0));
    }
  }

  void OnSetSpeechRate(const flutter::EncodableValue &arguments) {
    if (std::holds_alternative<double>(arguments)) {
      double rate = std::get<double>(arguments);
      rate = std::max(rate, 0.0);
      rate = std::min(rate, 1.0);
      // Scale the value to be between the supported range.
      int32_t min = 0, normal = 0, max = 0;
      tts_->GetSpeedRange(&min, &normal, &max);
      int32_t speed = min + (max - min) * rate;
      tts_->SetTtsSpeed(speed);
      SendResult(flutter::EncodableValue(1));
      return;
    }
    SendResult(flutter::EncodableValue(0));
  }

  void OnSetLanguage(const flutter::EncodableValue &arguments) {
    if (std::holds_alternative<std::string>(arguments)) {
      std::string language = std::move(std::get<std::string>(arguments));
      tts_->SetDefaultLanguage(language);
      SendResult(flutter::EncodableValue(1));
      return;
    }
    SendResult(flutter::EncodableValue(0));
  }

  void OnGetLanguage() {
    flutter::EncodableList list;
    for (auto language : tts_->GetSupportedLanaguages()) {
      list.push_back(flutter::EncodableValue(language));
    }
    SendResult(flutter::EncodableValue(list));
  }

  void OnGetDefaultVoice() {
    std::optional<std::string> voice = tts_->GetDefaultVoice();
    if (!voice.has_value()) {
      result_->Error("Operation failed",
                     "Failed to get TTS engine's default voice.");
      result_ = nullptr;
      return;
    }
    SendResult(flutter::EncodableValue(*voice));
  }

  void OnGetMaxSpeechInputLength() {
    std::optional<int32_t> length = tts_->GetMaxSpeechInputLength();
    if (!length.has_value()) {
      result_->Error("Operation failed",
                     "Failed to get TTS engine's max speech input length.");
      result_ = nullptr;
      return;
    }
    SendResult(flutter::EncodableValue(*length));
  }

  void OnSetVolume(const flutter::EncodableValue &arguments) {
    if (std::holds_alternative<double>(arguments)) {
      double volume = std::get<double>(arguments);
      if (tts_->SetVolume(volume)) {
        SendResult(flutter::EncodableValue(1));
        return;
      }
    }
    SendResult(flutter::EncodableValue(0));
  }

  void SendResult(const flutter::EncodableValue &result) {
    if (result_) {
      result_->Success(result);
      result_ = nullptr;
    }
  }

  void HandleAwaitSpeakCompletion(const flutter::EncodableValue &result) {
    if (await_speak_completion_) {
      result_for_await_speak_completion_->Success(result);
      result_for_await_speak_completion_ = nullptr;
    }
  }

  std::unique_ptr<TextToSpeech> tts_;
  bool await_speak_completion_ = false;

  std::unique_ptr<FlMethodResult> result_for_await_speak_completion_;
  std::unique_ptr<FlMethodResult> result_;
  std::unique_ptr<FlMethodChannel> channel_;
};

}  // namespace

void FlutterTtsTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterTtsTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
