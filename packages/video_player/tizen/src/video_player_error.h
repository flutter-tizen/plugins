#ifndef VIDEO_PLAYER_ERROR_H_
#define VIDEO_PLAYER_ERROR_H_

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

#endif  // VIDEO_PLAYER_ERROR_H_
