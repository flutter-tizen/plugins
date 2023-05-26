// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TEXT_TO_SPEACH_H_
#define FLUTTER_PLUGIN_TEXT_TO_SPEACH_H_

#include <tts.h>

#include <functional>
#include <optional>
#include <string>
#include <vector>

enum class TtsState { kCreated, kReady, kPlaying, kPaused };

using StateChangedCallback =
    std::function<void(TtsState previous, TtsState current)>;
using UtteranceCompletedCallback = std::function<void(int32_t utt_id)>;

class TextToSpeech {
 public:
  TextToSpeech() = default;

  ~TextToSpeech();

  bool Initialize();

  void SetStateChanagedCallback(StateChangedCallback callback) {
    state_changed_callback_ = callback;
  }

  void SetUtteranceCompletedCallback(UtteranceCompletedCallback callback) {
    utterance_completed_callback_ = callback;
  }

  std::string GetDefaultLanguage() { return default_language_; }

  void SetDefaultLanguage(const std::string &language) {
    default_language_ = language;
  }

  std::vector<std::string> &GetSupportedLanaguages();

  std::optional<std::pair<std::string, std::string>> GetDefaultVoice();

  std::optional<int32_t> GetMaxSpeechInputLength();

  std::optional<TtsState> GetState();

  bool AddText(const std::string &text);

  bool Speak();

  bool Stop();

  bool Pause();

  bool SetVolume(double volume);

  bool GetSpeedRange(int32_t *min, int32_t *normal, int32_t *max);

  void SetTtsSpeed(int32_t speed) { tts_speed_ = speed; }

  int32_t GetUttId() { return utt_id_; }

 private:
  void Prepare();
  void RegisterCallbacks();
  void UnregisterCallbacks();

  void ClearUttId() { utt_id_ = 0; }

  void SwitchVolumeOnStateChange(tts_state_e previous, tts_state_e current);
  bool SetSpeechVolumeInternal(int32_t volume);
  int32_t GetSpeechVolumeInternal();

  tts_h tts_ = nullptr;
  std::string default_language_;
  int32_t default_voice_type_ = TTS_VOICE_TYPE_AUTO;
  int32_t tts_speed_ = TTS_SPEED_AUTO;
  int32_t utt_id_ = 0;
  int32_t tts_volume_ = 0;
  int32_t system_volume_ = 0;
  int32_t system_max_volume_ = 0;
  std::vector<std::string> supported_lanaguages_;

  StateChangedCallback state_changed_callback_;
  UtteranceCompletedCallback utterance_completed_callback_;
};

#endif  // FLUTTER_PLUGIN_TEXT_TO_SPEACH_H_
