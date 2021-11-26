#include "database_manager.h"

#include <flutter/standard_method_codec.h>
#include <sqlite3.h>

#include <list>
#include <variant>

#include "errors.h"

namespace sqflite_database {
DatabaseManager::~DatabaseManager() {
  for (auto &&stmt : stmt_chache_) {
    FinalizeStmt(stmt.second);
    stmt.second = nullptr;
  }
  if (database_ != nullptr) {
    Close();
  }
}

void DatabaseManager::ThrowCurrentDatabaseError() {
  throw sqflite_errors::DatabaseError(GetErrorCode(), GetErrorMsg());
}

void DatabaseManager::Init() {
  int result_code = SQLITE_OK;
  result_code = sqlite3_shutdown();
  if (result_code != SQLITE_OK) {
    ThrowCurrentDatabaseError();
  };
  result_code = sqlite3_config(SQLITE_CONFIG_URI, 1);
  if (result_code != SQLITE_OK) {
    ThrowCurrentDatabaseError();
  }
  result_code = sqlite3_initialize();
  if (result_code != SQLITE_OK) {
    ThrowCurrentDatabaseError();
  }
}

void DatabaseManager::Open() {
  Init();
  int result_code =
      sqlite3_open_v2(path_.c_str(), &database_,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (result_code != SQLITE_OK) {
    ThrowCurrentDatabaseError();
  }
  result_code = sqlite3_busy_timeout(database_, kBusyTimeoutMs);
  if (result_code != SQLITE_OK) {
    sqlite3_close(database_);
    ThrowCurrentDatabaseError();
  }
}

void DatabaseManager::OpenReadOnly() {
  Init();
  int result_code =
      sqlite3_open_v2(path_.c_str(), &database_, SQLITE_OPEN_READONLY, NULL);
  if (result_code != SQLITE_OK) {
    ThrowCurrentDatabaseError();
  }
  result_code = sqlite3_busy_timeout(database_, kBusyTimeoutMs);
  if (result_code != SQLITE_OK) {
    sqlite3_close(database_);
    ThrowCurrentDatabaseError();
  }
}
const char *DatabaseManager::GetErrorMsg() { return sqlite3_errmsg(database_); }

int DatabaseManager::GetErrorCode() {
  return sqlite3_extended_errcode(database_);
}

void DatabaseManager::Close() {
  int result_code = sqlite3_close(database_);
  if (result_code) {
    ThrowCurrentDatabaseError();
  }
}

void DatabaseManager::BindStmtParams(DatabaseManager::Statement stmt,
                                     SQLParameters params) {
  int err = SQLITE_OK;
  const int params_length = params.size();
  for (int i = 0; i < params_length; i++) {
    auto idx = i + 1;
    auto param = params[i];
    switch (param.index()) {
      case 0: {
        err = sqlite3_bind_null(stmt, idx);
        break;
      }
      case 1: {
        auto val = std::get<bool>(param);
        err = sqlite3_bind_int(stmt, idx, int(val));
        break;
      }
      case 2: {
        auto val = std::get<int32_t>(param);
        err = sqlite3_bind_int(stmt, idx, val);
        break;
      }
      case 3: {
        auto val = std::get<int64_t>(param);
        err = sqlite3_bind_int64(stmt, idx, val);
        break;
      }
      case 4: {
        auto val = std::get<double>(param);
        err = sqlite3_bind_double(stmt, idx, val);
        break;
      }
      case 5: {
        auto val = std::get<std::string>(param);
        err = sqlite3_bind_text(stmt, idx, val.c_str(), val.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 6: {
        auto vec = std::get<std::vector<uint8_t>>(param);
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 7: {
        auto vec = std::get<std::vector<int32_t>>(param);
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 8: {
        auto vec = std::get<std::vector<int64_t>>(param);
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 9: {
        auto vec = std::get<std::vector<double>>(param);
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 10: {
        auto val = std::get<flutter::EncodableList>(param);
        std::vector<uint8_t> vec;
        // Only  a list of uint8_t for flutter EncodableValue is supported
        // to store it as a BLOB, otherwise a DatabaseError is triggered
        try {
          for (auto item : val) {
            vec.push_back(std::get<int>(item));
          }
        } catch (const std::bad_variant_access) {
          throw sqflite_errors::DatabaseError(
              sqflite_errors::kUnknownErrorCode,
              "statement parameter is not supported");
        }
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      default:
        throw sqflite_errors::DatabaseError(
            sqflite_errors::kUnknownErrorCode,
            "statement parameter is not supported");
    }
    if (err) {
      ThrowCurrentDatabaseError();
    }
  }
}

sqlite3_stmt *DatabaseManager::PrepareStmt(std::string sql) {
  auto cache_entry = stmt_chache_.find(sql);
  if (cache_entry != stmt_chache_.end()) {
    sqlite3_stmt *stmt = cache_entry->second;
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return stmt;
  } else {
    sqlite3_stmt *stmt;
    int result_code =
        sqlite3_prepare_v2(database_, sql.c_str(), -1, &stmt, nullptr);
    if (result_code) {
      ThrowCurrentDatabaseError();
    }
    if (stmt != nullptr) {
      stmt_chache_[sql] = stmt;
    }
    return stmt;
  }
}

void DatabaseManager::ExecuteStmt(DatabaseManager::Statement stmt) {
  int result_code = SQLITE_OK;
  do {
    result_code = sqlite3_step(stmt);
  } while (result_code == SQLITE_ROW);
  if (result_code != SQLITE_DONE) {
    ThrowCurrentDatabaseError();
  }
}

int DatabaseManager::GetStmtColumnsCount(DatabaseManager::Statement stmt) {
  return sqlite3_column_count(stmt);
}

int DatabaseManager::GetColumnType(DatabaseManager::Statement stmt, int iCol) {
  return sqlite3_column_type(stmt, iCol);
}

const char *DatabaseManager::GetColumnName(DatabaseManager::Statement stmt,
                                           int iCol) {
  return sqlite3_column_name(stmt, iCol);
}

std::pair<Columns, Resultset> DatabaseManager::QueryStmt(
    DatabaseManager::Statement stmt) {
  Columns cols;
  Resultset rs;
  const int cols_count = GetStmtColumnsCount(stmt);
  int result_code = SQLITE_OK;
  for (int i = 0; i < cols_count; i++) {
    auto cName = GetColumnName(stmt, i);
    cols.push_back(std::string(cName));
  }
  do {
    result_code = sqlite3_step(stmt);
    if (result_code == SQLITE_ROW) {
      Result result;
      for (int i = 0; i < cols_count; i++) {
        ResultValue val;
        auto columnType = GetColumnType(stmt, i);
        auto columnName = GetColumnName(stmt, i);
        switch (columnType) {
          case SQLITE_INTEGER:
            val = (int64_t)sqlite3_column_int64(stmt, i);
            result.push_back(val);
            break;
          case SQLITE_FLOAT:
            val = sqlite3_column_double(stmt, i);
            result.push_back(val);
            break;
          case SQLITE_TEXT:
            val = std::string((const char *)sqlite3_column_text(stmt, i));
            result.push_back(val);
            break;
          case SQLITE_BLOB: {
            const uint8_t *blob =
                reinterpret_cast<const uint8_t *>(sqlite3_column_blob(stmt, i));
            std::vector<uint8_t> v(&blob[0],
                                   &blob[sqlite3_column_bytes(stmt, i)]);
            result.push_back(v);
            break;
          }
          case SQLITE_NULL:
            val = nullptr;
            result.push_back(val);
            break;
          default:
            break;
        }
      }
      rs.push_back(result);
    }
  } while (result_code == SQLITE_ROW);
  if (result_code != SQLITE_DONE) {
    ThrowCurrentDatabaseError();
  }
  return std::make_pair(cols, rs);
}

std::pair<Columns, Resultset> DatabaseManager::Query(std::string sql,
                                                     SQLParameters params) {
  auto stmt = PrepareStmt(sql);
  BindStmtParams(stmt, params);
  return QueryStmt(stmt);
}

void DatabaseManager::FinalizeStmt(DatabaseManager::Statement stmt) {
  sqlite3_finalize(stmt);
}

void DatabaseManager::Execute(std::string sql, SQLParameters params) {
  auto stmt = PrepareStmt(sql);
  BindStmtParams(stmt, params);
  ExecuteStmt(stmt);
}
}  // namespace sqflite_database
