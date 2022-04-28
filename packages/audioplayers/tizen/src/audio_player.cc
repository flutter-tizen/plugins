#include "audio_player.h"

#include "audio_player_error.h"
#include "log.h"

static void HandleResult(const std::string &func_name, int result) {
  if (result != PLAYER_ERROR_NONE) {
    std::string error(get_error_message(result));
    LOG_ERROR("%s failed : %s", func_name.c_str(), error.c_str());
    throw AudioPlayerError(error, func_name + " failed");
  }
}

AudioPlayer::AudioPlayer(const std::string &player_id, bool low_latency,
                         PreparedListener prepared_listener,
                         StartPlayingListener start_playing_listener,
                         SeekCompletedListener seek_completed_listener,
                         PlayCompletedListener play_completed_listener,
                         ErrorListener error_listener)
    : player_id_(player_id),
      low_latency_(low_latency),
      prepared_listener_(prepared_listener),
      start_playing_listener_(start_playing_listener),
      seek_completed_listener_(seek_completed_listener),
      play_completed_listener_(play_completed_listener),
      error_listener_(error_listener) {}

AudioPlayer::~AudioPlayer() { Release(); }

void AudioPlayer::Play() {
  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_IDLE && preparing_) {
    // Player is preparing, play will be called in prepared callback.
    should_play_ = true;
    return;
  }

  if (state == PLAYER_STATE_NONE) {
    CreatePlayer();
  }

  int ret;
  switch (state) {
    case PLAYER_STATE_NONE:
    case PLAYER_STATE_IDLE:
      if (audio_data_.size() > 0) {
        ret = player_set_memory_buffer(player_, (void *)audio_data_.data(),
                                       audio_data_.size());
        HandleResult("player_set_memory_buffer", ret);
      } else {
        ret = player_set_uri(player_, url_.c_str());
        HandleResult("player_set_uri", ret);
      }
      should_play_ = true;
      PreparePlayer();
      break;
    case PLAYER_STATE_READY:
    case PLAYER_STATE_PAUSED:
      ret = player_start(player_);
      HandleResult("player_start", ret);
      should_play_ = false;
      EmitPositionUpdates();
      break;
    default:
      // Player is already playing audio.
      break;
  }
}

void AudioPlayer::Pause() {
  if (GetPlayerState() == PLAYER_STATE_PLAYING) {
    int ret = player_pause(player_);
    HandleResult("player_pause", ret);
  }
  should_play_ = false;
}

void AudioPlayer::Stop() {
  if (release_mode_ == ReleaseMode::kRelease) {
    Release();
  } else {
    player_state_e state = GetPlayerState();
    if (state == PLAYER_STATE_PLAYING || state == PLAYER_STATE_PAUSED) {
      int ret = player_stop(player_);
      HandleResult("player_stop", ret);
    }
  }
  should_play_ = false;
  seeking_ = false;
}

void AudioPlayer::Release() {
  if (player_) {
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_destroy(player_);
    player_ = nullptr;
  }
  if (timer_) {
    ecore_timer_del(timer_);
    timer_ = nullptr;
  }
}

void AudioPlayer::Seek(int position) {
  if (seeking_) {
    return;
  }

  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
      state == PLAYER_STATE_PAUSED) {
    seeking_ = true;
    int ret = player_set_play_position(player_, position, true, OnSeekCompleted,
                                       (void *)this);
    if (ret != PLAYER_ERROR_NONE) {
      seeking_ = false;
      std::string error(get_error_message(ret));
      LOG_ERROR("player_set_play_position failed : %s", error.c_str());
      throw AudioPlayerError(error, "player_set_play_position failed");
    }
  } else {
    // Player is unprepared, do seek in prepared callback.
    should_seek_to_ = position;
  }
}

void AudioPlayer::SetUrl(const std::string &url) {
  if (url != url_) {
    url_ = url;
    ResetPlayer();

    int ret = player_set_uri(player_, url.c_str());
    HandleResult("player_set_uri", ret);

    PreparePlayer();
  }
  audio_data_.clear();
}

void AudioPlayer::SetDataSource(std::vector<uint8_t> &data) {
  if (data != audio_data_) {
    audio_data_.swap(data);
    ResetPlayer();

    int ret = player_set_memory_buffer(player_, (void *)audio_data_.data(),
                                       audio_data_.size());
    HandleResult("player_set_memory_buffer", ret);

    PreparePlayer();
  }
}

void AudioPlayer::SetVolume(double volume) {
  if (volume_ != volume) {
    volume_ = volume;
    if (GetPlayerState() != PLAYER_STATE_NONE) {
      int ret = player_set_volume(player_, volume_, volume_);
      HandleResult("player_set_volume", ret);
    }
  }
}

void AudioPlayer::SetPlaybackRate(double rate) {
  if (playback_rate_ != rate) {
    playback_rate_ = rate;
    player_state_e state = GetPlayerState();
    if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
        state == PLAYER_STATE_PAUSED) {
      int ret = player_set_playback_rate(player_, rate);
      HandleResult("player_set_playback_rate", ret);
    }
  }
}

void AudioPlayer::SetReleaseMode(ReleaseMode mode) {
  if (release_mode_ != mode) {
    release_mode_ = mode;
    if (GetPlayerState() != PLAYER_STATE_NONE) {
      int ret =
          player_set_looping(player_, (release_mode_ == ReleaseMode::kLoop));
      HandleResult("player_set_looping", ret);
    }
  }
}

