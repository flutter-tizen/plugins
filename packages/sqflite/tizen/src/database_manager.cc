#include "database_manager.h"

#include <flutter/standard_method_codec.h>

#include "list"
#include "log.h"
#include "sqlite3.h"
#include "variant"

DatabaseManager::DatabaseManager(std::string aPath, int aId,
                                 bool aSingleInstance, int aLogLevel) {
  sqliteDatabase = nullptr;
  if (aPath.size() == 0) {
    throw DatabaseError(-1, "empty database path");
  }
  path = aPath;
  singleInstance = aSingleInstance;
  id = aId;
  logLevel = aLogLevel;
}

DatabaseManager::~DatabaseManager() {
  for (auto &&stmt : stmtCache) {
    finalizeStmt(stmt.second);
    stmt.second = nullptr;
  }
  if (sqliteDatabase != nullptr) {
    close();
  }
}

void DatabaseManager::init() {
  LOG_DEBUG("initializing database");
  int resultCode = SQLITE_OK;
  resultCode = sqlite3_shutdown();
  if (resultCode != SQLITE_OK) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  };
  resultCode = sqlite3_config(SQLITE_CONFIG_URI, 1);
  if (resultCode != SQLITE_OK) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  }
  resultCode = sqlite3_initialize();
  if (resultCode != SQLITE_OK) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  }
}

void DatabaseManager::open() {
  init();
  LOG_DEBUG("opening/creating read write database");
  int resultCode =
      sqlite3_open_v2(path.c_str(), &sqliteDatabase,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (resultCode != SQLITE_OK) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  }
}

void DatabaseManager::openReadOnly() {
  init();
  LOG_DEBUG("open read only database");
  int resultCode = sqlite3_open_v2(path.c_str(), &sqliteDatabase,
                                   SQLITE_OPEN_READONLY, NULL);
  if (resultCode != SQLITE_OK) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  }
}
const char *DatabaseManager::getErrorMsg() {
  return sqlite3_errmsg(sqliteDatabase);
}

int DatabaseManager::getErrorCode() {
  return sqlite3_extended_errcode(sqliteDatabase);
}

void DatabaseManager::close() {
  LOG_DEBUG("closing database");
  int resultCode = sqlite3_close(sqliteDatabase);
  if (resultCode) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  }
}

void DatabaseManager::bindStmtParams(DatabaseManager::statement stmt,
                                     DatabaseManager::parameters params) {
  int err = SQLITE_OK;
  const int paramsLength = params.size();
  LOG_DEBUG("received %d params to execute sql", paramsLength);
  for (int i = 0; i < paramsLength; i++) {
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
      throw DatabaseError(getErrorCode(), getErrorMsg());
    }
  }
}

sqlite3_stmt *DatabaseManager::prepareStmt(std::string sql) {
  auto cacheEntry = stmtCache.find(sql);
  if (cacheEntry != stmtCache.end()) {
    sqlite3_stmt *stmt = cacheEntry->second;
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);
    return stmt;
  } else {
    sqlite3_stmt *stmt;
    int resultCode =
        sqlite3_prepare_v2(sqliteDatabase, sql.c_str(), -1, &stmt, nullptr);
    if (resultCode) {
      throw DatabaseError(getErrorCode(), getErrorMsg());
    }
    if (stmt != nullptr) {
      stmtCache[sql] = stmt;
    }
    return stmt;
  }
}

void DatabaseManager::executeStmt(DatabaseManager::statement stmt) {
  int resultCode = SQLITE_OK;
  do {
    resultCode = sqlite3_step(stmt);
  } while (resultCode == SQLITE_ROW);
  if (resultCode != SQLITE_DONE) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  }
}

int DatabaseManager::getStmtColumnsCount(DatabaseManager::statement stmt) {
  return sqlite3_column_count(stmt);
}

int DatabaseManager::getColumnType(DatabaseManager::statement stmt, int iCol) {
  return sqlite3_column_type(stmt, iCol);
}

const char *DatabaseManager::getColumnName(DatabaseManager::statement stmt,
                                           int iCol) {
  return sqlite3_column_name(stmt, iCol);
}

std::pair<DatabaseManager::columns, DatabaseManager::resultset>
DatabaseManager::queryStmt(DatabaseManager::statement stmt) {
  DatabaseManager::columns cols;
  DatabaseManager::resultset rs;
  const int columnsCount = getStmtColumnsCount(stmt);
  int resultCode = SQLITE_OK;
  for (int i = 0; i < columnsCount; i++) {
    auto cName = getColumnName(stmt, i);
    cols.push_back(std::string(cName));
  }
  do {
    resultCode = sqlite3_step(stmt);
    LOG_DEBUG("step result %d", resultCode);
    if (resultCode == SQLITE_ROW) {
      DatabaseManager::result result;
      for (int i = 0; i < columnsCount; i++) {
        DatabaseManager::resultvalue val;
        auto columnType = getColumnType(stmt, i);
        auto columnName = getColumnName(stmt, i);
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
  } while (resultCode == SQLITE_ROW);
  if (resultCode != SQLITE_DONE) {
    throw DatabaseError(getErrorCode(), getErrorMsg());
  }
  return std::make_pair(cols, rs);
}

std::pair<DatabaseManager::columns, DatabaseManager::resultset>
DatabaseManager::query(std::string sql, DatabaseManager::parameters params) {
  LOG_DEBUG("preparing statement to execute sql: %s", sql.c_str());
  auto stmt = prepareStmt(sql);
  bindStmtParams(stmt, params);
  return queryStmt(stmt);
}

void DatabaseManager::finalizeStmt(DatabaseManager::statement stmt) {
  LOG_DEBUG("finalizing prepared statement for sql");
  sqlite3_finalize(stmt);
}

sqlite3 *DatabaseManager::getWritableDatabase() { return sqliteDatabase; }

sqlite3 *DatabaseManager::getReadableDatabase() { return sqliteDatabase; }

void DatabaseManager::execute(std::string sql,
                              DatabaseManager::parameters params) {
  LOG_DEBUG("preparing statement to execute sql: %s", sql.c_str());
  auto stmt = prepareStmt(sql);
  bindStmtParams(stmt, params);
  executeStmt(stmt);
}
