// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_TIZEN_PLUGINS_WRAPPER_VIDEO_PLAYER_H_
#define FLUTTER_TIZEN_PLUGINS_WRAPPER_VIDEO_PLAYER_H_

#include <stdbool.h>

typedef enum {
  PLAYER_ADAPTIVE_INFO_LIVE_DURATION = 3,
  PLAYER_ADAPTIVE_INFO_IS_LIVE = 19,
} player_adaptive_Info_e;

#ifdef __cplusplus
extern "C" {
#endif

int ftpw_video_player_player_get_adaptive_streaming_info(void *player,
                                                         void *adaptive_info,
                                                         int adaptive_type);
int ftpw_video_player_screensaver_reset_timeout(void);
int ftpw_video_player_screensaver_override_reset(bool onoff);

#ifdef __cplusplus
}
#endif

#endif
