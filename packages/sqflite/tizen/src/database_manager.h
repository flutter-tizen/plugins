#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

#include <flutter/standard_method_codec.h>
#include <sqlite3.h>

#include <list>
#include <string>

class DatabaseManager {
 public:
  typedef std::variant<int64_t, std::string, double, std::vector<uint8_t>,
                       std::nullptr_t>
      resultvalue;
  typedef std::vector<resultvalue> result;
  typedef std::vector<result> resultset;
  typedef std::vector<std::string> columns;
  typedef flutter::EncodableList parameters;

  static const int kBusyTimeoutMs = 2500;

  DatabaseManager(std::string path, int id, bool single_instance, int log_level)
      : path_(path),
        database_id_(id),
        single_instance_(single_instance),
        log_level_(log_level){};
  virtual ~DatabaseManager();

  inline const std::string path() { return path_; };
  inline const int logLevel() { return log_level_; };
  inline const bool singleInstance() { return single_instance_; };
  inline const sqlite3 *database() { return database_; };

  void Open();
  void OpenReadOnly();
  const char *GetErrorMsg();
  int GetErrorCode();
  sqlite3 *GetWritableDatabase();
  sqlite3 *GetReadableDatabase();
  void Execute(std::string sql, parameters params = parameters());
  std::pair<DatabaseManager::columns, DatabaseManager::resultset> Query(
      std::string sql, parameters params = parameters());

 private:
  typedef sqlite3_stmt *statement;

  void Init();
  void Close();
  void BindStmtParams(statement stmt, parameters params);
  void ExecuteStmt(statement stmt);
  std::pair<DatabaseManager::columns, DatabaseManager::resultset> QueryStmt(
      statement stmt);
  void FinalizeStmt(statement stmt);
  sqlite3_stmt *PrepareStmt(std::string sql);
  int GetStmtColumnsCount(statement stmt);
  int GetColumnType(statement stmt, int iCol);
  const char *GetColumnName(statement stmt, int iCol);
  void ThrowCurrentDatabaseError();

  sqlite3 *database_;
  std::map<std::string, sqlite3_stmt *> stmt_chache_;
  bool single_instance_;
  std::string path_;
  int database_id_;
  int log_level_;
};
#endif  // DATABASE_MANAGER_H_
