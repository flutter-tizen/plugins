#ifndef LOG_LEVEL_H
#define LOG_LEVEL_H

namespace btlog {
enum class LogLevel {
  EMERGENCY,
  ALERT,
  CRITICAL,
  ERROR,
  WARNING,
  NOTICE,
  INFO,
  DEBUG,
};
}
#endif  // LOG_LEVEL_H
