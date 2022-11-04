// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIDEO_PLAYER_VIDEOHOLE_PLUGIN_VIDEO_PLAYER_OPTIONS_H_
#define VIDEO_PLAYER_VIDEOHOLE_PLUGIN_VIDEO_PLAYER_OPTIONS_H_

class VideoPlayerOptions {
 public:
  VideoPlayerOptions() {}
  ~VideoPlayerOptions() = default;

  VideoPlayerOptions(const VideoPlayerOptions &other) = default;
  VideoPlayerOptions &operator=(const VideoPlayerOptions &other) = default;

  void SetMixWithOthers(bool mix_with_others) {
    mix_with_others_ = mix_with_others;
  }
  bool GetMixWithOthers() const { return mix_with_others_; }

 private:
  bool mix_with_others_ = true;
};

#endif  // VIDEO_PLAYER_VIDEOHOLE_PLUGIN_VIDEO_PLAYER_OPTIONS_H_
