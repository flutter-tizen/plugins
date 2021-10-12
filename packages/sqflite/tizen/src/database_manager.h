#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <flutter/standard_method_codec.h>

#include "list"
#include "sqlite3.h"
#include "string"

using std::string;

const string DATABASE_ERROR_CODE = "sqlite_error";
const string DATABASE_MSG_ERROR_CLOSED = "database_closed";

const int DATABASE_STATUS_OK = SQLITE_OK;

class DatabaseManager {
 public:
  bool singleInstance;
  string path;
  int id;
  int logLevel;
  sqlite3 *sqliteDatabase;

  DatabaseManager(string aPath, int aId, bool aSingleInstance, int aLogLevel);
  ~DatabaseManager();

  typedef std::variant<int, string, double, std::nullptr_t> resultvalue;
  typedef std::list<std::pair<string, resultvalue>> result;
  typedef std::list<result> resultset;
  typedef std::list<string> columns;
  typedef flutter::EncodableList parameters;

  int open();
  int openReadOnly();
  int close();
  const char *getErrorMsg();
  sqlite3 *getWritableDatabase();
  sqlite3 *getReadableDatabase();
  int execute(string sql, parameters params);
  int query(string sql, parameters params, columns &cols, resultset &rs);

 private:
  typedef sqlite3_stmt *statement;

  int init();
  int prepareStmt(statement stmt, string sql);
  int bindStmtParams(statement stmt, parameters params);
  int executeStmt(statement stmt);
  int queryStmt(statement stmt, columns &cols, resultset &rs);
  int finalizeStmt(statement stmt);
  int getStmtColumnsCount(statement stmt);
  int getColumnType(statement stmt, int iCol);
  const char *getColumnName(statement stmt, int iCol);
};
#endif  // DATABASE_MANAGER_H_