#ifndef FLUTTER_PLUGIN_AUDIO_PLAYER_ERROR_H_
#define FLUTTER_PLUGIN_AUDIO_PLAYER_ERROR_H_

#include <string>

class AudioPlayerError {
 public:
  AudioPlayerError(const std::string &code, const std::string &message)
      : code_(code), message_(message) {}
  ~AudioPlayerError() = default;

  AudioPlayerError(const AudioPlayerError &other) {
    this->code_ = other.code_;
    this->message_ = other.message_;
  }
  AudioPlayerError &operator=(const AudioPlayerError &other) {
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

#endif  // FLUTTER_PLUGIN_AUDIO_PLAYER_ERROR_H_
