// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "text_to_speech.h"

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

void TextToSpeech::Init() {
  int ret = tts_create(&tts_);
  if (ret != TTS_ERROR_NONE) {
    tts_ = nullptr;
    LOG_ERROR("[TTS] tts_create failed: %s", get_error_message(ret));
  }
}

void TextToSpeech::Deinit() {
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

  ret = tts_prepare(tts_);
  if (ret != TTS_ERROR_NONE) {
    LOG_ERROR("[TTS] tts_prepare failed: %s", get_error_message(ret));
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

tts_state_e TextToSpeech::GetState() {
  tts_state_e state;
  int ret = tts_get_state(tts_, &state);
  if (ret != TTS_ERROR_NONE) {
    LOG_ERROR("[TTS] tts_prepare failed: %s", get_error_message(ret));
  }
  return state;
}

bool TextToSpeech::AddText(std::string text) {
  int ret = tts_add_text(tts_, text.c_str(), default_language_.c_str(),
                         default_voice_type_, tts_speed_, &utt_id_);
  if (ret != TTS_ERROR_NONE) {
    LOG_ERROR("[TTS] tts_add_text failed: %s", get_error_message(ret));
    return false;
  }
  return true;
}

bool TextToSpeech::Speak() {
  int ret = tts_play(tts_);
  if (ret != TTS_ERROR_NONE) {
    LOG_ERROR("[TTS] tts_play failed: %s", get_error_message(ret));
    return false;
  }
  return true;
}

bool TextToSpeech::Stop() {
  int ret = tts_stop(tts_);
  if (ret != TTS_ERROR_NONE) {
    LOG_ERROR("[TTS] tts_stop failed: %s", get_error_message(ret));
    return false;
  }
  return true;
}

bool TextToSpeech::Pause() {
  int ret = tts_pause(tts_);
  if (ret != TTS_ERROR_NONE) {
    LOG_ERROR("[TTS] tts_pause failed: %s", get_error_message(ret));
    return false;
  }
  return true;
}

bool TextToSpeech::GetSpeedRange(int *min, int *nomal, int *max) {
  int ret = tts_get_speed_range(tts_, min, nomal, max);
  if (ret != TTS_ERROR_NONE) {
    LOG_ERROR("[TTS] tts_get_speed_range failed: %s", get_error_message(ret));
    return false;
  }
  return true;
}
