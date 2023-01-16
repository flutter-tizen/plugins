// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_AUDIO_MANAGER_ERROR_H_
#define FLUTTER_PLUGIN_AUDIO_MANAGER_ERROR_H_

#include <string>

class AudioManagerError {
 public:
  AudioManagerError(const std::string &code, const std::string &message)
      : code_(code), message_(message) {}
  ~AudioManagerError() = default;

  AudioManagerError(const AudioManagerError &other) {
    this->code_ = other.code_;
    this->message_ = other.message_;
  }
  AudioManagerError &operator=(const AudioManagerError &other) {
    this->code_ = other.code_;
    this->message_ = other.message_;
    return *this;
  }

  std::string code() const { return code_; }
  std::string message() const { return message_; }

 private:
  std::string code_;
  std::string message_;
};

#endif  // FLUTTER_PLUGIN_AUDIO_MANAGER_ERROR_H_
