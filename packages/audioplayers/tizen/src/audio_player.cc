#include "audio_player.h"

#include "audio_player_error.h"

AudioPlayer::AudioPlayer(const std::string &player_id, bool low_latency,
                         PreparedListener prepared_listener,
                         UpdatePositionListener update_position_listener,
                         SeekCompletedListener seek_completed_listener,
                         PlayCompletedListener play_completed_listener,
                         ErrorListener error_listener)
    : player_id_(player_id),
      low_latency_(low_latency),
      prepared_listener_(prepared_listener),
      update_position_listener_(update_position_listener),
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

  switch (state) {
    case PLAYER_STATE_NONE:
    case PLAYER_STATE_IDLE: {
      if (audio_data_.size() > 0) {
        int ret = player_set_memory_buffer(player_, audio_data_.data(),
                                           audio_data_.size());
        if (ret != PLAYER_ERROR_NONE) {
          throw AudioPlayerError("player_set_memory_buffer failed",
                                 get_error_message(ret));
        }
      } else {
        int ret = player_set_uri(player_, url_.c_str());
        if (ret != PLAYER_ERROR_NONE) {
          throw AudioPlayerError("player_set_uri failed",
                                 get_error_message(ret));
        }
      }
      should_play_ = true;
      PreparePlayer();
      break;
    }
    case PLAYER_STATE_READY:
    case PLAYER_STATE_PAUSED: {
      int ret = player_start(player_);
      if (ret != PLAYER_ERROR_NONE) {
        throw AudioPlayerError("player_start failed", get_error_message(ret));
      }
      should_play_ = false;
      StartPositionUpdates();
      break;
    }
    default:
      // Player is already playing audio.
      break;
  }
}

void AudioPlayer::Pause() {
  if (GetPlayerState() == PLAYER_STATE_PLAYING) {
    int ret = player_pause(player_);
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_pause failed", get_error_message(ret));
    }
  }

  should_play_ = false;
}

void AudioPlayer::Stop() {
  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_PLAYING || state == PLAYER_STATE_PAUSED) {
    int ret = player_stop(player_);
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_stop failed", get_error_message(ret));
    }
  }

  should_play_ = false;
  seeking_ = false;

  if (release_mode_ == ReleaseMode::kRelease) {
    Release();
  }
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
                                       this);
    if (ret != PLAYER_ERROR_NONE) {
      seeking_ = false;
      throw AudioPlayerError("player_set_play_position failed",
                             get_error_message(ret));
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
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_set_uri failed", get_error_message(ret));
    }

    PreparePlayer();
  }
  audio_data_.clear();
}

void AudioPlayer::SetDataSource(std::vector<uint8_t> &data) {
  if (data != audio_data_) {
    audio_data_.swap(data);
    ResetPlayer();

    int ret = player_set_memory_buffer(player_, audio_data_.data(),
                                       audio_data_.size());
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_set_memory_buffer failed",
                             get_error_message(ret));
    }

    PreparePlayer();
  }
}

void AudioPlayer::SetVolume(double volume) {
  if (volume_ != volume) {
    volume_ = volume;
    if (GetPlayerState() != PLAYER_STATE_NONE) {
      int ret = player_set_volume(player_, volume_, volume_);
      if (ret != PLAYER_ERROR_NONE) {
        throw AudioPlayerError("player_set_volume failed",
                               get_error_message(ret));
      }
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
      if (ret != PLAYER_ERROR_NONE) {
        throw AudioPlayerError("player_set_playback_rate failed",
                               get_error_message(ret));
      }
    }
  }
}

void AudioPlayer::SetReleaseMode(ReleaseMode mode) {
  if (release_mode_ != mode) {
    release_mode_ = mode;
    if (GetPlayerState() != PLAYER_STATE_NONE) {
      int ret =
          player_set_looping(player_, (release_mode_ == ReleaseMode::kLoop));
      if (ret != PLAYER_ERROR_NONE) {
        throw AudioPlayerError("player_set_looping failed",
                               get_error_message(ret));
      }
    }
  }
}

int AudioPlayer::GetDuration() {
  int duration;
  int ret = player_get_duration(player_, &duration);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_get_duration failed",
                           get_error_message(ret));
  }
  return duration;
}

int AudioPlayer::GetCurrentPosition() {
  int position;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_get_play_position failed",
                           get_error_message(ret));
  }
  return position;
}

bool AudioPlayer::IsPlaying() {
  return (GetPlayerState() == PLAYER_STATE_PLAYING);
}

