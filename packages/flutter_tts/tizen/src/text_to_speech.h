// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TEXT_TO_SPEACH_H_
#define FLUTTER_PLUGIN_TEXT_TO_SPEACH_H_

#include <tts.h>

#include <functional>
#include <string>
#include <vector>

using OnStateChangedCallback =
    std::function<void(tts_state_e previous, tts_state_e current)>;
using OnUtteranceCompletedCallback = std::function<void(int utt_id)>;
using OnErrorCallback = std::function<void(int utt_id, tts_error_e reason)>;

class TextToSpeech {
 public:
  TextToSpeech() {
    // TODO : Handle initialization failure cases
    // Rarely, initializing TextToSpeech can fail. IMO, in this case,
    // we should throw an exception which means that the TextToSpeech instance
    // creation failed. In addition, we should consider catching the exception
    // and propagating it to the flutter side. however, I think this is optional
    // because flutter side is not expecting any errors.
    Init();
    RegisterTtsCallback();
    Prepare();
  }

  ~TextToSpeech() {
    UnregisterTtsCallback();
    Deinit();
  }

  void SetOnStateChanagedCallback(OnStateChangedCallback callback) {
    on_state_changed_ = callback;
  }

  void SetOnUtteranceCompletedCallback(OnUtteranceCompletedCallback callback) {
    on_utterance_completed_ = callback;
  }

  void SetErrorCallback(OnErrorCallback callback) { on_error_ = callback; }

  void OnStateChanged(tts_state_e previous, tts_state_e current);

  void OnUtteranceCompleted(int utt_id) {
    on_utterance_completed_(utt_id);
    ClearUttId();
  }

  void OnError(int utt_id, tts_error_e reason) { on_error_(utt_id, reason); }

  std::string GetDefaultLanguage() { return default_language_; }

  void SetDefaultLanguage(std::string language) {
    default_language_ = language;
  }

  std::vector<std::string> &GetSupportedLanaguages();

  tts_state_e GetState();

  bool AddText(std::string text);

  bool Speak();

  bool Stop();

  bool Pause();

  bool SetVolume(double volume_rate);

  bool GetSpeedRange(int *min, int *normal, int *max);

  void SetTtsSpeed(int speed) { tts_speed_ = speed; }

  int GetUttId() { return utt_id_; }

 private:
  void Init();
  void Deinit();
  void Prepare();
  void RegisterTtsCallback();
  void UnregisterTtsCallback();
  void HandleAwaitSpeakCompletion(tts_state_e previous, tts_state_e current);
  void ClearUttId() { utt_id_ = 0; }

  void SwitchVolumeOnStateChange(tts_state_e previous, tts_state_e current);
  bool SetSpeechVolumeInternal(int volume);
  int GetSpeechVolumeInternal();

  tts_h tts_ = nullptr;
  std::string default_language_;
  int default_voice_type_ = TTS_VOICE_TYPE_AUTO;
  int tts_speed_ = TTS_SPEED_AUTO;
  int utt_id_ = 0;
  int tts_volume_ = 0;
  int system_volume_ = 0;
  int system_max_volume_ = 0;
  std::vector<std::string> supported_lanaguages_;

  OnStateChangedCallback on_state_changed_;
  OnUtteranceCompletedCallback on_utterance_completed_;
  OnErrorCallback on_error_;
};

#endif  // FLUTTER_PLUGIN_TEXT_TO_SPEACH_H_
