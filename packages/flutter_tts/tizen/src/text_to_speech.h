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
    std::function<void(tts_state_e previous, tts_state_e current)>;
using UtteranceCompletedCallback = std::function<void(int32_t utt_id)>;
using ErrorCallback = std::function<void(int32_t utt_id, tts_error_e reason)>;

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

  void SetErrorCallback(ErrorCallback callback) { error_callback_ = callback; }

  void OnStateChanged(tts_state_e previous, tts_state_e current) {
    SwitchVolumeOnStateChange(previous, current);
    state_changed_callback_(previous, current);
  }

  void OnUtteranceCompleted(int32_t utt_id) {
    utterance_completed_callback_(utt_id);
    ClearUttId();
  }

  void OnError(int32_t utt_id, tts_error_e reason) {
    error_callback_(utt_id, reason);
  }

  std::string GetDefaultLanguage() { return default_language_; }

  void SetDefaultLanguage(std::string language) {
    default_language_ = language;
  }

  std::vector<std::string> &GetSupportedLanaguages();

  std::optional<TtsState> GetState();

  bool AddText(std::string text);

  bool Speak();

  bool Stop();

  bool Pause();

  bool SetVolume(double volume_rate);

  bool GetSpeedRange(int32_t *min, int32_t *normal, int32_t *max);

  void SetTtsSpeed(int32_t speed) { tts_speed_ = speed; }

  int32_t GetUttId() { return utt_id_; }

 private:
  void Prepare();
  void RegisterCallbacks();
  void UnregisterCallbacks();
  void HandleAwaitSpeakCompletion(tts_state_e previous, tts_state_e current);
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
  ErrorCallback error_callback_;
};

#endif  // FLUTTER_PLUGIN_TEXT_TO_SPEACH_H_
