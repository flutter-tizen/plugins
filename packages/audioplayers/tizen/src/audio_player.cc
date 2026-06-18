// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "audio_player.h"

#include <string>
#include <utility>

#include "audio_player_error.h"
#include "log.h"

namespace {

struct IdleData {
  AudioPlayer *player;
  std::shared_ptr<bool> is_alive;
};

}  // namespace

AudioPlayer::AudioPlayer(const std::string &player_id,
                         PreparedListener prepared_listener,
                         DurationListener duration_listener,
                         SeekCompletedListener seek_completed_listener,
                         PlayCompletedListener play_completed_listener,
                         LogListener log_listener)
    : player_id_(player_id),
      prepared_listener_(prepared_listener),
      duration_listener_(duration_listener),
      seek_completed_listener_(seek_completed_listener),
      play_completed_listener_(play_completed_listener),
      log_listener_(log_listener) {
  CreatePlayer();
}

AudioPlayer::~AudioPlayer() {
  if (player_) {
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_destroy(player_);
    player_ = nullptr;
  }
  if (timer_id_ != 0) {
    g_source_remove(timer_id_);
    timer_id_ = 0;
  }
  *is_alive_ = false;
}

void AudioPlayer::Play() {
  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_IDLE && preparing_) {
    // Player is preparing, play will be called in prepared callback.
    should_play_ = true;
    return;
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
        should_play_ = true;
        PreparePlayer();
      } else if (url_.size() > 0) {
        int ret = player_set_uri(player_, url_.c_str());
        if (ret != PLAYER_ERROR_NONE) {
          throw AudioPlayerError("player_set_uri failed",
                                 get_error_message(ret));
        }
        should_play_ = true;
        PreparePlayer();
      }
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
    // Reset the play position to 0 to match other platforms. This is
    // best-effort: on some devices (e.g. TV with network sources)
    // player_set_play_position right after stop can fail with an invalid
    // state, which must not crash the app.
    try {
      Seek(0);
    } catch (const AudioPlayerError &error) {
      OnLog("Failed to reset position on stop: " + error.message());
    }
  }

  should_play_ = false;
  seeking_ = false;

  if (release_mode_ == ReleaseMode::kRelease) {
    ReleaseMediaSource();
  }
}

void AudioPlayer::ReleaseMediaSource() {
  url_.clear();
  audio_data_.clear();
  ResetPlayer();
}

void AudioPlayer::Seek(int32_t position) {
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

void AudioPlayer::OnLog(const std::string &message) {
  if (log_listener_) {
    log_listener_(player_id_, message);
  }
}

void AudioPlayer::SetUrl(const std::string &url) {
  url_ = url;
  ResetPlayer();

  int ret = player_set_uri(player_, url.c_str());
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_set_uri failed", get_error_message(ret));
  }

  PreparePlayer();
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
  if (volume > 1) {
    volume = 1;
  } else if (volume < 0) {
    volume = 0;
  }
  volume_ = volume;

  if (GetPlayerState() != PLAYER_STATE_NONE) {
    int ret = player_set_volume(player_, volume_, volume_);
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_set_volume failed",
                             get_error_message(ret));
    }
  }
}

void AudioPlayer::SetPlaybackRate(double playback_rate) {
  // TODO(seungsoo47): The player_set_playback_rate() API has a limitation of
  // 0.5-2x on TV and is not supported on RPI.
  playback_rate_ = playback_rate;
  player_state_e state = GetPlayerState();
  if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
      state == PLAYER_STATE_PAUSED) {
    int ret = player_set_playback_rate(player_, playback_rate);
    if (ret != PLAYER_ERROR_NONE) {
      throw AudioPlayerError("player_set_playback_rate failed",
                             get_error_message(ret));
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

void AudioPlayer::SetLatencyMode(bool low_latency) {
  int ret = player_set_audio_latency_mode(
      player_, low_latency ? AUDIO_LATENCY_MODE_LOW : AUDIO_LATENCY_MODE_MID);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_set_audio_latency_mode failed",
                           get_error_message(ret));
  }
}

int AudioPlayer::GetDuration() {
  int32_t duration;
  int ret = player_get_duration(player_, &duration);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_get_duration failed",
                           get_error_message(ret));
  }
  return duration;
}

int AudioPlayer::GetCurrentPosition() {
  int32_t position;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    throw AudioPlayerError("player_get_play_position failed",
                           get_error_message(ret));
  }
  return position;
}

bool AudioPlayer::IsSourcePrepared() {
  player_state_e state = GetPlayerState();
  return state == PLAYER_STATE_READY || state == PLAYER_STATE_PLAYING ||
         state == PLAYER_STATE_PAUSED;
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
  auto *self = reinterpret_cast<AudioPlayer *>(data);
  // On TV devices, callbacks are not executed on the main loop. Therefore
  // we explicitly transfer the callback to the main loop to avoid any race
  // conditions and to allow creating timer objects in StartPositionUpdates.
  g_idle_add_full(
      G_PRIORITY_DEFAULT_IDLE,
      [](gpointer data) -> gboolean {
        auto *idle = static_cast<IdleData *>(data);
        if (!*idle->is_alive) {
          return G_SOURCE_REMOVE;
        }
        auto *player = idle->player;
        player->preparing_ = false;

        try {
          player->duration_listener_(player->player_id_, player->GetDuration());
          player->prepared_listener_(player->player_id_, true);
        } catch (const AudioPlayerError &error) {
          player->log_listener_(player->player_id_, error.code());
          return G_SOURCE_REMOVE;
        }
        player_set_playback_rate(player->player_, player->playback_rate_);

        if (player->should_play_) {
          int ret = player_start(player->player_);
          if (ret != PLAYER_ERROR_NONE) {
            player->log_listener_(player->player_id_, "player_start failed.");
            return G_SOURCE_REMOVE;
          }
          player->StartPositionUpdates();
          player->should_play_ = false;
        }

        if (player->should_seek_to_ > 0) {
          player->seeking_ = true;
          int ret =
              player_set_play_position(player->player_, player->should_seek_to_,
                                       true, OnSeekCompleted, player);
          if (ret != PLAYER_ERROR_NONE) {
            player->seeking_ = false;
            player->log_listener_(player->player_id_,
                                  "player_set_play_position failed.");
            return G_SOURCE_REMOVE;
          }
          player->should_seek_to_ = -1;
        }
        return G_SOURCE_REMOVE;
      },
      new IdleData{self, self->is_alive_},
      [](gpointer data) { delete static_cast<IdleData *>(data); });
}

