#include "audio_player.h"

#include "audio_player_error.h"
#include "log.h"

static std::string StateToString(player_state_e state) {
  std::string result;
  switch (state) {
    case PLAYER_STATE_NONE:
      result = "PLAYER_STATE_NONE";
      break;
    case PLAYER_STATE_IDLE:
      result = "PLAYER_STATE_IDLE";
      break;
    case PLAYER_STATE_READY:
      result = "PLAYER_STATE_READY";
      break;
    case PLAYER_STATE_PLAYING:
      result = "PLAYER_STATE_PLAYING";
      break;
    case PLAYER_STATE_PAUSED:
      result = "PLAYER_STATE_PAUSED";
      break;
  }
  return result;
}

static std::string PlayerErrorToString(int code) {
  std::string result;
  switch (code) {
    case PLAYER_ERROR_NONE:
      result = "player - Successful";
      break;
    case PLAYER_ERROR_OUT_OF_MEMORY:
      result = "player - Out of memory";
      break;
    case PLAYER_ERROR_INVALID_PARAMETER:
      result = "player - Invalid parameter";
      break;
    case PLAYER_ERROR_NO_SUCH_FILE:
      result = "player - No such file or directory";
      break;
    case PLAYER_ERROR_INVALID_OPERATION:
      result = "player - Invalid operation";
      break;
    case PLAYER_ERROR_FILE_NO_SPACE_ON_DEVICE:
      result = "player - No space left on the device";
      break;
    case PLAYER_ERROR_FEATURE_NOT_SUPPORTED_ON_DEVICE:
      result = "player - Not supported Feature";
      break;
    case PLAYER_ERROR_SEEK_FAILED:
      result = "player - Seek operation failure";
      break;
    case PLAYER_ERROR_INVALID_STATE:
      result = "player - Invalid state";
      break;
    case PLAYER_ERROR_NOT_SUPPORTED_FILE:
      result = "player - File format not supported";
      break;
    case PLAYER_ERROR_INVALID_URI:
      result = "player - Invalid URI";
      break;
    case PLAYER_ERROR_SOUND_POLICY:
      result = "player - Sound policy error";
      break;
    case PLAYER_ERROR_CONNECTION_FAILED:
      result = "player - Streaming connection failed";
      break;
    case PLAYER_ERROR_VIDEO_CAPTURE_FAILED:
      result = "player - Video capture failed";
      break;
    case PLAYER_ERROR_DRM_EXPIRED:
      result = "player - Expired license";
      break;
    case PLAYER_ERROR_DRM_NO_LICENSE:
      result = "player - No license";
      break;
    case PLAYER_ERROR_DRM_FUTURE_USE:
      result = "player - License for future use";
      break;
    case PLAYER_ERROR_DRM_NOT_PERMITTED:
      result = "player - Format not permitted";
      break;
    case PLAYER_ERROR_RESOURCE_LIMIT:
      result = "player - Resource limit";
      break;
    case PLAYER_ERROR_PERMISSION_DENIED:
      result = "player - Permission denied";
      break;
    case PLAYER_ERROR_SERVICE_DISCONNECTED:
      result = "player - Socket connection lost";
      break;
    case PLAYER_ERROR_BUFFER_SPACE:
      result = "player - No buffer space available";
      break;
    case PLAYER_ERROR_NOT_SUPPORTED_AUDIO_CODEC:
      result = "player - Not supported audio codec but video can be played";
      break;
    case PLAYER_ERROR_NOT_SUPPORTED_VIDEO_CODEC:
      result = "player - Not supported video codec but audio can be played";
      break;
    case PLAYER_ERROR_NOT_SUPPORTED_SUBTITLE:
      result = "player - Not supported subtitle format";
      break;
    case PLAYER_ERROR_NOT_SUPPORTED_FORMAT:
      result = "player - Not supported format";
      break;
    case PLAYER_ERROR_NOT_AVAILABLE:
      result = "player - Not available operation";
      break;
    default:
      result = "player - Unkonwn error";
      break;
  }
  return result;
}

