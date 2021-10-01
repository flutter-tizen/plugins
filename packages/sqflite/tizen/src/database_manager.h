#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include "list"
#include "sqlite3.h"
#include "string"

class DatabaseManager {
 public:
  bool singleInstance;
  std::string path;
  int id;
  int logLevel;
  sqlite3 *sqliteDatabase;
  bool inTransaction;

  DatabaseManager(std::string aPath, int aId, bool aSingleInstance,
                  int aLogLevel);
  ~DatabaseManager();

  int open();
  int openReadOnly();
  int close();
  void remove();
  const char *getErrorMsg();
  sqlite3 *getWritableDatabase();
  sqlite3 *getReadableDatabase();
  int execSQL(std::string sql, flutter::EncodableList params);
};
#endif  // DATABASE_MANAGER_H_