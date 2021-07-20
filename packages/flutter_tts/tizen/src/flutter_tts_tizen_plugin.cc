#include "flutter_tts_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "log.h"
#include "text_to_speech.h"

class FlutterTtsTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<FlutterTtsTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  FlutterTtsTizenPlugin(flutter::PluginRegistrar *registrar) {
    channel_ =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "flutter_tts",
            &flutter::StandardMethodCodec::GetInstance());
    channel_->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });

    tts_ = std::make_unique<TextToSpeech>();
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
    // Keep in sync with the return values implemented in:
    // https://github.com/dlutton/flutter_tts/blob/master/android/src/main/java/com/tundralabs/fluttertts/FlutterTtsPlugin.java
    // The API specification is vague, and there is no detailed description of
    // the return value, so I mimic the Android implementation.

    const auto method_name = method_call.method_name();
    const auto &arguments = *method_call.arguments();

    if (method_name.compare("awaitSpeakCompletion") == 0) {
      if (std::holds_alternative<bool>(arguments)) {
        await_speak_completion_ = std::get<bool>(arguments);
        result->Success(flutter::EncodableValue(1));
        return;
      }
      result->Success(flutter::EncodableValue(0));
    } else if (method_name.compare("speak") == 0) {
      if (tts_->GetState() == TTS_STATE_PLAYING) {
        LOG_ERROR("[TTS] : You cannot speak again while speaking.");
        result->Success(flutter::EncodableValue(0));
        return;
      }

      if (std::holds_alternative<std::string>(arguments)) {
        std::string text = std::get<std::string>(arguments);
        if (!tts_->AddText(text)) {
          LOG_ERROR("Invalid Arguments!");
          result->Success(flutter::EncodableValue(0));
          return;
        }
      }

      if (tts_->Speak()) {
        if (await_speak_completion_ && !result_for_await_speak_completion_) {
          LOG_DEBUG("Store result ptr for await speak completion");
          result_for_await_speak_completion_ = std::move(result);
        } else {
          result->Success(flutter::EncodableValue(1));
        }
      } else {
        result->Success(flutter::EncodableValue(0));
      }
    } else if (method_name.compare("stop") == 0) {
      if (tts_->Stop()) {
        result->Success(flutter::EncodableValue(1));
      } else {
        result->Success(flutter::EncodableValue(0));
      }
    } else if (method_name.compare("pause") == 0) {
      if (tts_->Pause()) {
        result->Success(flutter::EncodableValue(1));
      } else {
        result->Success(flutter::EncodableValue(0));
      }
    } else if (method_name.compare("getSpeechRateValidRange") == 0) {
      int min = 0, normal = 0, max = 0;
      tts_->GetSpeedRange(&min, &normal, &max);
      flutter::EncodableMap map;
      map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
          "min", min));
      map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
          "normal", normal));
      map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
          "max", max));
      map.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
          "platform", "tizen"));
      result->Success(flutter::EncodableValue(std::move(map)));
      return;
    } else if (method_name.compare("setSpeechRate") == 0) {
      if (std::holds_alternative<double>(arguments)) {
        int speed = (int)std::get<double>(arguments);
        tts_->SetTtsSpeed(speed);
        result->Success(flutter::EncodableValue(1));
        return;
      }
      result->Success(flutter::EncodableValue(0));
    } else if (method_name.compare("setLanguages") == 0) {
      if (std::holds_alternative<std::string>(arguments)) {
        std::string language = std::move(std::get<std::string>(arguments));
        tts_->SetDefaultLanguage(language);
        result->Success(flutter::EncodableValue(1));
        return;
      }
      result->Success(flutter::EncodableValue(0));
    } else if (method_name.compare("getLanguages") == 0) {
      flutter::EncodableList list;
      for (auto language : tts_->GetSupportedLanaguages()) {
        list.push_back(flutter::EncodableValue(language));
      }
      result->Success(flutter::EncodableValue(list));
    } else {
      result->Error("-1", "Not supported method");
      return;
    }
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
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;

  bool await_speak_completion_ = false;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
      result_for_await_speak_completion_;
};

void FlutterTtsTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterTtsTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