AudioPlayer::AudioPlayer(const std::string &playerId, bool isLowLatency,
                         PreparedListener preparedListener,
                         StartPlayingListener startPlayingListener,
                         SeekCompletedListener seekCompletedListener,
                         PlayCompletedListener playCompletedListener,
                         ErrorListener errorListener) {
  LOG_INFO("AudioPlayer %s is constructing...", playerId.c_str());
  playerId_ = playerId;
  isLowLatency_ = isLowLatency;
  preparedListener_ = preparedListener;
  startPlayingListener_ = startPlayingListener;
  seekCompletedListener_ = seekCompletedListener;
  playCompletedListener_ = playCompletedListener;
  errorListener_ = errorListener;

  player_ = nullptr;
  volume_ = 1.0;
  playbackRate_ = 1.0;
  releaseMode_ = RELEASE;
  shouldSeekTo_ = -1;
}

AudioPlayer::~AudioPlayer() {
  LOG_INFO("AudioPlayer %s is destructing...", playerId_.c_str());
  Release();
}

void AudioPlayer::Play() {
  LOG_INFO("AudioPlayer %s will play audio...", playerId_.c_str());
  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_IDLE && isPreparing_) {
    LOG_DEBUG("player is preparing, play will be called in prepared callback");
    shouldPlay_ = true;
    return;
  }

  if (state == PLAYER_STATE_NONE) {
    CreatePlayer();
  }

  int result;
  switch (state) {
    case PLAYER_STATE_NONE:
    case PLAYER_STATE_IDLE:
      if (audioData_.size() > 0) {
        LOG_DEBUG("set audio buffer, buffer size : %d", audioData_.size());
        result = player_set_memory_buffer(player_, (void *)audioData_.data(),
                                          audioData_.size());
        HandleResult("player_set_memory_buffer", result);
      } else {
        LOG_DEBUG("set uri (%s)", url_.c_str());
        result = player_set_uri(player_, url_.c_str());
        HandleResult("player_set_uri", result);
      }
      shouldPlay_ = true;
      PreparePlayer();
      break;
    case PLAYER_STATE_READY:
    case PLAYER_STATE_PAUSED:
      result = player_start(player_);
      HandleResult("player_start", result);
      shouldPlay_ = false;
      startPlayingListener_(playerId_);
      break;
    default:
      LOG_DEBUG("player is already playing audio");
      break;
  }
}

void AudioPlayer::Pause() {
  LOG_INFO("AudioPlayer %s is pausing...", playerId_.c_str());
  if (GetPlayerState() == PLAYER_STATE_PLAYING) {
    int result = player_pause(player_);
    HandleResult("player_pause", result);
  }
  shouldPlay_ = false;
}

void AudioPlayer::Stop() {
  LOG_INFO("AudioPlayer %s is stoping...", playerId_.c_str());
  if (releaseMode_ == RELEASE) {
    Release();
  } else {
    player_state_e state = GetPlayerState();
    if (state == PLAYER_STATE_PLAYING || state == PLAYER_STATE_PAUSED) {
      int result = player_stop(player_);
      HandleResult("player_stop", result);
    }
  }
  shouldPlay_ = false;
  isSeeking_ = false;
}

void AudioPlayer::Release() {
  LOG_INFO("AudioPlayer %s is releasing...", playerId_.c_str());
  if (player_ != nullptr) {
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_destroy(player_);
    player_ = nullptr;
  }
}

void AudioPlayer::Seek(int position) {
  LOG_INFO("AudioPlayer %s is seeking...", playerId_.c_str());
  if (isSeeking_) {
    LOG_DEBUG("player is already seeking, can't seek again");
    return;
  }

  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
      state == PLAYER_STATE_PAUSED) {
    LOG_DEBUG("set play position %d", position);
    isSeeking_ = true;
    int result = player_set_play_position(player_, position, true,
                                          OnSeekCompleted, (void *)this);
    if (result != PLAYER_ERROR_NONE) {
      isSeeking_ = false;
      std::string error = PlayerErrorToString(result);
      LOG_ERROR("player_set_play_position failed : %s", error.c_str());
      throw AudioPlayerError(error, "player_set_play_position failed");
    }
  } else {
    LOG_DEBUG("player is unprepared, do seek in prepared callback");
    shouldSeekTo_ = position;
  }
}

