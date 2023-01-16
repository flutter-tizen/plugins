// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_AUDIO_MANAGER_H_
#define FLUTTER_PLUGIN_AUDIO_MANAGER_H_

#include <sound_manager.h>

#include <functional>
#include <string>

using VolumeChangedListener =
    std::function<void(std::string type_str, int32_t volume)>;

class AudioManager {
 public:
  AudioManager() {}
  ~AudioManager() {}

  std::string getCurrentPlaybackType();

  int32_t getMaxLevel(std::string getMaxLevel);

  void SetLevel(std::string type, int32_t volume);

  int32_t GetLevel(std::string type);

  static void OnVolumeChanged(sound_type_e type, unsigned int volume,
                              void *user_data);

  void AddVolumeChangedCallback(VolumeChangedListener volume_changed_listener);

  void RemoveVolumeChangedCallback();

 private:
  bool registered_volume_changed_cb_ = false;
  int32_t volume_changed_cb_id_ = 0;
  VolumeChangedListener volume_changed_listener_;
};

#endif  // FLUTTER_PLUGIN_AUDIO_MANAGER_H_
