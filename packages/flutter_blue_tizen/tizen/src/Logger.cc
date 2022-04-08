#include <LogLevel.h>
#include <Logger.h>

namespace btlog {
auto Logger::log(LogLevel messLevel, const std::string& mess) noexcept -> void {
  if (messLevel <= logLevel.var) {
    log_priority p;
    switch (messLevel) {
      case LogLevel::EMERGENCY:
        p = log_priority::DLOG_FATAL;
        break;
      case LogLevel::ALERT:
        p = log_priority::DLOG_FATAL;
        break;
      case LogLevel::CRITICAL:
        p = log_priority::DLOG_FATAL;
        break;
      case LogLevel::ERROR:
        p = log_priority::DLOG_ERROR;
        break;
      case LogLevel::WARNING:
        p = log_priority::DLOG_WARN;
        break;
      case LogLevel::NOTICE:
        p = log_priority::DLOG_INFO;
        break;
      case LogLevel::INFO:
        p = log_priority::DLOG_INFO;
        break;
      case LogLevel::DEBUG:
        p = log_priority::DLOG_DEBUG;
        break;
      default:
        p = log_priority::DLOG_VERBOSE;
        break;
    }
    std::scoped_lock l(m);
    dlog_print(p, logTag.c_str(), mess.c_str(), "");
  }
}
auto Logger::setLogLevel(LogLevel _logLevel) noexcept -> void {
  std::scoped_lock lock(logLevel.mut);
  logLevel.var = _logLevel;
  log(LogLevel::DEBUG,
      "set log level to =  " + std::to_string(static_cast<int>(logLevel.var)));
}
auto Logger::showResultError(std::string componentName, int res) -> void {
  if (res) {
    std::string err = get_error_message(res);
    Logger::log(LogLevel::ERROR, "[" + componentName + "] failed with " + err);
  }
}

}  // namespace btlog
