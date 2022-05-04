#ifndef FLUTTER_PLUGIN_AUDIO_PLAYER_H_
#define FLUTTER_PLUGIN_AUDIO_PLAYER_H_

#include <Ecore.h>
#include <player.h>

#include <functional>
#include <string>
#include <vector>

#include "tizen_result.h"

enum class ReleaseMode { kRelease, kLoop, kStop };

using PreparedListener =
    std::function<void(const std::string &player_id, int duration)>;
using UpdatePositionListener = std::function<void(
    const std::string &player_id, const int duration, const int position)>;
using SeekCompletedListener = std::function<void(const std::string &player_id)>;
using PlayCompletedListener = std::function<void(const std::string &player_id)>;
using ErrorListener = std::function<void(const std::string &player_id,
                                         const std::string &message)>;

class AudioPlayer {
 public:
  AudioPlayer(const std::string &player_id, bool low_latency,
              PreparedListener prepared_listener,
              UpdatePositionListener update_position_listener,
              SeekCompletedListener seek_completed_listener,
              PlayCompletedListener play_completed_listener,
              ErrorListener error_listener);
  ~AudioPlayer();

  TizenResult Play();
  TizenResult Pause();
  TizenResult Stop();
  void Release();
  TizenResult Seek(int position);  // milliseconds

  // If you use HTTP or RTSP, URI must start with "http://" or "rtsp://".
  // The default protocol is "file://".
  TizenResult SetUrl(const std::string &url);
  TizenResult SetDataSource(std::vector<uint8_t> &data);
  TizenResult SetVolume(double volume);
  TizenResult SetPlaybackRate(double rate);
  TizenResult SetReleaseMode(ReleaseMode mode);
  TizenResult GetDuration(int &out);
  TizenResult GetCurrentPosition(int &out);
  std::string GetPlayerId() const { return player_id_; }
  TizenResult IsPlaying(bool &out);

 private:
  // The player state should be none before calling this function.
  TizenResult CreatePlayer();
  // The player state should be idle before calling this function.
  TizenResult PreparePlayer();
  TizenResult ResetPlayer();
  void StartPositionUpdates();
  TizenResult GetPlayerState(player_state_e &out);

  static void OnPrepared(void *data);
  static void OnSeekCompleted(void *data);
  static void OnPlayCompleted(void *data);
  static void OnInterrupted(player_interrupted_code_e code, void *data);
  static void OnError(int code, void *data);
  static Eina_Bool OnPositionUpdate(void *data);

  player_h player_ = nullptr;
  std::string player_id_;
  bool low_latency_;
  std::string url_;
  std::vector<uint8_t> audio_data_;
  double volume_ = 1.0;
  double playback_rate_ = 1.0;
  ReleaseMode release_mode_ = ReleaseMode::kRelease;
  int should_seek_to_ = -1;
  bool preparing_ = false;
  bool seeking_ = false;
  bool should_play_ = false;
  Ecore_Timer *timer_ = nullptr;
  PreparedListener prepared_listener_;
  UpdatePositionListener update_position_listener_;
  SeekCompletedListener seek_completed_listener_;
  PlayCompletedListener play_completed_listener_;
  ErrorListener error_listener_;
};

#endif  // FLUTTER_PLUGIN_AUDIO_PLAYER_H_
