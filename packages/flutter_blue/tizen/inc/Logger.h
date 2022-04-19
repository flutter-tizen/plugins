#ifndef FLUTTER_BLUE_TIZEN_LOGGER_H
#define FLUTTER_BLUE_TIZEN_LOGGER_H

#include <Utils.h>
#include <dlog.h>

#include <string>
namespace flutter_blue_tizen {
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
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_LOGGER_H
