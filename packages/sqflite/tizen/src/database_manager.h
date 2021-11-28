#ifndef SQFLITE_DATABASE_MANAGER_H_
#define SQFLITE_DATABASE_MANAGER_H_

#include <flutter/standard_method_codec.h>
#include <sqlite3.h>

#include <list>
#include <string>

namespace sqflite_database {

typedef sqlite3 *Database;
typedef std::variant<int64_t, std::string, double, std::vector<uint8_t>,
                     std::nullptr_t>
    ResultValue;
typedef std::vector<ResultValue> Result;
typedef std::vector<Result> Resultset;
typedef std::vector<std::string> Columns;
typedef flutter::EncodableList SQLParameters;

class DatabaseManager {
 public:
  static const int kBusyTimeoutMs = 2500;

  DatabaseManager(std::string path, int id, bool single_instance, int log_level)
      : path_(path),
        database_id_(id),
        single_instance_(single_instance),
        log_level_(log_level){};
  virtual ~DatabaseManager();

  inline const std::string path() { return path_; };
  inline const int log_level() { return log_level_; };
  inline const bool single_instance() { return single_instance_; };
  inline const int database_id() { return database_id_; };
  inline const Database database() { return database_; };

  void Open();
  void OpenReadOnly();
  const char *GetErrorMsg();
  int GetErrorCode();
  void Execute(std::string sql, SQLParameters params = SQLParameters());
  std::pair<Columns, Resultset> Query(std::string sql,
                                      SQLParameters params = SQLParameters());

 private:
  typedef sqlite3_stmt *Statement;

  void Init();
  void Close();
  void BindStmtParams(Statement stmt, SQLParameters params);
  void ExecuteStmt(Statement stmt);
  std::pair<Columns, Resultset> QueryStmt(Statement stmt);
  void FinalizeStmt(Statement stmt);
  Statement PrepareStmt(std::string sql);
  int GetStmtColumnsCount(Statement stmt);
  int GetColumnType(Statement stmt, int iCol);
  const char *GetColumnName(Statement stmt, int iCol);
  void ThrowCurrentDatabaseError();
  void LogQuery(Statement statement);

  Database database_;
  std::map<std::string, Statement> statement_cache_;
  bool single_instance_;
  std::string path_;
  int database_id_;
  int log_level_;
};
}  // namespace sqflite_database
#endif  // SQFLITE_DATABASE_MANAGER_H_
