#ifndef ERRORS_H
#define ERRORS_H

#include <stdexcept>

struct DatabaseError : public std::runtime_error {
  DatabaseError(int code, const char *msg)
      : std::runtime_error(std::string(msg) + " (code " + std::to_string(code) +
                           ")") {}
};

#endif  // ERRORS_H