void AudioPlayer::SetUrl(const std::string &url) {
  LOG_INFO("AudioPlayer %s is setting url...", playerId_.c_str());
  if (url != url_) {
    url_ = url;
    ResetPlayer();

    LOG_DEBUG("set new uri (%s)", url.c_str());
    int result = player_set_uri(player_, url.c_str());
    HandleResult("player_set_uri", result);

    PreparePlayer();
  }
  audioData_.clear();
}

void AudioPlayer::SetDataSource(std::vector<uint8_t> &data) {
  LOG_INFO("AudioPlayer %s is setting buffer...", playerId_.c_str());
  if (data != audioData_) {
    audioData_.swap(data);
    ResetPlayer();

    LOG_DEBUG("set audio buffer, buffer size : %d", audioData_.size());
    int result = player_set_memory_buffer(player_, (void *)audioData_.data(),
                                          audioData_.size());
    HandleResult("player_set_memory_buffer", result);

    PreparePlayer();
  }
}

void AudioPlayer::SetVolume(double volume) {
  LOG_INFO("AudioPlayer %s is setting volume %f...", playerId_.c_str(), volume);
  if (volume_ != volume) {
    volume_ = volume;
    if (GetPlayerState() != PLAYER_STATE_NONE) {
      LOG_DEBUG("set volume : %f", volume_);
      int result = player_set_volume(player_, volume_, volume_);
      HandleResult("player_set_volume", result);
    }
  }
}

void AudioPlayer::SetPlaybackRate(double rate) {
  LOG_INFO("AudioPlayer %s is setting plackback rate %f...", playerId_.c_str(),
           rate);
  if (playbackRate_ != rate) {
    playbackRate_ = rate;
    player_state_e state = GetPlayerState();
    if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
        state == PLAYER_STATE_PAUSED) {
      LOG_DEBUG("set plackback rate : %f", rate);
      int result = player_set_playback_rate(player_, rate);
      HandleResult("player_set_volume", result);
    }
  }
}

void AudioPlayer::SetReleaseMode(ReleaseMode mode) {
  LOG_INFO("AudioPlayer %s is setting ReleaseMode %d...", playerId_.c_str(),
           mode);
  if (releaseMode_ != mode) {
    releaseMode_ = mode;
    if (GetPlayerState() != PLAYER_STATE_NONE) {
      LOG_DEBUG("set looping : %d", releaseMode_ == LOOP);
      int result = player_set_looping(player_, (releaseMode_ == LOOP));
      HandleResult("player_set_looping", result);
    }
  }
}

int AudioPlayer::GetDuration() {
  int duration;
  int result = player_get_duration(player_, &duration);
  HandleResult("player_get_duration", result);
  LOG_INFO("audio (%s) duration: %d", url_.c_str(), duration);
  return duration;
}

int AudioPlayer::GetCurrentPosition() {
  int position;
  int result = player_get_play_position(player_, &position);
  HandleResult("player_get_play_position", result);
  LOG_INFO("audio (%s) position: %d", url_.c_str(), position);
  return position;
}

std::string AudioPlayer::GetPlayerId() const { return playerId_; }

bool AudioPlayer::IsPlaying() {
  return (GetPlayerState() == PLAYER_STATE_PLAYING);
}

void AudioPlayer::CreatePlayer() {
  LOG_INFO("create audio player...");
  shouldPlay_ = false;
  isPreparing_ = false;

  int result = player_create(&player_);
  HandleResult("player_create", result);

  if (isLowLatency_) {
    result = player_set_audio_latency_mode(player_, AUDIO_LATENCY_MODE_LOW);
    HandleResult("player_set_audio_latency_mode", result);
  }

  result = player_set_completed_cb(player_, OnPlayCompleted, (void *)this);
  HandleResult("player_set_completed_cb", result);

  result = player_set_interrupted_cb(player_, OnInterrupted, (void *)this);
  HandleResult("player_set_interrupted_cb", result);

  result = player_set_error_cb(player_, OnErrorOccurred, (void *)this);
  HandleResult("player_set_error_cb", result);
}

