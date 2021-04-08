#ifndef AUDIO_PLAYER_H_
#define AUDIO_PLAYER_H_

#include <player.h>

#include <functional>
#include <string>
#include <vector>

#include "audio_player_options.h"

using PreparedListener =
    std::function<void(const std::string &playerId, int duration)>;
using StartPlayingListener = std::function<void(const std::string &playerId)>;
using SeekCompletedListener = std::function<void(const std::string &playerId)>;
using PlayCompletedListener = std::function<void(const std::string &playerId)>;
using ErrorListener = std::function<void(const std::string &playerId,
                                         const std::string &message)>;

class AudioPlayer {
 public:
  AudioPlayer(const std::string &playerId, bool isLowLatency,
              PreparedListener preparedListener,
              StartPlayingListener startPlayingListener,
              SeekCompletedListener seekCompletedListener,
              PlayCompletedListener playCompletedListener,
              ErrorListener errorListener);
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
  void HandleResult(const std::string &funcName, int result);

  static void OnPrepared(void *data);
  static void OnSeekCompleted(void *data);
  static void OnPlayCompleted(void *data);
  static void OnInterrupted(player_interrupted_code_e code, void *data);
  static void OnErrorOccurred(int code, void *data);

  player_h player_;
  std::string playerId_;
  bool isLowLatency_;
  std::string url_;
  std::vector<uint8_t> audioData_;
  double volume_;
  double playbackRate_;
  ReleaseMode releaseMode_;
  int shouldSeekTo_;
  bool isPreparing_;
  bool isSeeking_;
  bool shouldPlay_;
  PreparedListener preparedListener_;
  StartPlayingListener startPlayingListener_;
  SeekCompletedListener seekCompletedListener_;
  PlayCompletedListener playCompletedListener_;
  ErrorListener errorListener_;
};

#endif  // AUDIO_PLAYER_H_
