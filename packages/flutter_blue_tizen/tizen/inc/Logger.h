#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include <dlog.h>
#include <LogLevel.h>
#include <Utils.h>

namespace btlog{
    class Logger{
        inline static btu::SafeType<LogLevel> logLevel{LogLevel::DEBUG};
        inline static std::string logTag = "FlutterBlueTizenPlugin";
        inline static std::mutex m;
    public:
        static auto setLogLevel(LogLevel _logLevel) noexcept -> void;
        static auto log(LogLevel level, const std::string& mess) noexcept -> void;
        static auto showResultError(std::string componentName, int res) -> void;
    };
} // namespace btlog

#endif //LOGGER_H
