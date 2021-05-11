#include "audio_player.h"

#include "audio_player_error.h"
#include "log.h"

AudioPlayer::AudioPlayer(const std::string &player_id, bool low_latency,
                         PreparedListener prepared_listener,
                         StartPlayingListener start_playing_listener,
                         SeekCompletedListener seek_completed_listener,
                         PlayCompletedListener play_completed_listener,
                         ErrorListener error_listener) {
  LOG_INFO("AudioPlayer %s is constructing...", player_id.c_str());
  player_id_ = player_id;
  low_latency_ = low_latency;
  prepared_listener_ = prepared_listener;
  start_playing_listener_ = start_playing_listener;
  seek_completed_listener_ = seek_completed_listener;
  play_completed_listener_ = play_completed_listener;
  error_listener_ = error_listener;

  player_ = nullptr;
  volume_ = 1.0;
  playback_rate_ = 1.0;
  release_mode_ = RELEASE;
  should_seek_to_ = -1;
}

AudioPlayer::~AudioPlayer() {
  LOG_INFO("AudioPlayer %s is destructing...", player_id_.c_str());
  Release();
}

void AudioPlayer::Play() {
  LOG_INFO("AudioPlayer %s will play audio...", player_id_.c_str());
  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_IDLE && preparing_) {
    LOG_DEBUG("player is preparing, play will be called in prepared callback");
    should_play_ = true;
    return;
  }

  if (state == PLAYER_STATE_NONE) {
    CreatePlayer();
  }

  int result;
  switch (state) {
    case PLAYER_STATE_NONE:
    case PLAYER_STATE_IDLE:
      if (audio_data_.size() > 0) {
        LOG_DEBUG("set audio buffer, buffer size : %d", audio_data_.size());
        result = player_set_memory_buffer(player_, (void *)audio_data_.data(),
                                          audio_data_.size());
        HandleResult("player_set_memory_buffer", result);
      } else {
        LOG_DEBUG("set uri (%s)", url_.c_str());
        result = player_set_uri(player_, url_.c_str());
        HandleResult("player_set_uri", result);
      }
      should_play_ = true;
      PreparePlayer();
      break;
    case PLAYER_STATE_READY:
    case PLAYER_STATE_PAUSED:
      result = player_start(player_);
      HandleResult("player_start", result);
      should_play_ = false;
      start_playing_listener_(player_id_);
      break;
    default:
      LOG_DEBUG("player is already playing audio");
      break;
  }
}

void AudioPlayer::Pause() {
  LOG_INFO("AudioPlayer %s is pausing...", player_id_.c_str());
  if (GetPlayerState() == PLAYER_STATE_PLAYING) {
    int result = player_pause(player_);
    HandleResult("player_pause", result);
  }
  should_play_ = false;
}

void AudioPlayer::Stop() {
  LOG_INFO("AudioPlayer %s is stopping...", player_id_.c_str());
  if (release_mode_ == RELEASE) {
    Release();
  } else {
    player_state_e state = GetPlayerState();
    if (state == PLAYER_STATE_PLAYING || state == PLAYER_STATE_PAUSED) {
      int result = player_stop(player_);
      HandleResult("player_stop", result);
    }
  }
  should_play_ = false;
  seeking_ = false;
}

void AudioPlayer::Release() {
  LOG_INFO("AudioPlayer %s is releasing...", player_id_.c_str());
  if (player_ != nullptr) {
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_destroy(player_);
    player_ = nullptr;
  }
}

void AudioPlayer::Seek(int position) {
  LOG_INFO("AudioPlayer %s is seeking...", player_id_.c_str());
  if (seeking_) {
    LOG_DEBUG("player is already seeking, can't seek again");
    return;
  }

  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
      state == PLAYER_STATE_PAUSED) {
    LOG_DEBUG("set play position %d", position);
    seeking_ = true;
    int result = player_set_play_position(player_, position, true,
                                          OnSeekCompleted, (void *)this);
    if (result != PLAYER_ERROR_NONE) {
      seeking_ = false;
      std::string error(get_error_message(result));
      LOG_ERROR("player_set_play_position failed : %s", error.c_str());
      throw AudioPlayerError(error, "player_set_play_position failed");
    }
  } else {
    LOG_DEBUG("player is unprepared, do seek in prepared callback");
    should_seek_to_ = position;
  }
}

void AudioPlayer::SetUrl(const std::string &url) {
  LOG_INFO("AudioPlayer %s is setting url...", player_id_.c_str());
  if (url != url_) {
    url_ = url;
    ResetPlayer();

    LOG_DEBUG("set new uri (%s)", url.c_str());
    int result = player_set_uri(player_, url.c_str());
    HandleResult("player_set_uri", result);

    PreparePlayer();
  }
  audio_data_.clear();
}