void AudioPlayer::OnSeekCompleted(void *data) {
  auto *self = reinterpret_cast<AudioPlayer *>(data);
  // On TV devices, callbacks are not executed on the main loop. Therefore
  // we explicitly transfer the callback to the main loop to avoid any race
  // conditions.
  g_idle_add_full(
      G_PRIORITY_DEFAULT_IDLE,
      [](gpointer data) -> gboolean {
        auto *idle = static_cast<IdleData *>(data);
        if (!*idle->is_alive) {
          return G_SOURCE_REMOVE;
        }
        auto *player = idle->player;
        player->seek_completed_listener_(player->player_id_);
        player->seeking_ = false;
        return G_SOURCE_REMOVE;
      },
      new IdleData{self, self->is_alive_},
      [](gpointer data) { delete static_cast<IdleData *>(data); });
}

void AudioPlayer::OnPlayCompleted(void *data) {
  auto *self = reinterpret_cast<AudioPlayer *>(data);
  // On TV devices, callbacks are not executed on the main loop. Therefore
  // we explicitly transfer the callback to the main loop to avoid any race
  // conditions.
  g_idle_add_full(
      G_PRIORITY_DEFAULT_IDLE,
      [](gpointer data) -> gboolean {
        auto *idle = static_cast<IdleData *>(data);
        if (!*idle->is_alive) {
          return G_SOURCE_REMOVE;
        }
        auto *player = idle->player;
        try {
          player->Seek(0);
          player->Stop();
          player->play_completed_listener_(player->player_id_);
        } catch (const AudioPlayerError &error) {
          player->log_listener_(player->player_id_, error.code());
        }
        return G_SOURCE_REMOVE;
      },
      new IdleData{self, self->is_alive_},
      [](gpointer data) { delete static_cast<IdleData *>(data); });
}

void AudioPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  auto *self = reinterpret_cast<AudioPlayer *>(data);
  // On TV devices, callbacks are not executed on the main loop. Transfer to
  // the main loop so the log event is sent on the platform thread.
  g_idle_add_full(
      G_PRIORITY_DEFAULT_IDLE,
      [](gpointer data) -> gboolean {
        auto *idle = static_cast<IdleData *>(data);
        if (!*idle->is_alive) {
          return G_SOURCE_REMOVE;
        }
        idle->player->log_listener_(idle->player->player_id_,
                                    "Player interrupted.");
        return G_SOURCE_REMOVE;
      },
      new IdleData{self, self->is_alive_},
      [](gpointer data) { delete static_cast<IdleData *>(data); });
}

void AudioPlayer::OnError(int code, void *data) {
  auto *self = reinterpret_cast<AudioPlayer *>(data);
  // On TV devices, callbacks are not executed on the main loop. Transfer to
  // the main loop so the log event is sent on the platform thread. The error
  // message is resolved here and carried via a heap-allocated context.
  struct ErrorData {
    AudioPlayer *player;
    std::shared_ptr<bool> is_alive;
    std::string message;
  };
  g_idle_add_full(
      G_PRIORITY_DEFAULT_IDLE,
      [](gpointer data) -> gboolean {
        auto *error_data = static_cast<ErrorData *>(data);
        if (!*error_data->is_alive) {
          return G_SOURCE_REMOVE;
        }
        error_data->player->log_listener_(error_data->player->player_id_,
                                          error_data->message);
        return G_SOURCE_REMOVE;
      },
      new ErrorData{self, self->is_alive_, get_error_message(code)},
      [](gpointer data) { delete static_cast<ErrorData *>(data); });
}

void AudioPlayer::StartPositionUpdates() {
  if (timer_id_ == 0) {
    // The audioplayers app facing package expects position
    // update events to fire roughly every 200 milliseconds.
    const guint kTimeInterval = 200;
    timer_id_ = g_timeout_add(kTimeInterval, OnPositionUpdate, this);
    if (timer_id_ == 0) {
      log_listener_(player_id_, "Failed to add a position update timer.");
    }
  }
}

gboolean AudioPlayer::OnPositionUpdate(gpointer data) {
  auto *player = reinterpret_cast<AudioPlayer *>(data);
  try {
    if (player->IsPlaying()) {
      player->duration_listener_(player->player_id_, player->GetDuration());
      return G_SOURCE_CONTINUE;
    }
  } catch (const AudioPlayerError &error) {
    player->log_listener_(player->player_id_, "Failed to update position.");
  }
  player->timer_id_ = 0;
  return G_SOURCE_REMOVE;
}
