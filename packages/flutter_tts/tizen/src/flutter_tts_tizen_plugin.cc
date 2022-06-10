// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_tts_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
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
    try {
      tts_->Initialize();
    } catch (const TextToSpeechError &error) {
      // TODO : Handle initialization failure cases
      // Rarely, initializing TextToSpeech can fail. we should consider catching
      // the exception and propagating it to the flutter side. however, I think
      // this is optional because flutter side is not expecting any errors.
      LOG_ERROR("Operation failed : %s", error.GetErrorString().c_str());
      tts_ = nullptr;
      return;
    }
    tts_->SetOnStateChanagedCallback(
        [this](tts_state_e previous, tts_state_e current) -> void {
          std::unique_ptr<flutter::EncodableValue> value =
              std::make_unique<flutter::EncodableValue>(true);
          if (current == TTS_STATE_PLAYING) {
            if (previous == TTS_STATE_READY) {
              channel_->InvokeMethod("speak.onStart", std::move(value));
            } else if (previous == TTS_STATE_PAUSED) {
              channel_->InvokeMethod("speak.onContinue", std::move(value));
            }
          } else if (current == TTS_STATE_PAUSED) {
            channel_->InvokeMethod("speak.onPause", std::move(value));
          } else if (current == TTS_STATE_READY) {
            if (previous == TTS_STATE_PLAYING || previous == TTS_STATE_PAUSED) {
              // utterance ID is not zero during speaking and pausing.
              if (tts_->GetUttId()) {
                channel_->InvokeMethod("speak.onCancel", std::move(value));
                HandleAwaitSpeakCompletion(0);
              }
            }
          }
        });

    tts_->SetOnUtteranceCompletedCallback([this](int utt_id) -> void {
      LOG_INFO("[TTS] Utterance (%d) is completed", utt_id);
      std::unique_ptr<flutter::EncodableValue> args =
          std::make_unique<flutter::EncodableValue>(true);
      channel_->InvokeMethod("speak.onComplete", std::move(args));
      HandleAwaitSpeakCompletion(1);
    });

    tts_->SetErrorCallback([](int utt_id, tts_error_e reason) -> void {
      // It seems unnecessary for now.
    });
  }

  virtual ~FlutterTtsTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto method_name = method_call.method_name();
    const auto &arguments = *method_call.arguments();

    result_ = std::move(result);

    if (!tts_) {
      result_->Error("Operation failed", "TTS is invalid.");
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
    } else if (method_name == "getSpeechRateValidRange") {
      OnGetSpeechRateValidRange();
    } else if (method_name == "setSpeechRate") {
      OnSetSpeechRate(arguments);
    } else if (method_name == "setLanguage") {
      OnSetLanguage(arguments);
    } else if (method_name == "getLanguages") {
      OnGetLanguage();
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
    SendErrorResult("Invalid argument", "Argument is invaild.");
  }

  void OnSpeak(const flutter::EncodableValue &arguments) {
    try {
      if (tts_->GetState() == TtsState::kPlaying) {
        SendErrorResult("Operation cancelled",
                        "You cannot speak again while speaking.");
        return;
      }

      if (std::holds_alternative<std::string>(arguments)) {
        std::string text = std::get<std::string>(arguments);
        tts_->AddText(text);
      }

      tts_->Speak();

    } catch (const TextToSpeechError &error) {
      SendErrorResult("Operation failed", error.GetErrorString());
    }

    if (await_speak_completion_ && !result_for_await_speak_completion_) {
      LOG_DEBUG("Store result ptr for await speak completion");
      result_for_await_speak_completion_ = std::move(result_);
    } else {
      SendResult(flutter::EncodableValue(1));
    }
  }

  void OnStop() {
    try {
      tts_->Stop();
    } catch (const TextToSpeechError &error) {
      SendErrorResult("Operation failed", error.GetErrorString());
    }
    SendResult(flutter::EncodableValue(1));
  }

  void OnPause() {
    try {
      tts_->Pause();
    } catch (const TextToSpeechError &error) {
      SendErrorResult("Operation failed", error.GetErrorString());
    }
    SendResult(flutter::EncodableValue(1));
  }

  void OnGetSpeechRateValidRange() {
    int min = 0, normal = 0, max = 0;
    try {
      tts_->GetSpeedRange(&min, &normal, &max);
    } catch (const TextToSpeechError &error) {
      SendErrorResult("Operation failed", error.GetErrorString());
      return;
    }
    flutter::EncodableMap map;
    map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "min", min));
    map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "normal", normal));
    map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "max", max));
    map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "platform", "tizen"));
    SendResult(flutter::EncodableValue(std::move(map)));
  }

  void OnSetSpeechRate(const flutter::EncodableValue &arguments) {
    if (std::holds_alternative<double>(arguments)) {
      int speed = (int)std::get<double>(arguments);
      tts_->SetTtsSpeed(speed);
      SendResult(flutter::EncodableValue(1));
      return;
    }
    SendErrorResult("Invalid argument", "SpeechRate is invaild.");
  }

  void OnSetLanguage(const flutter::EncodableValue &arguments) {
    if (std::holds_alternative<std::string>(arguments)) {
      std::string language = std::move(std::get<std::string>(arguments));
      tts_->SetDefaultLanguage(language);
      SendResult(flutter::EncodableValue(1));
      return;
    }
    SendErrorResult("Invalid argument", "Language is invaild.");
  }

  void OnGetLanguage() {
    flutter::EncodableList list;
    for (auto language : tts_->GetSupportedLanaguages()) {
      list.push_back(flutter::EncodableValue(language));
    }
    SendResult(flutter::EncodableValue(list));
  }

  void OnSetVolume(const flutter::EncodableValue &arguments) {
    if (std::holds_alternative<double>(arguments)) {
      double volume = std::get<double>(arguments);
      try {
        tts_->SetVolume(volume);
      } catch (const TextToSpeechError &error) {
        SendErrorResult("Operation failed", error.GetErrorString());
        return;
      }
    }
    SendErrorResult("Invalid argument", "Volume is invaild.");
  }

  void SendResult(const flutter::EncodableValue &result) {
    if (!result_) {
      return;
    }
    result_->Success(result);
    result_ = nullptr;
  }

  void SendErrorResult(const std::string &error_code,
                       const std::string &error_message) {
    // Keep in sync with the return values implemented in:
    // https://github.com/dlutton/flutter_tts/blob/master/android/src/main/java/com/tundralabs/fluttertts/FlutterTtsPlugin.java.
    // In principle, MethodResult was designed to call MethodResult.Error() to
    // notify the dart code of any method call failures from the host platform.
    // However, in the case of flutter_tts, it expects a return value 0 on
    // failure(and value 1 on success). Therefore in the scope of this plugin,
    // we call result_->Success(flutter::EncodableValue(0)) to notify errors.
    if (!result_) {
      return;
    }
    LOG_ERROR("%s", std::string(error_code + " : " + error_message).c_str());
    result_->Success(flutter::EncodableValue(0));
    result_ = nullptr;
  }

  void HandleAwaitSpeakCompletion(int value) {
    if (await_speak_completion_) {
      LOG_DEBUG("Send result for await speak completion[%d]", value);
      result_for_await_speak_completion_->Success(
          flutter::EncodableValue(value));
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
