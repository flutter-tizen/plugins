#include "database_manager.h"

#include <flutter/standard_method_codec.h>
#include <sqlite3.h>

#include <list>
#include <variant>

#include "errors.h"
#include "log.h"

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
  throw DatabaseError(GetErrorCode(), GetErrorMsg());
}

void DatabaseManager::Init() {
  LOG_DEBUG("initializing database");
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
  LOG_DEBUG("opening/creating read write database");
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
  LOG_DEBUG("open read only database");
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
  LOG_DEBUG("closing database");
  int result_code = sqlite3_close(database_);
  if (result_code) {
    ThrowCurrentDatabaseError();
  }
}

void DatabaseManager::BindStmtParams(DatabaseManager::statement stmt,
                                     DatabaseManager::parameters params) {
  int err = SQLITE_OK;
  const int params_length = params.size();
  LOG_DEBUG("received %d params to execute sql", params_length);
  for (int i = 0; i < params_length; i++) {
    auto idx = i + 1;
    auto param = params[i];
    switch (param.index()) {
      case 0: {
        LOG_DEBUG("binding null param");
        err = sqlite3_bind_null(stmt, idx);
        break;
      }
      case 1: {
        auto val = std::get<bool>(param);
        LOG_DEBUG("binding bool param: %d", int(val));
        err = sqlite3_bind_int(stmt, idx, int(val));
        break;
      }
      case 2: {
        auto val = std::get<int32_t>(param);
        LOG_DEBUG("binding param: %d", val);
        err = sqlite3_bind_int(stmt, idx, val);
        break;
      }
      case 3: {
        auto val = std::get<int64_t>(param);
        LOG_DEBUG("binding param: %d", val);
        err = sqlite3_bind_int64(stmt, idx, val);
        break;
      }
      case 4: {
        auto val = std::get<double>(param);
        LOG_DEBUG("binding param: %d", val);
        err = sqlite3_bind_double(stmt, idx, val);
        break;
      }
      case 5: {
        auto val = std::get<std::string>(param);
        LOG_DEBUG("binding param: %s", val.c_str());
        err = sqlite3_bind_text(stmt, idx, val.c_str(), val.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 6: {
        auto vec = std::get<std::vector<uint8_t>>(param);
        LOG_DEBUG("binding uint8 vector param of length: %d", vec.size());
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 7: {
        auto vec = std::get<std::vector<int32_t>>(param);
        LOG_DEBUG("binding int32 vector param of length: %d", vec.size());
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 8: {
        auto vec = std::get<std::vector<int64_t>>(param);
        LOG_DEBUG("binding int64 vector param of length: %d", vec.size());
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 9: {
        auto vec = std::get<std::vector<double>>(param);
        LOG_DEBUG("binding double vector param of length: %d", vec.size());
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      case 10: {
        auto val = std::get<flutter::EncodableList>(param);
        std::vector<uint8_t> vec;
        LOG_DEBUG("binding vector param from encodable list of length: %d",
                  val.size());
        // Only  a list of uint8_t for flutter EncodableValue is supported
        // to store it as a BLOB, otherwise a DatabaseError is triggered
        try {
          for (auto item : val) {
            vec.push_back(std::get<int>(item));
          }
        } catch (const std::bad_variant_access) {
          throw DatabaseError(-1, "statement parameter is not supported");
        }
        err = sqlite3_bind_blob(stmt, idx, vec.data(), (int)vec.size(),
                                SQLITE_TRANSIENT);
        break;
      }
      default:
        throw DatabaseError(-1, "statement parameter is not supported");
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

void DatabaseManager::ExecuteStmt(DatabaseManager::statement stmt) {
  int result_code = SQLITE_OK;
  do {
    result_code = sqlite3_step(stmt);
  } while (result_code == SQLITE_ROW);
  if (result_code != SQLITE_DONE) {
    ThrowCurrentDatabaseError();
  }
}

int DatabaseManager::GetStmtColumnsCount(DatabaseManager::statement stmt) {
  return sqlite3_column_count(stmt);
}

int DatabaseManager::GetColumnType(DatabaseManager::statement stmt, int iCol) {
  return sqlite3_column_type(stmt, iCol);
}

const char *DatabaseManager::GetColumnName(DatabaseManager::statement stmt,
                                           int iCol) {
  return sqlite3_column_name(stmt, iCol);
}

std::pair<DatabaseManager::columns, DatabaseManager::resultset>
DatabaseManager::QueryStmt(DatabaseManager::statement stmt) {
  DatabaseManager::columns cols;
  DatabaseManager::resultset rs;
  const int cols_count = GetStmtColumnsCount(stmt);
  int result_code = SQLITE_OK;
  for (int i = 0; i < cols_count; i++) {
    auto cName = GetColumnName(stmt, i);
    cols.push_back(std::string(cName));
  }
  do {
    result_code = sqlite3_step(stmt);
    LOG_DEBUG("step result %d", result_code);
    if (result_code == SQLITE_ROW) {
      DatabaseManager::result result;
      for (int i = 0; i < cols_count; i++) {
        DatabaseManager::resultvalue val;
        auto columnType = GetColumnType(stmt, i);
        auto columnName = GetColumnName(stmt, i);
        LOG_DEBUG("obtained col type %d to be pushed to resultset row",
                  columnType);
        switch (columnType) {
          case SQLITE_INTEGER:
            val = (int64_t)sqlite3_column_int64(stmt, i);
            LOG_DEBUG("obtained result for col %s and value %d", columnName,
                      std::get<int64_t>(val));
            result.push_back(val);
            break;
          case SQLITE_FLOAT:
            val = sqlite3_column_double(stmt, i);
            LOG_DEBUG("obtained result for col %s and value %d", columnName,
                      std::get<double>(val));
            result.push_back(val);
            break;
          case SQLITE_TEXT:
            val = std::string((const char *)sqlite3_column_text(stmt, i));
            LOG_DEBUG("obtained result for col %s and value %s", columnName,
                      std::get<std::string>(val).c_str());
            result.push_back(val);
            break;
          case SQLITE_BLOB: {
            LOG_DEBUG("obtained BLOB result for col %s", columnName);
            const uint8_t *blob =
                reinterpret_cast<const uint8_t *>(sqlite3_column_blob(stmt, i));
            std::vector<uint8_t> v(&blob[0],
                                   &blob[sqlite3_column_bytes(stmt, i)]);
            result.push_back(v);
            break;
          }
          case SQLITE_NULL:
            LOG_DEBUG("obtained NULL result for col %s", columnName);
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

std::pair<DatabaseManager::columns, DatabaseManager::resultset>
DatabaseManager::Query(std::string sql, DatabaseManager::parameters params) {
  LOG_DEBUG("preparing statement to execute sql: %s", sql.c_str());
  auto stmt = PrepareStmt(sql);
  BindStmtParams(stmt, params);
  return QueryStmt(stmt);
}

void DatabaseManager::FinalizeStmt(DatabaseManager::statement stmt) {
  LOG_DEBUG("finalizing prepared statement for sql");
  sqlite3_finalize(stmt);
}

void DatabaseManager::Execute(std::string sql,
                              DatabaseManager::parameters params) {
  LOG_DEBUG("preparing statement to execute sql: %s", sql.c_str());
  auto stmt = PrepareStmt(sql);
  BindStmtParams(stmt, params);
  ExecuteStmt(stmt);
}
