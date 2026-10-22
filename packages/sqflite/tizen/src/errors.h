#ifndef SQFLITE_ERRORS_H_
#define SQFLITE_ERRORS_H_

#include <stdexcept>

namespace sqflite_errors {

const int kUnknownErrorCode = -1;

struct DatabaseError : public std::runtime_error {
  DatabaseError(int code, const char *msg)
      : std::runtime_error(std::string(msg) + " (code " + std::to_string(code) +
                           ")") {}

  bool transaction_closed = false;
};
}  // namespace sqflite_errors

#endif  // SQFLITE_ERRORS_H_
