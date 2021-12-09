#ifndef SQFLITE_LOG_LEVEL_H_
#define SQFLITE_LOG_LEVEL_H_

namespace sqflite_log_level {

enum LogLevel { kNone, kSql, kVerbose };

inline int HasSqlLevel(int level) { return level >= kSql; }

inline int HasVerboseLevel(int level) { return level >= kVerbose; }

};  // namespace sqflite_log_level

#endif  // SQFLITE_LOG_LEVEL_H_