void AudioPlayer::SetDataSource(std::vector<uint8_t> &data) {
  LOG_INFO("AudioPlayer %s is setting buffer...", player_id_.c_str());
  if (data != audio_data_) {
    audio_data_.swap(data);
    ResetPlayer();

    LOG_DEBUG("set audio buffer, buffer size : %d", audio_data_.size());
    int result = player_set_memory_buffer(player_, (void *)audio_data_.data(),
                                          audio_data_.size());
    HandleResult("player_set_memory_buffer", result);

    PreparePlayer();
  }
}

void AudioPlayer::SetVolume(double volume) {
  LOG_INFO("AudioPlayer %s is setting volume %f...", player_id_.c_str(),
           volume);
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
  LOG_INFO("AudioPlayer %s is setting plackback rate %f...", player_id_.c_str(),
           rate);
  if (playback_rate_ != rate) {
    playback_rate_ = rate;
    player_state_e state = GetPlayerState();
    if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
        state == PLAYER_STATE_PAUSED) {
      LOG_DEBUG("set plackback rate : %f", rate);
      int result = player_set_playback_rate(player_, rate);
      HandleResult("player_set_playback_rate", result);
    }
  }
}

void AudioPlayer::SetReleaseMode(ReleaseMode mode) {
  LOG_INFO("AudioPlayer %s is setting ReleaseMode %d...", player_id_.c_str(),
           mode);
  if (release_mode_ != mode) {
    release_mode_ = mode;
    if (GetPlayerState() != PLAYER_STATE_NONE) {
      LOG_DEBUG("set looping : %d", release_mode_ == LOOP);
      int result = player_set_looping(player_, (release_mode_ == LOOP));
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

std::string AudioPlayer::GetPlayerId() const { return player_id_; }

bool AudioPlayer::IsPlaying() {
  return (GetPlayerState() == PLAYER_STATE_PLAYING);
}

void AudioPlayer::CreatePlayer() {
  LOG_INFO("create audio player...");
  should_play_ = false;
  preparing_ = false;

  int result = player_create(&player_);
  HandleResult("player_create", result);

  if (low_latency_) {
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

  LOG_DEBUG("set looping %d", (release_mode_ == LOOP));
  result = player_set_looping(player_, (release_mode_ == LOOP));
  HandleResult("player_set_looping", result);

  LOG_DEBUG("prepare audio player asynchronously");
  result = player_prepare_async(player_, OnPrepared, (void *)this);
  HandleResult("player_prepare_async", result);
  preparing_ = true;
  seeking_ = false;
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
      if (preparing_) {
        LOG_DEBUG("player is preparing, unprepare the player");
        result = player_unprepare(player_);
        HandleResult("player_unprepare", result);
        preparing_ = false;
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

void AudioPlayer::HandleResult(const std::string &func_name, int result) {
  if (result != PLAYER_ERROR_NONE) {
    std::string error(get_error_message(result));
    LOG_ERROR("%s failed : %s", func_name.c_str(), error.c_str());
    throw AudioPlayerError(error, func_name + " failed");
  }
}

void AudioPlayer::OnPrepared(void *data) {
  LOG_INFO("Audio player is prepared");
  AudioPlayer *player = (AudioPlayer *)data;
  player->preparing_ = false;

  int duration = 0;
  int result = player_get_duration(player->player_, &duration);
  if (result == PLAYER_ERROR_NONE) {
    player->prepared_listener_(player->player_id_, duration);
  }

  player_set_playback_rate(player->player_, player->playback_rate_);

  if (player->should_play_) {
    LOG_DEBUG("start to play audio");
    result = player_start(player->player_);
    if (result == PLAYER_ERROR_NONE) {
      player->start_playing_listener_(player->player_id_);
    }
    player->should_play_ = false;
  }

  if (player->should_seek_to_ > 0) {
    LOG_DEBUG("set play position %d", player->should_seek_to_);
    player->seeking_ = true;
    result = player_set_play_position(player->player_, player->should_seek_to_,
                                      true, OnSeekCompleted, data);
    if (result != PLAYER_ERROR_NONE) {
      LOG_ERROR("failed to set play position");
      player->seeking_ = false;
    }
    player->should_seek_to_ = -1;
  }
}

void AudioPlayer::OnSeekCompleted(void *data) {
  LOG_DEBUG("completed to set position");
  AudioPlayer *player = (AudioPlayer *)data;
  player->seek_completed_listener_(player->player_id_);
  player->seeking_ = false;
}

void AudioPlayer::OnPlayCompleted(void *data) {
  LOG_DEBUG("completed to play audio");
  AudioPlayer *player = (AudioPlayer *)data;
  if (player->release_mode_ != LOOP) {
    player->Stop();
  }
  player->play_completed_listener_(player->player_id_);
}

void AudioPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  LOG_ERROR("interruption occurred: %d", code);
  AudioPlayer *player = (AudioPlayer *)data;
  player->error_listener_(player->player_id_, "player - Interrupted");
}

void AudioPlayer::OnErrorOccurred(int code, void *data) {
  std::string error(get_error_message(code));
  LOG_ERROR("error occurred: %s", error.c_str());
  AudioPlayer *player = (AudioPlayer *)data;
  player->error_listener_(player->player_id_, "error occurred: " + error);
}