void AudioPlayer::CreatePlayer() {
  should_play_ = false;
  preparing_ = false;

  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_create failed", get_error_message(ret));
  }

  if (low_latency_) {
    ret = player_set_audio_latency_mode(player_, AUDIO_LATENCY_MODE_LOW);
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_set_audio_latency_mode failed",
                             get_error_message(ret));
    }
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_set_completed_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_interrupted_cb(player_, OnInterrupted, this);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_set_interrupted_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_error_cb(player_, OnError, this);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_set_error_cb failed",
                           get_error_message(ret));
  }
}

void AudioPlayer::PreparePlayer() {
  int ret = player_set_volume(player_, volume_, volume_);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_set_volume failed", get_error_message(ret));
  }

  ret = player_set_looping(player_, (release_mode_ == ReleaseMode::kLoop));
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_set_looping failed", get_error_message(ret));
  }

  ret = player_prepare_async(player_, OnPrepared, this);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_prepare_async failed",
                           get_error_message(ret));
  }

  preparing_ = true;
  seeking_ = false;
}

void AudioPlayer::ResetPlayer() {
  player_state_e state = GetPlayerState();
  switch (state) {
    case PLAYER_STATE_NONE:
      CreatePlayer();
      break;
    case PLAYER_STATE_IDLE:
      if (preparing_) {
        // Cancel preparing if it's already preparing.
        int ret = player_unprepare(player_);
        if (ret != PLAYER_ERROR_NONE) {
          throw AudioPlayerError("player_unprepare failed",
                                 get_error_message(ret));
        }
        preparing_ = false;
      }
      break;
    case PLAYER_STATE_READY:
    case PLAYER_STATE_PLAYING:
    case PLAYER_STATE_PAUSED:
      int ret = player_unprepare(player_);
      if (ret != PLAYER_ERROR_NONE) {
        throw AudioPlayerError("player_unprepare failed",
                               get_error_message(ret));
      }
      break;
  }
}

player_state_e AudioPlayer::GetPlayerState() {
  player_state_e state = PLAYER_STATE_NONE;
  if (player_) {
    int ret = player_get_state(player_, &state);
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_get_state failed", get_error_message(ret));
    }
  }
  return state;
}

void AudioPlayer::OnPrepared(void *data) {
  auto *player = reinterpret_cast<AudioPlayer *>(data);
  player->preparing_ = false;

  player->prepared_listener_(player->player_id_, player->GetDuration());
  player_set_playback_rate(player->player_, player->playback_rate_);

  if (player->should_play_) {
    int ret = player_start(player->player_);
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_start failed", get_error_message(ret));
    }
    player->StartPositionUpdates();
    player->should_play_ = false;
  }

  if (player->should_seek_to_ > 0) {
    player->seeking_ = true;
    int ret = player_set_play_position(player->player_, player->should_seek_to_,
                                       true, OnSeekCompleted, data);
    if (ret != PLAYER_ERROR_NONE) {
      player->seeking_ = false;
      throw AudioPlayerError("player_set_play_position failed",
                             get_error_message(ret));
    }
    player->should_seek_to_ = -1;
  }
}

void AudioPlayer::OnSeekCompleted(void *data) {
  auto *player = reinterpret_cast<AudioPlayer *>(data);
  player->seek_completed_listener_(player->player_id_);
  player->seeking_ = false;
}

void AudioPlayer::OnPlayCompleted(void *data) {
  auto *player = reinterpret_cast<AudioPlayer *>(data);
  if (player->release_mode_ != ReleaseMode::kLoop) {
    player->Stop();
  }
  player->play_completed_listener_(player->player_id_);
}

void AudioPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  auto *player = reinterpret_cast<AudioPlayer *>(data);
  player->error_listener_(player->player_id_, "Player interrupted.");
}

void AudioPlayer::OnError(int code, void *data) {
  auto *player = reinterpret_cast<AudioPlayer *>(data);
  std::string error(get_error_message(code));
  player->error_listener_(player->player_id_, "Player error: " + error);
}

void AudioPlayer::StartPositionUpdates() {
  if (!timer_) {
    // The audioplayers app facing package expects position
    // update events to fire roughly every 200 milliseconds.
    const double kTimeInterval = 0.2;
    timer_ = ecore_timer_add(kTimeInterval, OnPositionUpdate, this);
    if (!timer_) {
      error_listener_(player_id_, "Failed to add postion update timer.");
    }
  }
}

Eina_Bool AudioPlayer::OnPositionUpdate(void *data) {
  auto *player = reinterpret_cast<AudioPlayer *>(data);
  std::string player_id = player->GetPlayerId();
  try {
    if (player->IsPlaying()) {
      int duration = player->GetDuration();
      int position = player->GetCurrentPosition();
      player->update_position_listener_(player_id, duration, position);
      return ECORE_CALLBACK_RENEW;
    }
  } catch (const AudioPlayerError &error) {
    player->error_listener_(player_id, "Failed to update position.");
  }
  player->timer_ = nullptr;
  return ECORE_CALLBACK_CANCEL;
}
