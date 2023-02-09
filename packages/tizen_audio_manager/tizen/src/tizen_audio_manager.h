// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_AUDIO_MANAGER_H_
#define FLUTTER_PLUGIN_TIZEN_AUDIO_MANAGER_H_

#include <sound_manager.h>

#include <functional>
#include <string>

using OnVolumeChanged =
    std::function<void(const std::string& type_str, int32_t volume)>;

class TizenAudioManager {
 public:
  TizenAudioManager() {}
  ~TizenAudioManager() {}

  std::string GetCurrentPlaybackType();

  int32_t GetMaxVolume(const std::string& type);

  void SetVolume(const std::string& type, int32_t volume);

  int32_t GetVolume(const std::string& type);

  void RegisterVolumeChangedCallback(OnVolumeChanged volume_changed_callback);

  void UnregisterVolumeChangedCallback();

 private:
  int32_t volume_changed_cb_id_ = 0;
  OnVolumeChanged volume_changed_callback_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_TIZEN_AUDIO_MANAGER_H_
