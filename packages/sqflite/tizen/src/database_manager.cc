#include "database_manager.h"

#include <flutter/standard_method_codec.h>
#include <sqlite3.h>

#include <list>
#include <variant>

#include "errors.h"
#include "log.h"
#include "log_level.h"

namespace sqflite_database {

DatabaseManager::~DatabaseManager() {
  for (auto &&statement : statement_cache_) {
    FinalizeStmt(statement.second);
    statement.second = nullptr;
  }

  Close(true);
}

void DatabaseManager::ThrowCurrentDatabaseError() {
  throw sqflite_errors::DatabaseError(GetErrorCode(), GetErrorMsg());
}

void DatabaseManager::Open() {
  int result_code =
      sqlite3_open_v2(path_.c_str(), &database_,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (result_code != SQLITE_OK) {
    Close(false);
    ThrowCurrentDatabaseError();
  }
  result_code = sqlite3_busy_timeout(database_, kBusyTimeoutMs);
  if (result_code != SQLITE_OK) {
    Close(false);
    ThrowCurrentDatabaseError();
  }
}

void DatabaseManager::OpenReadOnly() {
  int result_code =
      sqlite3_open_v2(path_.c_str(), &database_, SQLITE_OPEN_READONLY, NULL);
  if (result_code != SQLITE_OK) {
    Close(false);
    ThrowCurrentDatabaseError();
  }
  result_code = sqlite3_busy_timeout(database_, kBusyTimeoutMs);
  if (result_code != SQLITE_OK) {
    Close(false);
    ThrowCurrentDatabaseError();
  }
}

const char *DatabaseManager::GetErrorMsg() { return sqlite3_errmsg(database_); }

int DatabaseManager::GetErrorCode() {
  return sqlite3_extended_errcode(database_);
}

void DatabaseManager::Close(bool raise_error) {
  int result_code = sqlite3_close_v2(database_);
  database_ = nullptr;
  if (result_code != SQLITE_OK && raise_error) {
    ThrowCurrentDatabaseError();
  }
}

void DatabaseManager::BindStmtParams(DatabaseManager::Statement statement,
                                     SQLParameters parameters) {
  int result_code = SQLITE_OK;
  const int parameters_length = parameters.size();
  for (int i = 0; i < parameters_length; i++) {
    auto idx = i + 1;
    auto parameter = parameters[i];
    switch (parameter.index()) {
      case 0: {
        result_code = sqlite3_bind_null(statement, idx);
        break;
      }
      case 1: {
        auto value = std::get<bool>(parameter);
        result_code = sqlite3_bind_int(statement, idx, int(value));
        break;
      }
      case 2: {
        auto value = std::get<int32_t>(parameter);
        result_code = sqlite3_bind_int(statement, idx, value);
        break;
      }
      case 3: {
        auto value = std::get<int64_t>(parameter);
        result_code = sqlite3_bind_int64(statement, idx, value);
        break;
      }
      case 4: {
        auto value = std::get<double>(parameter);
        result_code = sqlite3_bind_double(statement, idx, value);
        break;
      }
      case 5: {
        auto value = std::get<std::string>(parameter);
        result_code = sqlite3_bind_text(statement, idx, value.c_str(),
                                        value.size(), SQLITE_TRANSIENT);
        break;
      }
      case 6: {
        auto vector = std::get<std::vector<uint8_t>>(parameter);
        result_code = sqlite3_bind_blob(statement, idx, vector.data(),
                                        (int)vector.size(), SQLITE_TRANSIENT);
        break;
      }
      case 7: {
        auto vector = std::get<std::vector<int32_t>>(parameter);
        result_code = sqlite3_bind_blob(statement, idx, vector.data(),
                                        (int)vector.size(), SQLITE_TRANSIENT);
        break;
      }
      case 8: {
        auto vector = std::get<std::vector<int64_t>>(parameter);
        result_code = sqlite3_bind_blob(statement, idx, vector.data(),
                                        (int)vector.size(), SQLITE_TRANSIENT);
        break;
      }
      case 9: {
        auto vector = std::get<std::vector<double>>(parameter);
        result_code = sqlite3_bind_blob(statement, idx, vector.data(),
                                        (int)vector.size(), SQLITE_TRANSIENT);
        break;
      }
      case 10: {
        auto value = std::get<flutter::EncodableList>(parameter);
        std::vector<uint8_t> vector;
        // Only  a list of uint8_t for flutter EncodableValue is supported
        // to store it as a BLOB, otherwise a DatabaseError is triggered
        try {
          for (auto item : value) {
            vector.push_back(std::get<int>(item));
          }
        } catch (const std::bad_variant_access) {
          throw sqflite_errors::DatabaseError(
              sqflite_errors::kUnknownErrorCode,
              "statement parameter is not supported");
        }
        result_code = sqlite3_bind_blob(statement, idx, vector.data(),
                                        (int)vector.size(), SQLITE_TRANSIENT);
        break;
      }
      default: {
        throw sqflite_errors::DatabaseError(
            sqflite_errors::kUnknownErrorCode,
            "statement parameter is not supported");
      }
    }
    if (result_code != SQLITE_OK) {
      ThrowCurrentDatabaseError();
    }
  }
}

DatabaseManager::Statement DatabaseManager::PrepareStmt(std::string sql) {
  auto cache_entry = statement_cache_.find(sql);
  if (cache_entry != statement_cache_.end()) {
    DatabaseManager::Statement statement = cache_entry->second;
    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);
    return statement;
  } else {
    DatabaseManager::Statement statement;
    int result_code =
        sqlite3_prepare_v2(database_, sql.c_str(), -1, &statement, nullptr);
    if (result_code) {
      FinalizeStmt(statement);
      ThrowCurrentDatabaseError();
    }
    if (statement != nullptr) {
      statement_cache_[sql] = statement;
    }
    return statement;
  }
}

void DatabaseManager::ExecuteStmt(DatabaseManager::Statement statement) {
  int result_code = SQLITE_OK;
  do {
    result_code = sqlite3_step(statement);
  } while (result_code == SQLITE_ROW);
  if (result_code != SQLITE_DONE) {
    ThrowCurrentDatabaseError();
  }
}

int DatabaseManager::GetStmtColumnsCount(DatabaseManager::Statement statement) {
  return sqlite3_column_count(statement);
}

int DatabaseManager::GetColumnType(DatabaseManager::Statement statement,
                                   int column_index) {
  return sqlite3_column_type(statement, column_index);
}

const char *DatabaseManager::GetColumnName(DatabaseManager::Statement statement,
                                           int column_index) {
  return sqlite3_column_name(statement, column_index);
}

std::pair<Columns, Resultset> DatabaseManager::QueryStmt(
    DatabaseManager::Statement statement) {
  Columns columns;
  Resultset resultset;
  const int columns_count = GetStmtColumnsCount(statement);
  int result_code = SQLITE_OK;
  for (int i = 0; i < columns_count; i++) {
    auto column_name = GetColumnName(statement, i);
    columns.push_back(std::string(column_name));
  }
  do {
    result_code = sqlite3_step(statement);
    if (result_code == SQLITE_ROW) {
      Result result;
      for (int i = 0; i < columns_count; i++) {
        ResultValue value;
        auto column_type = GetColumnType(statement, i);
        switch (column_type) {
          case SQLITE_INTEGER:
            value = (int64_t)sqlite3_column_int64(statement, i);
            result.push_back(value);
            break;
          case SQLITE_FLOAT:
            value = sqlite3_column_double(statement, i);
            result.push_back(value);
            break;
          case SQLITE_TEXT:
            value =
                std::string((const char *)sqlite3_column_text(statement, i));
            result.push_back(value);
            break;
          case SQLITE_BLOB: {
            const uint8_t *blob = reinterpret_cast<const uint8_t *>(
                sqlite3_column_blob(statement, i));
            std::vector<uint8_t> v(&blob[0],
                                   &blob[sqlite3_column_bytes(statement, i)]);
            result.push_back(v);
            break;
          }
          case SQLITE_NULL:
            value = nullptr;
            result.push_back(value);
            break;
          default:
            break;
        }
      }
      resultset.push_back(result);
    }
  } while (result_code == SQLITE_ROW);
  if (result_code != SQLITE_DONE) {
    ThrowCurrentDatabaseError();
  }
  return std::make_pair(columns, resultset);
}

void DatabaseManager::FinalizeStmt(DatabaseManager::Statement statement) {
  sqlite3_finalize(statement);
}

void DatabaseManager::LogQuery(Statement statement) {
  LOG_DEBUG("%s", sqlite3_expanded_sql(statement));
}

std::pair<Columns, Resultset> DatabaseManager::Query(std::string sql,
                                                     SQLParameters parameters) {
  auto statement = PrepareStmt(sql);
  BindStmtParams(statement, parameters);
  if (sqflite_log_level::HasSqlLevel(log_level_)) {
    LogQuery(statement);
  }
  return QueryStmt(statement);
}

void DatabaseManager::Execute(std::string sql, SQLParameters parameters) {
  Statement statement = PrepareStmt(sql);
  BindStmtParams(statement, parameters);
  if (sqflite_log_level::HasSqlLevel(log_level_)) {
    LogQuery(statement);
  }
  ExecuteStmt(statement);
}
}  // namespace sqflite_database