void AudioPlayer::PreparePlayer() {
  LOG_DEBUG("set volume %f", volume_);
  int result = player_set_volume(player_, volume_, volume_);
  HandleResult("player_set_volume", result);

  LOG_DEBUG("set looping %d", (releaseMode_ == LOOP));
  result = player_set_looping(player_, (releaseMode_ == LOOP));
  HandleResult("player_set_looping", result);

  LOG_DEBUG("prepare audio player asynchronously");
  result = player_prepare_async(player_, OnPrepared, (void *)this);
  HandleResult("player_prepare_async", result);
  isPreparing_ = true;
  isSeeking_ = false;
}

void AudioPlayer::ResetPlayer() {
  LOG_INFO("reset audio player...");
  int result;
  player_state_e state = GetPlayerState();
  switch (state) {
    case PLAYER_STATE_NONE:
      CreatePlayer();
      break;
    case PLAYER_STATE_IDLE:
      if (isPreparing_) {
        LOG_DEBUG("player is preparing, unprepare the player");
        result = player_unprepare(player_);
        HandleResult("player_unprepare", result);
        isPreparing_ = false;
      }
      break;
    case PLAYER_STATE_READY:
    case PLAYER_STATE_PLAYING:
    case PLAYER_STATE_PAUSED:
      LOG_DEBUG("unprepare audio player");
      result = player_unprepare(player_);
      HandleResult("player_unprepare", result);
      break;
  }
}

player_state_e AudioPlayer::GetPlayerState() {
  player_state_e state;
  if (player_ == nullptr) {
    state = PLAYER_STATE_NONE;
  } else {
    int result = player_get_state(player_, &state);
    if (result != PLAYER_ERROR_NONE) {
      state = PLAYER_STATE_NONE;
    }
  }
  return state;
}

void AudioPlayer::HandleResult(const std::string &funcName, int result) {
  if (result != PLAYER_ERROR_NONE) {
    std::string error = PlayerErrorToString(result);
    LOG_ERROR("%s failed : %s", funcName.c_str(), error.c_str());
    throw AudioPlayerError(error, funcName + " failed");
  }
}

void AudioPlayer::OnPrepared(void *data) {
  LOG_INFO("Audio player is prepared");
  AudioPlayer *player = (AudioPlayer *)data;
  player->isPreparing_ = false;

  int duration = 0;
  int result = player_get_duration(player->player_, &duration);
  if (result == PLAYER_ERROR_NONE) {
    player->preparedListener_(player->playerId_, duration);
  }

  player_set_playback_rate(player->player_, player->playbackRate_);

  if (player->shouldPlay_) {
    LOG_DEBUG("start to play audio");
    result = player_start(player->player_);
    if (result == PLAYER_ERROR_NONE) {
      player->startPlayingListener_(player->playerId_);
    }
    player->shouldPlay_ = false;
  }

  if (player->shouldSeekTo_ > 0) {
    LOG_DEBUG("set play position %d", player->shouldSeekTo_);
    player->isSeeking_ = true;
    result = player_set_play_position(player->player_, player->shouldSeekTo_,
                                      true, OnSeekCompleted, data);
    if (result != PLAYER_ERROR_NONE) {
      LOG_ERROR("failed to set play position");
      player->isSeeking_ = false;
    }
    player->shouldSeekTo_ = -1;
  }
}

void AudioPlayer::OnSeekCompleted(void *data) {
  LOG_DEBUG("completed to set position");
  AudioPlayer *player = (AudioPlayer *)data;
  player->seekCompletedListener_(player->playerId_);
  player->isSeeking_ = false;
}

void AudioPlayer::OnPlayCompleted(void *data) {
  LOG_DEBUG("completed to play audio");
  AudioPlayer *player = (AudioPlayer *)data;
  if (player->releaseMode_ != LOOP) {
    player->Stop();
  }
  player->playCompletedListener_(player->playerId_);
}

void AudioPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  LOG_ERROR("interruption occurred: %d", code);
  AudioPlayer *player = (AudioPlayer *)data;
  player->errorListener_(player->playerId_, "player - Interrupted");
}

void AudioPlayer::OnErrorOccurred(int code, void *data) {
  std::string error = PlayerErrorToString(code);
  LOG_ERROR("error occurred: %s", error.c_str());
  AudioPlayer *player = (AudioPlayer *)data;
  player->errorListener_(player->playerId_, "error occurred: " + error);
}
