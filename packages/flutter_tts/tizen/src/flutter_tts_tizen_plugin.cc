#include "flutter_tts_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <tts.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "log.h"

class FlutterTtsTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<FlutterTtsTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  FlutterTtsTizenPlugin(flutter::PluginRegistrar *registrar)
      : tts_(nullptr),
        voice_type_(TTS_VOICE_TYPE_AUTO),
        speed_(TTS_SPEED_AUTO),
        speaking_(false),
        await_speak_completion_(false) {
    channel_ =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "flutter_tts",
            &flutter::StandardMethodCodec::GetInstance());
    channel_->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });
    EnsureTtsHandle();
  }

  virtual ~FlutterTtsTizenPlugin() {
    if (tts_ != nullptr) {
      tts_destroy(tts_);
      tts_ = nullptr;
    }
  }

  void EndSpeaking() {
    speaking_ = false;
    if (await_speak_completion_) {
      result_->Success(flutter::EncodableValue(1));
    }
  }

 private:
  bool EnsureTtsHandle() {
    if (tts_ != nullptr) {
      return true;
    }

    int ret = tts_create(&tts_);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_create failed: %s", get_error_message(ret));
      tts_ = nullptr;
      return false;
    }

    ret = tts_set_state_changed_cb(tts_, OnStateChanged, (void *)this);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_set_state_changed_cb failed: %s",
                get_error_message(ret));
      tts_destroy(tts_);
      tts_ = nullptr;
      return false;
    }

    ret = tts_set_utterance_completed_cb(tts_, OnUtteranceCompleted,
                                         (void *)this);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_set_utterance_completed_cb failed: %s",
                get_error_message(ret));
      tts_destroy(tts_);
      tts_ = nullptr;
      return false;
    }

    if (language_.size() == 0) {
      char *language;
      ret = tts_get_default_voice(tts_, &language, &voice_type_);

      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_get_default_voice failed: %s",
                  get_error_message(ret));
        tts_destroy(tts_);
        tts_ = nullptr;
        return false;
      }
      language_ = language;
      free(language);
    }

    ret = tts_foreach_supported_voices(tts_, OnSupportedVoices, (void *)this);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_foreach_supported_voices failed: %s",
                get_error_message(ret));
      tts_destroy(tts_);
      tts_ = nullptr;
      return false;
    }
    language_s.erase(unique(language_s.begin(), language_s.end()),
                     language_s.end());

    ret = tts_prepare(tts_);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_prepare failed: %s", get_error_message(ret));
      tts_destroy(tts_);
      tts_ = nullptr;
      return false;
    }

    return true;
  }

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if (!EnsureTtsHandle()) {
      LOG_ERROR("[TTS] Failed to ensure native tts APIs handle.");
      result->Error("Platform Error",
                    "Failed to ensure native tts APIs handle.");
      return;
    }

    // Keep in sync with the return values implemented in:
    // https://github.com/dlutton/flutter_tts/blob/master/android/src/main/java/com/tundralabs/fluttertts/FlutterTtsPlugin.java
    // The API specification is vague, and there is no detailed description of
    // the return value, so I mimic the Android implementation.

    const auto method_name = method_call.method_name();
    const auto &arguments = *method_call.arguments();
    LOG_INFO("method : %s", method_name.c_str());
    int ret = TTS_ERROR_NONE;

    if (method_name.compare("awaitSpeakCompletion") == 0) {
      if (std::holds_alternative<bool>(arguments)) {
        await_speak_completion_ = std::get<bool>(arguments);
        result->Success(flutter::EncodableValue(1));
        return;
      }
      result->Success(flutter::EncodableValue(0));
    } else if (method_name.compare("speak") == 0) {
      if (speaking_) {
        LOG_ERROR("[TTS] : You cannot speak again while speaking.");
        result->Success(flutter::EncodableValue(0));
        return;
      }

      if (std::holds_alternative<std::string>(arguments)) {
        std::string text = std::get<std::string>(arguments);
        int utt_id;
        ret = tts_add_text(tts_, text.c_str(), language_.c_str(), voice_type_,
                           speed_, &utt_id);
        if (ret != TTS_ERROR_NONE) {
          LOG_ERROR("[TTS] tts_add_text failed: %s", get_error_message(ret));
          result->Success(flutter::EncodableValue(0));
          return;
        }
      } else {
        LOG_ERROR("Invalid Arguments!");
        result->Success(flutter::EncodableValue(0));
        return;
      }

      ret = tts_play(tts_);

      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_play failed: %s", get_error_message(ret));
        result->Success(flutter::EncodableValue(0));
        return;
      }

      if (await_speak_completion_) {
        result_ = std::move(result);
      } else {
        result->Success(flutter::EncodableValue(1));
      }
    } else if (method_name.compare("stop") == 0) {
      ret = tts_stop(tts_);
      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_stop failed: %s", get_error_message(ret));
        result->Success(flutter::EncodableValue(0));
        return;
      }
      result->Success(flutter::EncodableValue(1));
    } else if (method_name.compare("pause") == 0) {
      ret = tts_pause(tts_);
      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_pause failed: %s", get_error_message(ret));
        result->Success(flutter::EncodableValue(0));
        return;
      }
      result->Success(flutter::EncodableValue(1));
    } else if (method_name.compare("getSpeechRateValidRange") == 0) {
      int min = 0, normal = 0, max = 0;
      ret = tts_get_speed_range(tts_, &min, &normal, &max);
      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_get_speed_range failed: %s",
                  get_error_message(ret));
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
      result->Success(flutter::EncodableValue(std::move(map)));
      return;
    } else if (method_name.compare("setSpeechRate") == 0) {
      if (std::holds_alternative<double>(arguments)) {
        speed_ = (int)std::get<double>(arguments);
        result->Success(flutter::EncodableValue(1));
        return;
      }
      result->Success(flutter::EncodableValue(0));
    } else if (method_name.compare("setLanguages") == 0) {
      if (std::holds_alternative<std::string>(arguments)) {
        language_ = std::move(std::get<std::string>(arguments));
        result->Success(flutter::EncodableValue(1));
        return;
      }
      result->Success(flutter::EncodableValue(0));
    } else if (method_name.compare("getLanguages") == 0) {
      result->Success(flutter::EncodableValue(language_s));
    } else {
      result->Error("-1", "Not supported method");
      return;
    }
  }

  static void OnStateChanged(tts_h tts, tts_state_e previous,
                             tts_state_e current, void *user_data) {
    FlutterTtsTizenPlugin *plugin = (FlutterTtsTizenPlugin *)user_data;
    LOG_INFO("[TTS] State is changed (%d) to (%d)", previous, current);
    std::unique_ptr<flutter::EncodableValue> args =
        std::make_unique<flutter::EncodableValue>(true);
    if (current == TTS_STATE_PLAYING) {
      plugin->speaking_ = true;
      if (previous == TTS_STATE_READY) {
        plugin->channel_->InvokeMethod("speak.onStart", std::move(args));
      } else if (previous == TTS_STATE_PAUSED) {
        plugin->channel_->InvokeMethod("speak.onContinue", std::move(args));
      }
    } else if (current == TTS_STATE_PAUSED) {
      plugin->EndSpeaking();
      plugin->channel_->InvokeMethod("speak.onPause", std::move(args));
    } else if (current == TTS_STATE_READY) {
      if (previous == TTS_STATE_PLAYING || previous == TTS_STATE_PAUSED) {
        plugin->EndSpeaking();
        plugin->channel_->InvokeMethod("speak.onCancel", std::move(args));
      }
    }
    plugin->current_state_ = current;
  }

  static void OnUtteranceCompleted(tts_h tts, int utt_id, void *user_data) {
    FlutterTtsTizenPlugin *plugin = (FlutterTtsTizenPlugin *)user_data;
    tts_stop(plugin->tts_);
    LOG_INFO("[TTS] Utterance (%d) is completed", true);

    std::unique_ptr<flutter::EncodableValue> args =
        std::make_unique<flutter::EncodableValue>(true);
    plugin->channel_->InvokeMethod("speak.onComplete", std::move(args));
  }

  static bool OnSupportedVoices(tts_h tts, const char *language, int voice_type,
                                void *user_data) {
    if (nullptr != language) {
      FlutterTtsTizenPlugin *plugin = (FlutterTtsTizenPlugin *)user_data;

      plugin->language_s.push_back(flutter::EncodableValue(language));
      LOG_INFO("[TTS] Supported Voices - Language(%s), Type(%d)", language,
               voice_type);
      return true;
    }
    return false;
  }

  tts_h tts_;
  tts_state_e current_state_;
  bool speaking_;
  bool await_speak_completion_;
  std::string language_;
  int voice_type_;
  int speed_;
  flutter::EncodableList language_s;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result_;
};

void FlutterTtsTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterTtsTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
