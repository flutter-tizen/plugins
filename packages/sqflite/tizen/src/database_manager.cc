#include "database_manager.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <iomanip>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "list"
#include "sqlite3.h"
// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

DatabaseManager::DatabaseManager(std::string aPath, int aId,
                                 bool aSingleInstance, int aLogLevel) {
  path = aPath;
  singleInstance = aSingleInstance;
  id = aId;
  logLevel = aLogLevel;
}
DatabaseManager::~DatabaseManager(){};

void init() {
  sqlite3_shutdown();
  sqlite3_config(SQLITE_CONFIG_URI, 1);
  sqlite3_initialize();
}

int DatabaseManager::open() {
  init();
  return sqlite3_open_v2(path.c_str(), &sqliteDatabase,
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
}

// Change default error handler to avoid erasing the existing file.
int DatabaseManager::openReadOnly() {
  init();
  return sqlite3_open_v2(path.c_str(), &sqliteDatabase, SQLITE_READONLY, NULL);
}
const char *DatabaseManager::getErrorMsg() {
  return sqlite3_errmsg(sqliteDatabase);
}

int DatabaseManager::close() { return sqlite3_close(sqliteDatabase); }
void DatabaseManager::remove() {}

sqlite3 *DatabaseManager::getWritableDatabase() { return sqliteDatabase; }
sqlite3 *DatabaseManager::getReadableDatabase() { return sqliteDatabase; }
int DatabaseManager::execSQL(std::string sql, flutter::EncodableList params) {
  sqlite3_stmt *stmt;
  int rc = sqlite3_prepare_v2(sqliteDatabase, sql.c_str(), sql.length(), &stmt,
                              nullptr);
  if (rc != SQLITE_OK) {
    return rc;
  }
  // int paramsLength = params.size();
  // auto it = params.begin();
  // for (int i = 0; paramsLength; i++) {
  //   std::advance(it, i);
  //   auto param = *it;
  //   // 4. another type-matching visitor: a class with 3 overloaded
  //   // operator()'s Note: The `(auto arg)` template operator() will bind to
  //   // `int` and `long`
  //   //       in this case, but in its absence the `(double arg)` operator()
  //   //       *will also* bind to `int` and `long` because both are implicitly
  //   //       convertible to double. When using this form, care has to be
  //   taken
  //   //       that implicit conversions are handled correctly.
  //   std::visit(overloaded{
  //                  [stmt, i](int arg) { sqlite3_bind_int(stmt, i, arg); },
  //                  [stmt, i](double arg) { sqlite3_bind_double(stmt, i, arg);
  //                  }, [stmt, i](long arg) { sqlite3_bind_double(stmt, i,
  //                  arg); }, [stmt, i](const std::string &arg) {
  //                    sqlite3_bind_text(stmt, i, arg.c_str(), -1,
  //                    SQLITE_STATIC);
  //                  },
  //              },
  //              param);
  //   if (rc != SQLITE_OK) {
  //     return rc;
  //   }
  // }
  // if (rc != SQLITE_OK) {
  //   return rc;
  // }
  int stepResult = sqlite3_step(stmt);
  while (stepResult == SQLITE_ROW) {
    stepResult = sqlite3_step(stmt);
  }
  return sqlite3_finalize(stmt);
}

// sqlite3 getReadableDatabase() { return sqliteDatabase; }

//   boolean enableWriteAheadLogging() {
//     try {
//       return sqliteDatabase.enableWriteAheadLogging();
//     } catch (Exception e) {
//       Log.e(TAG, getThreadLogPrefix() + "enable WAL error: " + e);
//       return false;
//     }
//   }

//   String getThreadLogTag() {
//     Thread thread = Thread.currentThread();

//     return "" + id + "," + thread.getName() + "(" + thread.getId() + ")";
//   }

//   String getThreadLogPrefix() { return "[" + getThreadLogTag() + "] "; }

// static void deleteDatabase(std::string path) {
//   sqlite3_.deleteDatabase(new File(path));
// }