#ifndef VIDEO_PLAYER_ERROR_H_
#define VIDEO_PLAYER_ERROR_H_

#include <string>

class VideoPlayerError {
 public:
  VideoPlayerError(const std::string &message, const std::string &code)
      : message_(message), code_(code) {}
  ~VideoPlayerError() = default;

  VideoPlayerError(const VideoPlayerError &other) {
    this->message_ = other.message_;
    this->code_ = other.code_;
  }
  VideoPlayerError &operator=(const VideoPlayerError &other) {
    this->message_ = other.message_;
    this->code_ = other.code_;
    return *this;
  }

  std::string getMessage() const { return message_; }
  std::string getCode() const { return code_; }

 private:
  std::string message_;
  std::string code_;
};

#endif  // VIDEO_PLAYER_ERROR_H_