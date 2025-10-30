// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plus_player_util.h"

#include "log.h"
namespace video_player_avplay_tizen {

static constexpr plusplayer_track_type_e kUnknownTrackType =
    static_cast<plusplayer_track_type_e>(-1);

static const std::unordered_map<std::string, plusplayer_track_type_e>
    kTrackMap = {{"video", PLUSPLAYER_TRACK_TYPE_VIDEO},
                 {"audio", PLUSPLAYER_TRACK_TYPE_AUDIO},
                 {"text", PLUSPLAYER_TRACK_TYPE_SUBTITLE}};

static const std::unordered_map<std::string, plusplayer_property_e>
    kConvertPropertyMap = {
        {"ADAPTIVE_INFO", PLUSPLAYER_PROPERTY_ADAPTIVE_INFO},
        {"LISTEN_SPARSE_TRACK", PLUSPLAYER_PROPERTY_LISTEN_SPARSE_TRACK},
        {"CONFIG_LOW_LATENCY", PLUSPLAYER_PROPERTY_CONFIG_LOW_LATENCY},
        {"ATSC3_L1_SERVER_TIME", PLUSPLAYER_PROPERTY_ATSC3_L1_SERVER_TIME},
        {"AUDIO_DESCRIPTION", PLUSPLAYER_PROPERTY_AUDIO_DESCRIPTION},
        {"PRESELECTION_TAG", PLUSPLAYER_PROPERTY_PRESELECTION_TAG},
        {"USE_MAIN_OUT_SHARE", PLUSPLAYER_PROPERTY_USE_MAIN_OUT_SHARE},
        {"TOKEN", PLUSPLAYER_PROPERTY_URL_AUTH_TOKEN},
        {"USER_LOW_LATENCY", PLUSPLAYER_PROPERTY_USER_LOW_LATENCY},
        {"MAX_BANDWIDTH", PLUSPLAYER_PROPERTY_MAX_BANDWIDTH},
        {"MPEGH_METADATA", PLUSPLAYER_PROPERTY_MPEGH_METADATA},
        {"OPEN_HTTP_HEADER", PLUSPLAYER_PROPERTY_OPEN_HTTP_HEADER},
        {"AVAILABLE_BITRATE", PLUSPLAYER_PROPERTY_AVAILABLE_BITRATE},
        {"CUR_LATENCY", PLUSPLAYER_PROPERTY_CURRENT_LATENCY},
        {"IS_DVB_DASH", PLUSPLAYER_PROPERTY_IS_DVB_DASH},
        {"LIVE_PLAYER_START", PLUSPLAYER_PROPERTY_LIVE_PLAYER_START},
        {"START_DATE", PLUSPLAYER_PROPERTY_START_DATE},
        {"DASH_STREAM_INFO", PLUSPLAYER_PROPERTY_DASH_STREAM_INFO},
        {"HTTP_HEADER", PLUSPLAYER_PROPERTY_HTTP_HEADER}};

static const std::vector<plusplayer_display_rotation_type_e>
    kConvertDisplayRotationVec = {PLUSPLAYER_DISPLAY_ROTATION_TYPE_NONE,
                                  PLUSPLAYER_DISPLAY_ROTATION_TYPE_90,
                                  PLUSPLAYER_DISPLAY_ROTATION_TYPE_180,
                                  PLUSPLAYER_DISPLAY_ROTATION_TYPE_270};

static const std::vector<plusplayer_display_mode_e> kConvertDisplayModeMap = {
    PLUSPLAYER_DISPLAY_MODE_LETTER_BOX,
    PLUSPLAYER_DISPLAY_MODE_ORIGIN_SIZE,
    PLUSPLAYER_DISPLAY_MODE_FULL_SCREEN,
    PLUSPLAYER_DISPLAY_MODE_CROPPED_FULL,
    PLUSPLAYER_DISPLAY_MODE_ORIGIN_OR_LETTER,
    PLUSPLAYER_DISPLAY_MODE_DST_ROI,
    PLUSPLAYER_DISPLAY_MODE_AUTO_ASPECT_RATIO,
    PLUSPLAYER_DISPLAY_MODE_ROI_AUTO_ASPECT_RATIO};

plusplayer_display_rotation_type_e ConvertDisplayRotationType(
    const int64_t &rotation_type) {
  int index = static_cast<int>(rotation_type);
  return kConvertDisplayRotationVec[index];
}

plusplayer_display_mode_e ConvertDisplayMode(const int64_t &display_mode) {
  int index = static_cast<int>(display_mode);
  return (index >= 0 && index < kConvertDisplayModeMap.size())
             ? kConvertDisplayModeMap[index]
             : static_cast<plusplayer_display_mode_e>(-1);
}

plusplayer_track_type_e ConvertTrackType(const std::string &track_type) {
  auto it = kTrackMap.find(track_type);
  if (it != kTrackMap.end()) {
    return it->second;
  }
  // Return a sentinel value that does not correspond to any defined enum.
  return kUnknownTrackType;
}

plusplayer_property_e ConvertPropertyType(const std::string &type_value) {
  for (const auto &pair : kConvertPropertyMap) {
    if (pair.first == type_value) {
      return pair.second;
    }
  }
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
    case PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_FILE:
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
    case PLUSPLAYER_ERROR_TYPE_NOT_PERMITTED:
      return "DRM format not permitted";
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
    case PLUSPLAYER_ERROR_TYPE_DRM_DECRYPTION_FAILED:
      return "Drm decryption failed";
    case PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_FORMAT:
      return "Not supported media format";
    case PLUSPLAYER_ERROR_TYPE_STREAMING_PLAYER:
      return "Streaming player error";
    case PLUSPLAYER_ERROR_TYPE_DTCPFSK:
      return "Type dtcpfsk";
    case PLUSPLAYER_ERROR_TYPE_PRELOADING_TIMEOUT:
      return "Can't finish preloading in time";
    case PLUSPLAYER_ERROR_TYPE_NETWORK_ERROR:
      return "Network error";
    case PLUSPLAYER_ERROR_TYPE_NOT_CHANNEL_SURFING_ERROR:
      return "Not channel surfing error";
    default:
      return "Not defined error";
  }
}

}  // namespace video_player_avplay_tizen
