#ifndef AUDIO_PLAYER_H_
#define AUDIO_PLAYER_H_

#include <player.h>

#include <functional>
#include <string>
#include <vector>

#include "audio_player_options.h"

using PreparedListener =
    std::function<void(const std::string &player_id, int duration)>;
using StartPlayingListener = std::function<void(const std::string &player_id)>;
using SeekCompletedListener = std::function<void(const std::string &player_id)>;
using PlayCompletedListener = std::function<void(const std::string &player_id)>;
using ErrorListener = std::function<void(const std::string &player_id,
                                         const std::string &message)>;

class AudioPlayer {
 public:
  AudioPlayer(const std::string &player_id, bool low_latency,
              PreparedListener prepared_listener,
              StartPlayingListener start_playing_listener,
              SeekCompletedListener seek_completed_listener,
              PlayCompletedListener play_completed_listener,
              ErrorListener error_listener);
  ~AudioPlayer();

  void Play();
  void Pause();
  void Stop();
  void Release();
  void Seek(int position);  // milliseconds

  // If you use HTTP or RTSP, URI must start with "http://" or "rtsp://".
  // The default protocol is "file://".
  void SetUrl(const std::string &url);
  void SetDataSource(std::vector<uint8_t> &data);
  void SetVolume(double volume);
  void SetPlaybackRate(double rate);
  void SetReleaseMode(ReleaseMode mode);
  int GetDuration();
  int GetCurrentPosition();
  std::string GetPlayerId() const;
  bool IsPlaying();

 private:
  // the player state should be none before call this function
  void CreatePlayer();
  // the player state should be idle before call this function
  void PreparePlayer();
  void ResetPlayer();
  player_state_e GetPlayerState();
  void HandleResult(const std::string &func_name, int result);

  static void OnPrepared(void *data);
  static void OnSeekCompleted(void *data);
  static void OnPlayCompleted(void *data);
  static void OnInterrupted(player_interrupted_code_e code, void *data);
  static void OnErrorOccurred(int code, void *data);

  player_h player_;
  std::string player_id_;
  bool low_latency_;
  std::string url_;
  std::vector<uint8_t> audio_data_;
  double volume_;
  double playback_rate_;
  ReleaseMode release_mode_;
  int should_seek_to_;
  bool preparing_;
  bool seeking_;
  bool should_play_;
  PreparedListener prepared_listener_;
  StartPlayingListener start_playing_listener_;
  SeekCompletedListener seek_completed_listener_;
  PlayCompletedListener play_completed_listener_;
  ErrorListener error_listener_;
};

#endif  // AUDIO_PLAYER_H_
