// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_player_proxy.h"

#include <dlfcn.h>

#include "log.h"

typedef int (*FuncPlayerSetEcoreWlDisplay)(player_h player,
                                           player_display_type_e type,
                                           void* ecore_wl_window, int x, int y,
                                           int width, int height);

typedef int (*FuncPlayerSetDrmHandle)(player_h player,
                                      player_drm_type_e drm_type,
                                      int drm_handle);
typedef int (*FuncPlayerSetDrmInitCompleteCB)(
    player_h player, security_init_complete_cb callback, void* user_data);
typedef int (*FuncPlayerSetDrmInitDataCB)(player_h player,
                                          set_drm_init_data_cb callback,
                                          void* user_data);

typedef int (*FuncPlayerGetAdaptiveStreamingInfo)(player_h player,
                                                  void* adaptive_info,
                                                  int adaptive_type);

typedef int (*FuncPlayerGetTrackCountV2)(player_h player,
                                         player_stream_type_e type,
                                         int* pcount);
typedef int (*FuncPlayerGetVideoTrackInfoV2)(
    player_h player, int index, player_video_track_info_v2** track_info);
typedef int (*FuncPlayerGetAudioTrackInfoV2)(
    player_h player, int index, player_audio_track_info_v2** track_info);
typedef int (*FuncPlayerGetSubtitleTrackInfoV2)(
    player_h player, int index, player_subtitle_track_info_v2** track_info);

MediaPlayerProxy::MediaPlayerProxy() {
  media_player_handle_ = dlopen("libcapi-media-player.so.0", RTLD_LAZY);
  if (media_player_handle_ == nullptr) {
    LOG_ERROR("Failed to open media player.");
  }
}

MediaPlayerProxy::~MediaPlayerProxy() {
  if (media_player_handle_) {
    dlclose(media_player_handle_);
    media_player_handle_ = nullptr;
  }
}

int MediaPlayerProxy::player_set_ecore_wl_display(player_h player,
                                                  player_display_type_e type,
                                                  void* ecore_wl_window, int x,
                                                  int y, int width,
                                                  int height) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerSetEcoreWlDisplay player_set_ecore_wl_display =
      reinterpret_cast<FuncPlayerSetEcoreWlDisplay>(
          dlsym(media_player_handle_, "player_set_ecore_wl_display"));
  if (!player_set_ecore_wl_display) {
    LOG_ERROR("Fail to find player_set_ecore_wl_display.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_set_ecore_wl_display(player, type, ecore_wl_window, x, y, width,
                                     height);
}

int MediaPlayerProxy::player_set_drm_handle(player_h player,
                                            player_drm_type_e drm_type,
                                            int drm_handle) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerSetDrmHandle player_set_drm_handle =
      reinterpret_cast<FuncPlayerSetDrmHandle>(
          dlsym(media_player_handle_, "player_set_drm_handle"));
  if (!player_set_drm_handle) {
    LOG_ERROR("Fail to find player_set_ecore_wl_display.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_set_drm_handle(player, drm_type, drm_handle);
}

int MediaPlayerProxy::player_set_drm_init_complete_cb(
    player_h player, security_init_complete_cb callback, void* user_data) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerSetDrmInitCompleteCB player_set_drm_init_complete_cb =
      reinterpret_cast<FuncPlayerSetDrmInitCompleteCB>(
          dlsym(media_player_handle_, "player_set_drm_init_complete_cb"));
  if (!player_set_drm_init_complete_cb) {
    LOG_ERROR("Fail to find player_set_drm_init_complete_cb.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_set_drm_init_complete_cb(player, callback, user_data);
}

int MediaPlayerProxy::player_set_drm_init_data_cb(player_h player,
                                                  set_drm_init_data_cb callback,
                                                  void* user_data) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerSetDrmInitDataCB player_set_drm_init_data_cb =
      reinterpret_cast<FuncPlayerSetDrmInitDataCB>(
          dlsym(media_player_handle_, "player_set_drm_init_data_cb"));
  if (!player_set_drm_init_data_cb) {
    LOG_ERROR("Fail to find player_set_drm_init_data_cb.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_set_drm_init_data_cb(player, callback, user_data);
}

int MediaPlayerProxy::player_get_adaptive_streaming_info(player_h player,
                                                         void* adaptive_info,
                                                         int adaptive_type) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerGetAdaptiveStreamingInfo player_get_adaptive_streaming_info =
      reinterpret_cast<FuncPlayerGetAdaptiveStreamingInfo>(
          dlsym(media_player_handle_, "player_get_adaptive_streaming_info"));
  if (!player_get_adaptive_streaming_info) {
    LOG_ERROR("Fail to find player_get_adaptive_streaming_info.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_get_adaptive_streaming_info(player, adaptive_info,
                                            adaptive_type);
}

int MediaPlayerProxy::player_get_track_count_v2(player_h player,
                                                player_stream_type_e type,
                                                int* pcount) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerGetTrackCountV2 player_get_track_count_v2 =
      reinterpret_cast<FuncPlayerGetTrackCountV2>(
          dlsym(media_player_handle_, "player_get_track_count_v2"));
  if (!player_get_track_count_v2) {
    LOG_ERROR("Fail to find player_get_track_count_v2.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_get_track_count_v2(player, type, pcount);
}

int MediaPlayerProxy::player_get_video_track_info_v2(
    player_h player, int index, player_video_track_info_v2** track_info) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerGetVideoTrackInfoV2 player_get_video_track_info_v2 =
      reinterpret_cast<FuncPlayerGetVideoTrackInfoV2>(
          dlsym(media_player_handle_, "player_get_video_track_info_v2"));
  if (!player_get_video_track_info_v2) {
    LOG_ERROR("Fail to find player_get_video_track_info_v2.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_get_video_track_info_v2(player, index, track_info);
}

int MediaPlayerProxy::player_get_audio_track_info_v2(
    player_h player, int index, player_audio_track_info_v2** track_info) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerGetAudioTrackInfoV2 player_get_audio_track_info_v2 =
      reinterpret_cast<FuncPlayerGetAudioTrackInfoV2>(
          dlsym(media_player_handle_, "player_get_audio_track_info_v2"));
  if (!player_get_audio_track_info_v2) {
    LOG_ERROR("Fail to find player_get_audio_track_info_v2.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_get_audio_track_info_v2(player, index, track_info);
}

int MediaPlayerProxy::player_get_subtitle_track_info_v2(
    player_h player, int index, player_subtitle_track_info_v2** track_info) {
  if (!media_player_handle_) {
    LOG_ERROR("media_player_handle_ not valid");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  FuncPlayerGetSubtitleTrackInfoV2 player_get_subtitle_track_info_v2 =
      reinterpret_cast<FuncPlayerGetSubtitleTrackInfoV2>(
          dlsym(media_player_handle_, "player_get_subtitle_track_info_v2"));
  if (!player_get_subtitle_track_info_v2) {
    LOG_ERROR("Fail to find player_get_subtitle_track_info_v2.");
    return PLAYER_ERROR_NOT_AVAILABLE;
  }
  return player_get_subtitle_track_info_v2(player, index, track_info);
}
