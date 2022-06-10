// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "text_to_speech.h"

#include <sound_manager.h>

#include "log.h"

namespace {

void OnStateChanged(tts_h tts, tts_state_e previous, tts_state_e current,
                    void *user_data) {
  TextToSpeech *self = static_cast<TextToSpeech *>(user_data);
  self->OnStateChanged(previous, current);
}

void OnUtteranceCompleted(tts_h tts, int utt_id, void *user_data) {
  TextToSpeech *self = static_cast<TextToSpeech *>(user_data);
  self->OnUtteranceCompleted(utt_id);
  // Explicitly call stop method to change the tts state to ready.
  self->Stop();
}

void OnError(tts_h tts, int utt_id, tts_error_e reason, void *user_data) {
  TextToSpeech *self = static_cast<TextToSpeech *>(user_data);
  self->OnError(utt_id, reason);
}

}  // namespace

void TextToSpeech::Initialize() {
  TtsCreate();
  RegisterTtsCallback();
  Prepare();
}

void TextToSpeech::TtsCreate() {
  int ret = tts_create(&tts_);
  if (ret != TTS_ERROR_NONE) {
    tts_ = nullptr;
    throw TextToSpeechError("tts_create", ret);
  }
}

void TextToSpeech::TtsDestroy() {
  if (tts_) {
    tts_destroy(tts_);
    tts_ = nullptr;
  }
}

void TextToSpeech::Prepare() {
  char *language = nullptr;
  int voice_type = 0;
  int ret = tts_get_default_voice(tts_, &language, &voice_type);
  if (ret == TTS_ERROR_NONE) {
    default_language_ = language;
    default_voice_type_ = voice_type;
    free(language);
  }
  ret = sound_manager_get_max_volume(SOUND_TYPE_VOICE, &system_max_volume_);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    throw TextToSpeechError("sound_manager_get_max_volume", ret);
  }
  system_volume_ = GetSpeechVolumeInternal();
  tts_volume_ = system_volume_;

  ret = tts_prepare(tts_);
  if (ret != TTS_ERROR_NONE) {
    throw TextToSpeechError("tts_prepare", ret);
  }
}

void TextToSpeech::RegisterTtsCallback() {
  tts_set_state_changed_cb(tts_, ::OnStateChanged, (void *)this);
  tts_set_utterance_completed_cb(tts_, ::OnUtteranceCompleted, (void *)this);
  tts_set_error_cb(tts_, ::OnError, (void *)this);
}

void TextToSpeech::UnregisterTtsCallback() {
  tts_unset_state_changed_cb(tts_);
  tts_unset_utterance_completed_cb(tts_);
  tts_unset_error_cb(tts_);
}

void TextToSpeech::OnStateChanged(tts_state_e previous, tts_state_e current) {
  SwitchVolumeOnStateChange(previous, current);
  on_state_changed_(previous, current);
}

std::vector<std::string> &TextToSpeech::GetSupportedLanaguages() {
  if (supported_lanaguages_.size() == 0) {
    tts_foreach_supported_voices(
        tts_,
        [](tts_h tts, const char *language, int voice_type,
           void *user_data) -> bool {
          if (language == nullptr) {
            return false;
          }
          TextToSpeech *self = static_cast<TextToSpeech *>(user_data);
          self->supported_lanaguages_.push_back(std::string(language));
          LOG_INFO("[TTS] Supported Voices - Language(%s), Type(%d)", language,
                   voice_type);
          return true;
        },
        (void *)this);

    supported_lanaguages_.erase(
        unique(supported_lanaguages_.begin(), supported_lanaguages_.end()),
        supported_lanaguages_.end());
  }
  return supported_lanaguages_;
}

TtsState TextToSpeech::GetState() {
  tts_state_e state;
  int ret = tts_get_state(tts_, &state);
  if (ret != TTS_ERROR_NONE) {
    throw TextToSpeechError(ret);
  }
  switch (state) {
    case TTS_STATE_CREATED:
      return TtsState::kCreated;
    case TTS_STATE_READY:
    default:
      return TtsState::kReady;
    case TTS_STATE_PLAYING:
      return TtsState::kPlaying;
    case TTS_STATE_PAUSED:
      return TtsState::kPaused;
  }
}

void TextToSpeech::AddText(std::string text) {
  int ret = tts_add_text(tts_, text.c_str(), default_language_.c_str(),
                         default_voice_type_, tts_speed_, &utt_id_);
  if (ret != TTS_ERROR_NONE) {
    throw TextToSpeechError("tts_add_text", ret);
  }
}

void TextToSpeech::Speak() {
  int ret = tts_play(tts_);
  if (ret != TTS_ERROR_NONE) {
    throw TextToSpeechError("tts_play", ret);
  }
}

void TextToSpeech::Stop() {
  int ret = tts_stop(tts_);
  if (ret != TTS_ERROR_NONE) {
    throw TextToSpeechError("tts_stop", ret);
  }
}

void TextToSpeech::Pause() {
  int ret = tts_pause(tts_);
  if (ret != TTS_ERROR_NONE) {
    throw TextToSpeechError("tts_pause", ret);
  }
}

void TextToSpeech::SetVolume(double volume_rate) {
  tts_volume_ = static_cast<int>(system_max_volume_ * volume_rate);
  // Change volume instantly when tts is playing.
  if (GetState() == TtsState::kPlaying) {
    SetSpeechVolumeInternal(tts_volume_);
  }
}

void TextToSpeech::GetSpeedRange(int *min, int *normal, int *max) {
  int ret = tts_get_speed_range(tts_, min, normal, max);
  if (ret != TTS_ERROR_NONE) {
    throw TextToSpeechError(ret);
  }
}

void TextToSpeech::SwitchVolumeOnStateChange(tts_state_e previous,
                                             tts_state_e current) {
  if (previous == TTS_STATE_PLAYING) {
    // When playback is finished, save the tts_volume that might've changed
    // from hardware key, then restore the system volume.
    tts_volume_ = GetSpeechVolumeInternal();
    SetSpeechVolumeInternal(system_volume_);
  } else if (current == TTS_STATE_PLAYING) {
    // Before playback starts, save the system volume to restore when playback
    // is finished, then set the tts volume.
    system_volume_ = GetSpeechVolumeInternal();
    SetSpeechVolumeInternal(tts_volume_);
  }
}

int TextToSpeech::GetSpeechVolumeInternal() {
  int volume;
  int ret = sound_manager_get_volume(SOUND_TYPE_VOICE, &volume);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    throw TextToSpeechError("sound_manager_get_volume", ret);
  }
  return volume;
}

void TextToSpeech::SetSpeechVolumeInternal(int volume) {
  int ret = sound_manager_set_volume(SOUND_TYPE_VOICE, volume);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    throw TextToSpeechError("sound_manager_set_volume", ret);
  }
}
