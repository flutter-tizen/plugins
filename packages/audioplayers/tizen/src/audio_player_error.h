#ifndef AUDIO_PLAYER_ERROR_H_
#define AUDIO_PLAYER_ERROR_H_

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

  std::string GetCode() const { return code_; }
  std::string GetMessage() const { return message_; }

 private:
  std::string code_;
  std::string message_;
};

#endif  // AUDIO_PLAYER_ERROR_H_
