// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_AUDIO_PLAYER_H_
#define FLUTTER_PLUGIN_AUDIO_PLAYER_H_

#include <Ecore.h>
#include <player.h>

#include <functional>
#include <string>
#include <vector>

enum class ReleaseMode { kRelease, kLoop, kStop };

using PreparedListener =
    std::function<void(const std::string &player_id, bool is_prepared)>;
using DurationListener =
    std::function<void(const std::string &player_id, int32_t duration)>;
using CurrentPositionListener =
    std::function<void(const std::string &player_id, const int32_t position)>;
using SeekCompletedListener = std::function<void(const std::string &player_id)>;
using PlayCompletedListener = std::function<void(const std::string &player_id)>;
using LogListener = std::function<void(const std::string &player_id,
                                       const std::string &message)>;

class AudioPlayer {
 public:
  AudioPlayer(const std::string &player_id, PreparedListener prepared_listener,
              DurationListener duration_listener,
              SeekCompletedListener seek_completed_listener,
              PlayCompletedListener play_completed_listener,
              LogListener log_listener);

  ~AudioPlayer();

  void Play();
  void Pause();
  void Stop();
  void Release();
  void Seek(int32_t position);  // milliseconds

  // If you use HTTP or RTSP, URI must start with "http://" or "rtsp://".
  // The default protocol is "file://".
  void SetUrl(const std::string &url);
  void SetDataSource(std::vector<uint8_t> &data);
  void SetVolume(double volume);
  void SetPlaybackRate(double playback_rate);
  void SetReleaseMode(ReleaseMode mode);
  void SetLatencyMode(bool low_latency);
  int32_t GetDuration();
  int32_t GetCurrentPosition();
  std::string GetPlayerId() const { return player_id_; }
  bool IsPlaying();

 private:
  // The player state should be none before calling this function.
  void CreatePlayer();
  // The player state should be idle before calling this function.
  void PreparePlayer();
  void ResetPlayer();
  void StartPositionUpdates();
  player_state_e GetPlayerState();

  static void OnPrepared(void *data);
  static void OnSeekCompleted(void *data);
  // Callback invoked when the currently playing audio completes.
  // This will only be called when ReleaseMode isn't ReleaseMode::kLoop.
  static void OnPlayCompleted(void *data);
  static void OnInterrupted(player_interrupted_code_e code, void *data);
  static void OnError(int code, void *data);
  static Eina_Bool OnPositionUpdate(void *data);

  player_h player_ = nullptr;
  const std::string player_id_;
  std::string url_;
  std::vector<uint8_t> audio_data_;
  double volume_ = 1.0;
  double playback_rate_ = 1.0;
  ReleaseMode release_mode_ = ReleaseMode::kRelease;
  int should_seek_to_ = -1;
  bool preparing_ = false;
  bool seeking_ = false;
  bool should_play_ = false;
  bool released_ = false;
  Ecore_Timer *timer_ = nullptr;

  PreparedListener prepared_listener_;
  DurationListener duration_listener_;
  SeekCompletedListener seek_completed_listener_;
  PlayCompletedListener play_completed_listener_;
  LogListener log_listener_;
};

#endif  // FLUTTER_PLUGIN_AUDIO_PLAYER_H_
