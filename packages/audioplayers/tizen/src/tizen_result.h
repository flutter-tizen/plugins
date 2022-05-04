#ifndef FLUTTER_PLUGIN_TIZEN_RESULT_H_
#define FLUTTER_PLUGIN_TIZEN_RESULT_H_

#include <tizen.h>

#include <string>

class TizenResult {
 public:
  // Creates a "successful" result.
  TizenResult() : TizenResult(TIZEN_ERROR_NONE, "") {}
  TizenResult(int code) : TizenResult(code, "") {}
  TizenResult(int code, const std::string &message)
      : error_code_(code), message_(message) {}

  // Returns false on error.
  operator bool() const { return (error_code_ == TIZEN_ERROR_NONE); }

  std::string TizenMessage() const { return get_error_message(error_code_); }
  std::string Message() const {
    return message_.empty() ? get_error_message(error_code_) : message_;
  }

 private:
  int error_code_ = TIZEN_ERROR_NONE;
  std::string message_;
};

#endif  // FLUTTER_PLUGIN_TIZEN_RESULT_H_