int AudioPlayer::GetDuration() {
  int duration;
  int result = player_get_duration(player_, &duration);
  HandleResult("player_get_duration", result);
  return duration;
}

int AudioPlayer::GetCurrentPosition() {
  int position;
  int result = player_get_play_position(player_, &position);
  HandleResult("player_get_play_position", result);
  return position;
}

bool AudioPlayer::IsPlaying() {
  return (GetPlayerState() == PLAYER_STATE_PLAYING);
}

void AudioPlayer::CreatePlayer() {
  should_play_ = false;
  preparing_ = false;

  int ret = player_create(&player_);
  HandleResult("player_create", ret);

  if (low_latency_) {
    ret = player_set_audio_latency_mode(player_, AUDIO_LATENCY_MODE_LOW);
    HandleResult("player_set_audio_latency_mode", ret);
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, (void *)this);
  HandleResult("player_set_completed_cb", ret);

  ret = player_set_interrupted_cb(player_, OnInterrupted, (void *)this);
  HandleResult("player_set_interrupted_cb", ret);

  ret = player_set_error_cb(player_, OnErrorOccurred, (void *)this);
  HandleResult("player_set_error_cb", ret);
}

void AudioPlayer::PreparePlayer() {
  int ret = player_set_volume(player_, volume_, volume_);
  HandleResult("player_set_volume", ret);

  ret = player_set_looping(player_, (release_mode_ == ReleaseMode::kLoop));
  HandleResult("player_set_looping", ret);

  ret = player_prepare_async(player_, OnPrepared, (void *)this);
  HandleResult("player_prepare_async", ret);
  preparing_ = true;
  seeking_ = false;
}

void AudioPlayer::EmitPositionUpdates() {
  ecore_main_loop_thread_safe_call_async(StartPositionUpdates, (void *)this);
}

void AudioPlayer::ResetPlayer() {
  int ret;
  player_state_e state = GetPlayerState();
  switch (state) {
    case PLAYER_STATE_NONE:
      CreatePlayer();
      break;
    case PLAYER_STATE_IDLE:
      if (preparing_) {
        // Cancel preparing if it's already preparing.
        ret = player_unprepare(player_);
        HandleResult("player_unprepare", ret);
        preparing_ = false;
      }
      break;
    case PLAYER_STATE_READY:
    case PLAYER_STATE_PLAYING:
    case PLAYER_STATE_PAUSED:
      ret = player_unprepare(player_);
      HandleResult("player_unprepare", ret);
      break;
  }
}

player_state_e AudioPlayer::GetPlayerState() {
  player_state_e state = PLAYER_STATE_NONE;
  if (player_) {
    int ret = player_get_state(player_, &state);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("Getting player(id: %s) state failed: %s\n", player_id_.c_str(),
                get_error_message(ret));
    }
  }
  return state;
}

void AudioPlayer::OnPrepared(void *data) {
  AudioPlayer *player = (AudioPlayer *)data;
  player->preparing_ = false;

  int duration = 0;
  int ret = player_get_duration(player->player_, &duration);
  if (ret == PLAYER_ERROR_NONE) {
    player->prepared_listener_(player->player_id_, duration);
  }

  player_set_playback_rate(player->player_, player->playback_rate_);

  if (player->should_play_) {
    ret = player_start(player->player_);
    if (ret == PLAYER_ERROR_NONE) {
      player->EmitPositionUpdates();
    }
    player->should_play_ = false;
  }

  if (player->should_seek_to_ > 0) {
    player->seeking_ = true;
    ret = player_set_play_position(player->player_, player->should_seek_to_,
                                   true, OnSeekCompleted, data);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("failed to set play position");
      player->seeking_ = false;
    }
    player->should_seek_to_ = -1;
  }
}

void AudioPlayer::OnSeekCompleted(void *data) {
  AudioPlayer *player = (AudioPlayer *)data;
  player->seek_completed_listener_(player->player_id_);
  player->seeking_ = false;
}

void AudioPlayer::OnPlayCompleted(void *data) {
  AudioPlayer *player = (AudioPlayer *)data;
  if (player->release_mode_ != ReleaseMode::kLoop) {
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

void AudioPlayer::StartPositionUpdates(void *data) {
  AudioPlayer *player = (AudioPlayer *)data;
  if (!player->timer_) {
    const double kTimeInterval = 0.2;
    player->timer_ = ecore_timer_add(kTimeInterval, OnPositionUpdate, data);
    if (player->timer_ == nullptr) {
      LOG_ERROR("failed to add timer for UpdatePosition");
    }
  }
}

Eina_Bool AudioPlayer::OnPositionUpdate(void *data) {
  AudioPlayer *player = (AudioPlayer *)data;
  std::string player_id = player->GetPlayerId();
  try {
    if (player->IsPlaying()) {
      int duration = player->GetDuration();
      int position = player->GetCurrentPosition();
      player->start_playing_listener_(player_id, duration, position);
      return ECORE_CALLBACK_RENEW;
    }
  } catch (...) {
    LOG_ERROR("failed to update position for player %s", player_id.c_str());
  }
  player->timer_ = nullptr;
  return ECORE_CALLBACK_CANCEL;
}