#ifndef LOGGER_H
#define LOGGER_H

#include <Utils.h>
#include <dlog.h>

#include <string>

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
class Logger {
  inline static btu::SafeType<LogLevel> logLevel{LogLevel::DEBUG};
  inline static std::string logTag = "FlutterBlueTizenPlugin";
  inline static std::mutex m;

 public:
  static void setLogLevel(LogLevel _logLevel) noexcept;
  static void log(LogLevel level, const std::string& mess) noexcept;
  static void showResultError(std::string componentName, int res);
};
}  // namespace btlog

#endif  // LOGGER_H
