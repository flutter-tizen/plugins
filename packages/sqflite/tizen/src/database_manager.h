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

  DatabaseManager(std::string path, int database_id, bool single_instance,
                  int log_level)
      : path_(path),
        database_id_(database_id),
        single_instance_(single_instance),
        log_level_(log_level),
        database_(nullptr){};
  virtual ~DatabaseManager();

  inline const std::string path() { return path_; };
  inline const int database_id() { return database_id_; };
  inline const bool single_instance() { return single_instance_; };
  inline const int log_level() { return log_level_; };
  inline const Database database() { return database_; };

  void Open();
  void OpenReadOnly();
  const char *GetErrorMsg();
  int GetErrorCode();
  void Execute(std::string sql, SQLParameters parameters = SQLParameters());
  std::pair<Columns, Resultset> Query(
      std::string sql, SQLParameters parameters = SQLParameters());

 private:
  typedef sqlite3_stmt *Statement;

  void Close(bool raise_error);
  void BindStmtParams(Statement statement, SQLParameters parameters);
  void ExecuteStmt(Statement statement);
  std::pair<Columns, Resultset> QueryStmt(Statement statement);
  void FinalizeStmt(Statement statement);
  Statement PrepareStmt(std::string sql);
  int GetStmtColumnsCount(Statement statement);
  int GetColumnType(Statement statement, int column_index);
  const char *GetColumnName(Statement statement, int column_index);
  void ThrowCurrentDatabaseError();
  void LogQuery(Statement statement);

  std::map<std::string, Statement> statement_cache_;
  std::string path_;
  int database_id_;
  bool single_instance_;
  int log_level_;
  Database database_;
};
}  // namespace sqflite_database
#endif  // SQFLITE_DATABASE_MANAGER_H_
