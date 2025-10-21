// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PLUS_PLAYER_UTIL_H_
#define FLUTTER_PLUGIN_PLUS_PLAYER_UTIL_H_

#include <flutter/encodable_value.h>

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "plusplayer_capi/display.h"
#include "plusplayer_capi/error.h"
#include "plusplayer_capi/property.h"
#include "plusplayer_capi/state.h"
#include "plusplayer_capi/track.h"
#include "plusplayer_capi/track_capi.h"
namespace video_player_avplay_tizen {
struct PlayerMemento {
  uint64_t playing_time;
  plusplayer_state_e state;
  plusplayer_geometry_s display_area;
  bool is_live;
};

plusplayer_track_type_e ConvertTrackType(const std::string &track_type);
plusplayer_property_e ConvertPropertyType(const std::string &type_value);
std::vector<std::string> Split(const std::string &s, char delim);
std::string GetErrorMessage(plusplayer_error_type_e error_code);
}  // namespace video_player_avplay_tizen
#endif
