// Copyright 2024 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_player_proxy.h"

#include <dlfcn.h>

#include "log.h"

typedef int (*FuncPlayerGetAdaptiveStreamingInfo)(player_h player,
                                                  void* adaptive_info,
                                                  int adaptive_type);

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
