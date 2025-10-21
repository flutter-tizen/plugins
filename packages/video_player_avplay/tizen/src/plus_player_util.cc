// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plus_player_util.h"
#include "log.h"
namespace video_player_avplay_tizen {

static constexpr plusplayer_track_type_e kUnknownTrackType =
    static_cast<plusplayer_track_type_e>(-1);

static const std::unordered_map<std::string, plusplayer_track_type_e>
    kTrackMap = {
        {"video", plusplayer_track_type_e::PLUSPLAYER_TRACK_TYPE_VIDEO},
        {"audio", plusplayer_track_type_e::PLUSPLAYER_TRACK_TYPE_AUDIO},
        {"text", plusplayer_track_type_e::PLUSPLAYER_TRACK_TYPE_SUBTITLE}};

static const std::unordered_map<std::string, plusplayer_property_e> kMap = {
    {"ADAPTIVE_INFO", plusplayer_property_e::PLUSPLAYER_PROPERTY_ADAPTIVE_INFO},
    {"LISTEN_SPARSE_TRACK",
     plusplayer_property_e::PLUSPLAYER_PROPERTY_LISTEN_SPARSE_TRACK},
    {"AVAILABLE_BITRATE",
     plusplayer_property_e::PLUSPLAYER_PROPERTY_AVAILABLE_BITRATE},
    {"CURRENT_BANDWIDTH",
     plusplayer_property_e::PLUSPLAYER_PROPERTY_CURRENT_BANDWIDTH},
    {"GET_LIVE_DURATION",
     plusplayer_property_e::PLUSPLAYER_PROPERTY_LIVE_DURATION},
    {"IS_LIVE", plusplayer_property_e::PLUSPLAYER_PROPERTY_IS_LIVE},
    {"TOKEN", plusplayer_property_e::PLUSPLAYER_PROPERTY_TOKEN},
    {"OPEN_HTTP_HEADER",
     plusplayer_property_e::PLUSPLAYER_PROPERTY_OPEN_HTTP_HEADER}};

plusplayer_track_type_e ConvertTrackType(const std::string &track_type) {
  auto it = kTrackMap.find(track_type);
  if (it != kTrackMap.end()) {
    return it->second;
  }
  // Return a sentinel value that does not correspond to any defined enum.
  return kUnknownTrackType;
}

plusplayer_property_e ConvertPropertyType(const std::string &type_value) {
  auto it = kMap.find(type_value);
  if (it != kMap.end()) {
    return it->second;
  }
  // Return an invalid sentinel value if not found.
  return static_cast<plusplayer_property_e>(-1);
}

std::vector<std::string> Split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> tokens;
  while (getline(ss, item, delim)) {
    tokens.push_back(item);
  }
  return tokens;
}

std::string GetErrorMessage(plusplayer_error_type_e error_code) {
  switch (error_code) {
    case PLUSPLAYER_ERROR_TYPE_NONE:
      return "Successful";
    case PLUSPLAYER_ERROR_TYPE_OUT_OF_MEMORY:
      return "Out of memory";
    case PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER:
      return "Invalid parameter";
    case PLUSPLAYER_ERROR_TYPE_NO_SUCH_FILE:
      return "No such file or directory";
    case PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION:
      return "Invalid operation";
    case PLUSPLAYER_ERROR_TYPE_FILE_NO_SPACE_ON_DEVICE:
      return "No space left on the device";
    case PLUSPLAYER_ERROR_TYPE_FEATURE_NOT_SUPPORTED_ON_DEVICE:
      return "Not supported file on this device";
    case PLUSPLAYER_ERROR_TYPE_SEEK_FAILED:
      return "Seek operation failure";
    case PLUSPLAYER_ERROR_TYPE_INVALID_STATE:
      return "Invalid player state";
    case PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_FORMAT:
      return "File format not supported";
    case PLUSPLAYER_ERROR_TYPE_INVALID_URI:
      return "Invalid URI";
    case PLUSPLAYER_ERROR_TYPE_SOUND_POLICY:
      return "Sound policy error";
    case PLUSPLAYER_ERROR_TYPE_CONNECTION_FAILED:
      return "Streaming connection failed";
    case PLUSPLAYER_ERROR_TYPE_VIDEO_CAPTURE_FAILED:
      return "Video capture failed";
    case PLUSPLAYER_ERROR_TYPE_DRM_EXPIRED:
      return "DRM license expired";
    case PLUSPLAYER_ERROR_TYPE_DRM_NO_LICENSE:
      return "DRM no license";
    case PLUSPLAYER_ERROR_TYPE_DRM_FUTURE_USE:
      return "License for future use";
    case PLUSPLAYER_ERROR_TYPE_RESOURCE_LIMIT:
      return "Resource limit";
    case PLUSPLAYER_ERROR_TYPE_PERMISSION_DENIED:
      return "Permission denied";
    case PLUSPLAYER_ERROR_TYPE_SERVICE_DISCONNECTED:
      return "Service disconnected";
    case PLUSPLAYER_ERROR_TYPE_NO_BUFFER_SPACE:
      return "No buffer space available";
    case PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_AUDIO_CODEC:
      return "Not supported audio codec but video can be played";
    case PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_VIDEO_CODEC:
      return "Not supported video codec but audio can be played";
    case PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_SUBTITLE:
      return "Not supported subtitle format";
    default:
      return "Not defined error";
  }
}

}  // namespace video_player_avplay_tizen
