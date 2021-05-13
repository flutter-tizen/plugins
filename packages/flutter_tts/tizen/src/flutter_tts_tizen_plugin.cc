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

static std::string ErrorToString(int error) {
  switch (error) {
    case TTS_ERROR_NONE:
      return "TTS - Successful";
    case TTS_ERROR_OUT_OF_MEMORY:
      return "TTS - Out of Memory";
    case TTS_ERROR_IO_ERROR:
      return "TTS - I/O error";
    case TTS_ERROR_INVALID_PARAMETER:
      return "TTS - Invalid parameter";
    case TTS_ERROR_OUT_OF_NETWORK:
      return "TTS - Network is down";
    case TTS_ERROR_TIMED_OUT:
      return "TTS - No answer from the daemon";
    case TTS_ERROR_PERMISSION_DENIED:
      return "TTS - Permission denied";
    case TTS_ERROR_NOT_SUPPORTED:
      return "TTS - TTS NOT supported";
    case TTS_ERROR_INVALID_STATE:
      return "TTS - Invalid state";
    case TTS_ERROR_INVALID_VOICE:
      return "TTS - Invalid voice";
    case TTS_ERROR_OPERATION_FAILED:
      return "TTS - Operation failed";
    case TTS_ERROR_AUDIO_POLICY_BLOCKED:
      return "TTS - Audio policy blocked";
    case TTS_ERROR_NOT_SUPPORTED_FEATURE:
      return "TTS - Not supported feature of current engine";
    case TTS_ERROR_SERVICE_RESET:
      return "TTS - Service reset";
    default:
      return "TTS - Unknown Error";
  }
}

class FlutterTtsTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<FlutterTtsTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  FlutterTtsTizenPlugin(flutter::PluginRegistrar *registrar)
      : m_tts(nullptr),
        m_voice_type(TTS_VOICE_TYPE_AUTO),
        m_speed(TTS_SPEED_AUTO),
        m_speaking(false),
        m_await_speak_completion(false) {
    m_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "flutter_tts",
            &flutter::StandardMethodCodec::GetInstance());
    m_channel->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });
    EnsureTtsHandle();
  }

  virtual ~FlutterTtsTizenPlugin() {
    if (m_tts != nullptr) {
      tts_destroy(m_tts);
      m_tts = nullptr;
    }
  }

 private:
  bool EnsureTtsHandle() {
    if (m_tts != nullptr) {
      return true;
    }

    int ret = tts_create(&m_tts);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_create failed: %s", ErrorToString(ret).c_str());
      m_tts = nullptr;
      return false;
    }

    ret = tts_set_state_changed_cb(m_tts, OnStateChanged, (void *)this);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_set_state_changed_cb failed: %s",
                ErrorToString(ret).c_str());
      tts_destroy(m_tts);
      m_tts = nullptr;
      return false;
    }

    ret = tts_set_utterance_completed_cb(m_tts, OnUtteranceCompleted,
                                         (void *)this);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_set_utterance_completed_cb failed: %s",
                ErrorToString(ret).c_str());
      tts_destroy(m_tts);
      m_tts = nullptr;
      return false;
    }

    if (m_language.size() == 0) {
      char *language;
      ret = tts_get_default_voice(m_tts, &language, &m_voice_type);

      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_get_default_voice failed: %s",
                  ErrorToString(ret).c_str());
        tts_destroy(m_tts);
        m_tts = nullptr;
        return false;
      }
      m_language = language;
      free(language);
    }

    ret = tts_foreach_supported_voices(m_tts, OnSupportedVoices, (void *)this);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_foreach_supported_voices failed: %s",
                ErrorToString(ret).c_str());
      tts_destroy(m_tts);
      m_tts = nullptr;
      return false;
    }
    m_languages.erase(unique(m_languages.begin(), m_languages.end()),
                      m_languages.end());

    ret = tts_prepare(m_tts);
    if (ret != TTS_ERROR_NONE) {
      LOG_ERROR("[TTS] tts_prepare failed: %s", ErrorToString(ret).c_str());
      tts_destroy(m_tts);
      m_tts = nullptr;
      return false;
    }

    return true;
  }

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if (!EnsureTtsHandle()) {
      LOG_ERROR("[TTS] EnsureTtsHandle() failed: %s", "EnsureTtsHandle failed");
      result->Error("Invalid Operation", "Invalid Operation");
      return;
    }

    const auto method_name = method_call.method_name();
    const auto &arguments = *method_call.arguments();
    LOG_INFO("method : %s", method_name.c_str());
    int ret = TTS_ERROR_NONE;

    if (method_name.compare("awaitSpeakCompletion") == 0) {
      if (std::holds_alternative<bool>(arguments)) {
        m_await_speak_completion = std::get<bool>(arguments);
        result->Success();
        return;
      } else {
        result->Error("Invalid Arguments", "Invalid Arguments");
        return;
      }
    } else if (method_name.compare("speak") == 0) {
      if (m_speaking) {
        LOG_ERROR("[TTS] : You cannot speak again while speaking.");
        result->Error("Invalid Operation", "Invalid Operation");
        return;
      }
      if (std::holds_alternative<std::string>(arguments)) {
        std::string text = std::get<std::string>(arguments);
        int utt_id;
        ret = tts_add_text(m_tts, text.c_str(), m_language.c_str(),
                           m_voice_type, m_speed, &utt_id);
        if (ret != TTS_ERROR_NONE) {
          LOG_ERROR("[TTS] tts_add_text failed: %s",
                    ErrorToString(ret).c_str());
          result->Error(std::to_string(ret), "Failed to speak(tts_add_text).");
          return;
        }
      } else {
        result->Error("Invalid Arguments", "Invalid Arguments");
        return;
      }
      ret = tts_play(m_tts);
      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_play failed: %s", ErrorToString(ret).c_str());
        result->Error(std::to_string(ret), "Failed to speak(tts_play).");
        return;
      }
      if (m_await_speak_completion) {
        m_speaking = true;
        m_result = std::move(result);
      } else {
        result->Success();
        return;
      }
    } else if (method_name.compare("stop") == 0) {
      ret = tts_stop(m_tts);
      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_stop failed: %s", ErrorToString(ret).c_str());
        result->Error(std::to_string(ret), "Failed to stop.");
        return;
      }
      result->Success();
      return;
    } else if (method_name.compare("pause") == 0) {
      ret = tts_pause(m_tts);
      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_pause failed: %s", ErrorToString(ret).c_str());
        result->Error(std::to_string(ret), "Failed to pause.");
        return;
      }
      result->Success();
      return;
    } else if (method_name.compare("getSpeechRateValidRange") == 0) {
      int min, normal, max;
      ret = tts_get_speed_range(m_tts, &min, &normal, &max);
      if (ret != TTS_ERROR_NONE) {
        LOG_ERROR("[TTS] tts_get_speed_range failed: %s",
                  ErrorToString(ret).c_str());
        result->Error(std::to_string(ret),
                      "Failed to getSpeechRateValidRange.");
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
      result->Success(flutter::EncodableValue(std::move(map)));
      return;
    } else if (method_name.compare("setSpeechRate") == 0) {
      if (std::holds_alternative<double>(arguments)) {
        m_speed = (int)std::get<double>(arguments);
        result->Success();
        return;
      } else {
        result->Error("Invalid Arguments", "Invalid Arguments");
        return;
      }
    } else if (method_name.compare("setLanguages") == 0) {
      if (std::holds_alternative<std::string>(arguments)) {
        m_language = std::move(std::get<std::string>(arguments));
        result->Success();
        return;
      } else {
        result->Error("Invalid Arguments", "Invalid Arguments");
        return;
      }
    } else if (method_name.compare("getLanguages") == 0) {
      result->Success(flutter::EncodableValue(m_languages));
      return;
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
      if (previous == TTS_STATE_READY) {
        plugin->m_channel->InvokeMethod("speak.onStart", std::move(args));
      } else if (previous == TTS_STATE_PAUSED) {
        plugin->m_channel->InvokeMethod("speak.onContinue", std::move(args));
      }
    } else if (current == TTS_STATE_PAUSED) {
      plugin->m_channel->InvokeMethod("speak.onPause", std::move(args));
    } else if (current == TTS_STATE_READY) {
      if (previous == TTS_STATE_PLAYING || previous == TTS_STATE_PAUSED) {
        plugin->m_channel->InvokeMethod("speak.onCancel", std::move(args));
      }
    }
    plugin->m_current_state = current;
  }

  static void OnUtteranceCompleted(tts_h tts, int utt_id, void *user_data) {
    FlutterTtsTizenPlugin *plugin = (FlutterTtsTizenPlugin *)user_data;
    tts_stop(plugin->m_tts);
    LOG_INFO("[TTS] Utterance (%d) is completed", true);
    if (plugin->m_await_speak_completion) {
      plugin->m_speaking = false;
      plugin->m_result->Success();
    }
    std::unique_ptr<flutter::EncodableValue> args =
        std::make_unique<flutter::EncodableValue>(true);
    plugin->m_channel->InvokeMethod("speak.onComplete", std::move(args));
  }

  static bool OnSupportedVoices(tts_h tts, const char *language, int voice_type,
                                void *user_data) {
    if (nullptr != language) {
      FlutterTtsTizenPlugin *plugin = (FlutterTtsTizenPlugin *)user_data;

      plugin->m_languages.push_back(flutter::EncodableValue(language));
      LOG_INFO("[TTS] Supported Voices - Language(%s), Type(%d)", language,
               voice_type);
      return true;
    }
    return false;
  }

  tts_h m_tts;
  tts_state_e m_current_state;
  bool m_speaking;
  bool m_await_speak_completion;
  std::string m_language;
  int m_voice_type;
  int m_speed;
  flutter::EncodableList m_languages;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> m_channel;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> m_result;
};

void FlutterTtsTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterTtsTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
