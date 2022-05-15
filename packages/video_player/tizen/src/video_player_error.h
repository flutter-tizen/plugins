// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_VIDEO_PLAYER_ERROR_H_
#define FLUTTER_PLUGIN_VIDEO_PLAYER_ERROR_H_

#include <string>

class VideoPlayerError {
 public:
  VideoPlayerError(const std::string &code, const std::string &message)
      : code_(code), message_(message) {}
  ~VideoPlayerError() = default;

  VideoPlayerError(const VideoPlayerError &other) {
    this->code_ = other.code_;
    this->message_ = other.message_;
  }
  VideoPlayerError &operator=(const VideoPlayerError &other) {
    this->code_ = other.code_;
    this->message_ = other.message_;
    return *this;
  }

  std::string getCode() const { return code_; }
  std::string getMessage() const { return message_; }

 private:
  std::string code_;
  std::string message_;
};

#endif  // FLUTTER_PLUGIN_VIDEO_PLAYER_ERROR_H_
