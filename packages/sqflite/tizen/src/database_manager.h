#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <flutter/standard_method_codec.h>

#include "list"
#include "sqlite3.h"
#include "string"

using std::string;

struct DatabaseError : public std::runtime_error {
  DatabaseError(int code, const char *msg)
      : std::runtime_error(string(msg) + " (code " + std::to_string(code) +
                           ")") {}
};

class DatabaseManager {
 public:
  sqlite3 *sqliteDatabase;
  std::map<string, sqlite3_stmt *> stmtCache;
  bool singleInstance;
  string path;
  int id;
  int logLevel;

  DatabaseManager(string aPath, int aId, bool aSingleInstance, int aLogLevel);
  virtual ~DatabaseManager();

  typedef std::variant<int64_t, string, double, std::vector<uint8_t>,
                       std::nullptr_t>
      resultvalue;
  typedef std::vector<resultvalue> result;
  typedef std::vector<result> resultset;
  typedef std::vector<string> columns;
  typedef flutter::EncodableList parameters;

  void open();
  void openReadOnly();
  const char *getErrorMsg();
  int getErrorCode();
  sqlite3 *getWritableDatabase();
  sqlite3 *getReadableDatabase();
  void execute(string sql, parameters params = parameters());
  std::pair<DatabaseManager::columns, DatabaseManager::resultset> query(
      string sql, parameters params = parameters());

 private:
  typedef sqlite3_stmt *statement;

  void init();
  void close();
  void prepareStmt(statement stmt, string sql);
  void bindStmtParams(statement stmt, parameters params);
  void executeStmt(statement stmt);
  std::pair<DatabaseManager::columns, DatabaseManager::resultset> queryStmt(
      statement stmt);
  void finalizeStmt(statement stmt);
  sqlite3_stmt *prepareStmt(string sql);
  int getStmtColumnsCount(statement stmt);
  int getColumnType(statement stmt, int iCol);
  const char *getColumnName(statement stmt, int iCol);
};
#endif  // DATABASE_MANAGER_H_