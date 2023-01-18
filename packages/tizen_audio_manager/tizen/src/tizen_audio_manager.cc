// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_audio_manager.h"

#include <memory>
#include <string>
#include <vector>

#include "log.h"
#include "tizen_audio_manager_error.h"

namespace {

constexpr std::array<std::pair<std::string_view, sound_type_e>, 8> kSoundTypes{
    {{"system", SOUND_TYPE_SYSTEM},
     {"notification", SOUND_TYPE_NOTIFICATION},
     {"alarm", SOUND_TYPE_ALARM},
     {"ringtone", SOUND_TYPE_RINGTONE},
     {"media", SOUND_TYPE_MEDIA},
     {"call", SOUND_TYPE_CALL},
     {"voip", SOUND_TYPE_VOIP},
     {"voice", SOUND_TYPE_VOICE}}};

bool ConvertSoundTypeToString(sound_type_e type, std::string &str) {
  auto iter = kSoundTypes.begin();
  for (; iter != kSoundTypes.end(); iter++) {
    if (iter->second == type) {
      str = iter->first;
      return true;
    }
  }
  return false;
}

bool ConvertStringToSoundType(const std::string &str, sound_type_e &type) {
  auto iter = kSoundTypes.begin();
  for (; iter != kSoundTypes.end(); iter++) {
    if (iter->first == str) {
      type = iter->second;
      return true;
    }
  }
  return false;
}

}  // namespace

std::string TizenAudioManager::getCurrentPlaybackType() {
  sound_type_e type;

  const int ret = sound_manager_get_current_sound_type(&type);

  if (ret == SOUND_MANAGER_ERROR_NONE) {
    std::string type_str;

    if (ConvertSoundTypeToString(type, type_str)) {
      return type_str;
    } else {
      LOG_ERROR("Failed to convert type to string.");
      throw TizenAudioManagerError("UnknownType",
                                   "Failed to convert type to string.");
    }
  } else if (ret == SOUND_MANAGER_ERROR_NO_PLAYING_SOUND) {
    return "none";
  } else {
    throw TizenAudioManagerError("sound_manager_get_current_sound_type",
                                 get_error_message(ret));
  }
}

int32_t TizenAudioManager::GetMaxVolume(const std::string &type) {
  sound_type_e sound_type;
  if (!ConvertStringToSoundType(type, sound_type)) {
    LOG_ERROR("Failed to convert string to type.");
    throw TizenAudioManagerError("UnknownType",
                                 "Failed to convert string to type.");
  }

  int max;
  const int ret = sound_manager_get_max_volume(sound_type, &max);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    throw TizenAudioManagerError("sound_manager_get_max_volume",
                                 get_error_message(ret));
  }
  return max;
}

void TizenAudioManager::SetVolume(const std::string &type, int32_t volume) {
  sound_type_e sound_type;
  if (!ConvertStringToSoundType(type, sound_type)) {
    LOG_ERROR("Failed to convert string to type.");
    throw TizenAudioManagerError("UnknownType",
                                 "Failed to convert string to type.");
  }

  const int ret = sound_manager_set_volume(sound_type, volume);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    throw TizenAudioManagerError("sound_manager_set_volume",
                                 get_error_message(ret));
  }
}

int32_t TizenAudioManager::GetVolume(const std::string &type) {
  sound_type_e sound_type;
  if (!ConvertStringToSoundType(type, sound_type)) {
    LOG_ERROR("Failed to convert string to type.");
    throw TizenAudioManagerError("UnknownType",
                                 "Failed to convert string to type.");
  }
  int volume;
  const int ret = sound_manager_get_volume(sound_type, &volume);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    throw TizenAudioManagerError("sound_manager_get_volume",
                                 get_error_message(ret));
  }
  return volume;
}

void TizenAudioManager::RegisterVolumeChangedCallback(
    OnVolumeChanged volume_changed_callback) {
  volume_changed_callback_ = volume_changed_callback;
  const int ret = sound_manager_add_volume_changed_cb(
      [](sound_type_e type, unsigned int volume, void *user_data) {
        TizenAudioManager *audio_manager =
            static_cast<TizenAudioManager *>(user_data);
        std::string type_str;
        if (!ConvertSoundTypeToString(type, type_str)) {
          throw TizenAudioManagerError("UnknownType",
                                       "Failed to convert type to string.");
        }
        audio_manager->volume_changed_callback_(type_str, volume);
      },
      this, &volume_changed_cb_id_);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get current audio type: %s", get_error_message(ret));
    throw TizenAudioManagerError("sound_manager_add_volume_changed_cb",
                                 get_error_message(ret));
  }
}

void TizenAudioManager::UnregisterVolumeChangedCallback() {
  if (volume_changed_callback_) {
    sound_manager_remove_volume_changed_cb(volume_changed_cb_id_);
    volume_changed_callback_ = nullptr;
  }
}
