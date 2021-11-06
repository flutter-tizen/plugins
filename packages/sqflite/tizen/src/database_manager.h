#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <flutter/standard_method_codec.h>

#include "list"
#include "sqlite3.h"
#include "string"

using std::string;

const string DATABASE_ERROR_CODE = "sqlite_error";
const string DATABASE_MSG_ERROR_CLOSED = "database_closed";
const string DATABASE_MSG_OPEN_FAILED = "open_failed";

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
  typedef std::list<std::pair<string, resultvalue>> result;
  typedef std::list<result> resultset;
  typedef std::list<string> columns;
  typedef flutter::EncodableList parameters;

  void open();
  void openReadOnly();
  const char *getErrorMsg();
  int getErrorCode();
  sqlite3 *getWritableDatabase();
  sqlite3 *getReadableDatabase();
  void execute(string sql, parameters params);
  void query(string sql, parameters params, columns &cols, resultset &rs);

 private:
  typedef sqlite3_stmt *statement;

  void init();
  void close();
  void prepareStmt(statement stmt, string sql);
  void bindStmtParams(statement stmt, parameters params);
  void executeStmt(statement stmt);
  void queryStmt(statement stmt, columns &cols, resultset &rs);
  void finalizeStmt(statement stmt);
  sqlite3_stmt *prepareStmt(string sql);
  int getStmtColumnsCount(statement stmt);
  int getColumnType(statement stmt, int iCol);
  const char *getColumnName(statement stmt, int iCol);
};
#endif  // DATABASE_MANAGER_H_