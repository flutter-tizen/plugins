#include "database_manager.h"

#include <flutter/standard_method_codec.h>

#include "list"
#include "log.h"
#include "sqlite3.h"
#include "variant"

using std::string;

DatabaseManager::DatabaseManager(string aPath, int aId, bool aSingleInstance,
                                 int aLogLevel) {
  path = aPath;
  singleInstance = aSingleInstance;
  id = aId;
  logLevel = aLogLevel;
}
DatabaseManager::~DatabaseManager(){};

int DatabaseManager::init() {
  LOG_DEBUG("initializing database");
  int resultCode = DATABASE_STATUS_OK;
  resultCode = sqlite3_shutdown();
  if (resultCode != DATABASE_STATUS_OK) {
    return resultCode;
  }
  resultCode = sqlite3_config(SQLITE_CONFIG_URI, 1);
  if (resultCode != DATABASE_STATUS_OK) {
    return resultCode;
  }
  return sqlite3_initialize();
}

int DatabaseManager::open() {
  int resultCode = DATABASE_STATUS_OK;
  resultCode = init();
  if (resultCode != DATABASE_STATUS_OK) {
    return resultCode;
  }
  LOG_DEBUG("opening/creating read write database");
  return sqlite3_open_v2(path.c_str(), &sqliteDatabase,
                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
}

int DatabaseManager::openReadOnly() {
  int resultCode = DATABASE_STATUS_OK;
  resultCode = init();
  if (resultCode != DATABASE_STATUS_OK) {
    return resultCode;
  }
  LOG_DEBUG("open read only database");
  return sqlite3_open_v2(path.c_str(), &sqliteDatabase, SQLITE_READONLY, NULL);
}
const char *DatabaseManager::getErrorMsg() {
  return sqlite3_errmsg(sqliteDatabase);
}

int DatabaseManager::close() {
  LOG_DEBUG("close database");
  return sqlite3_close(sqliteDatabase);
}

int DatabaseManager::prepareStmt(DatabaseManager::statement stmt, string sql) {
  LOG_DEBUG("preparing statement to execute sql: %s", sql.c_str());
  return sqlite3_prepare_v2(sqliteDatabase, sql.c_str(), -1, &stmt, nullptr);
}

int DatabaseManager::bindStmtParams(DatabaseManager::statement stmt,
                                    DatabaseManager::parameters params) {
  int resultCode = DATABASE_STATUS_OK;
  const int paramsLength = params.size();
  LOG_DEBUG("received %d params to execute sql", paramsLength);
  for (int i = 0; i < paramsLength; i++) {
    auto param = params[i];
    switch (param.index()) {
      case 0: {
        LOG_DEBUG("binding null param");
        resultCode = sqlite3_bind_null(stmt, i + 1);
        break;
      }
      case 1: {
        auto val = std::get<bool>(param);
        LOG_DEBUG("binding bool param: %d", int(val));
        resultCode = sqlite3_bind_int(stmt, i + 1, int(val));
        break;
      }
      case 2: {
        auto val = std::get<int>(param);
        LOG_DEBUG("binding param: %d", val);
        resultCode = sqlite3_bind_int(stmt, i + 1, val);
        break;
      }
      case 3: {
        auto val = std::get<int>(param);
        LOG_DEBUG("binding param: %d", val);
        resultCode = sqlite3_bind_int64(stmt, i + 1, val);
        break;
      }
      case 4: {
        auto val = std::get<double>(param);
        LOG_DEBUG("binding param: %d", val);
        resultCode = sqlite3_bind_double(stmt, i + 1, val);
        break;
      }
      case 5: {
        auto val = std::get<string>(param);
        LOG_DEBUG("binding param: %s", val.c_str());
        resultCode = sqlite3_bind_text(stmt, i + 1, val.c_str(), val.size(),
                                       SQLITE_TRANSIENT);
        break;
      }
      default:
        return SQLITE_ERROR;
    }
  }
  return resultCode;
}

int DatabaseManager::executeStmt(DatabaseManager::statement stmt) {
  LOG_DEBUG("executing prepared statement");
  int resultCode = DATABASE_STATUS_OK;
  resultCode = sqlite3_step(stmt);
  while (resultCode == SQLITE_ROW) {
    resultCode = sqlite3_step(stmt);
  }
  return resultCode;
}

int DatabaseManager::getStmtColumnsCount(DatabaseManager::statement stmt) {
  return sqlite3_column_count(stmt);
}

int DatabaseManager::getColumnType(DatabaseManager::statement stmt, int iCol) {
  LOG_DEBUG("get column type for col %d", iCol);
  return sqlite3_column_type(stmt, iCol);
}

const char *DatabaseManager::getColumnName(DatabaseManager::statement stmt,
                                           int iCol) {
  LOG_DEBUG("get column name for col %d", iCol);
  return sqlite3_column_name(stmt, iCol);
}

int DatabaseManager::queryStmt(DatabaseManager::statement stmt,
                               DatabaseManager::columns &cols,
                               DatabaseManager::resultset &rs) {
  // LOG_DEBUG("querying prepared statement");
  //   LOG_DEBUG("Error??? %d", sqlite3_errcode(sqliteDatabase));
  //   LOG_DEBUG("Error??? %s", sqlite3_errmsg(sqliteDatabase));
  //   LOG_DEBUG("column type %d", cType);
  //   LOG_DEBUG("column name %s", cName);
  //   cols.push_back(string(cName));
  //   columns[i].name = string(cName);
  //   columns[i].type = cType;
  const int columnsCount = getStmtColumnsCount(stmt);
  LOG_DEBUG("columns count %d", columnsCount);
  int resultCode;
  for (int i = 0; i < columnsCount; i++) {
    auto cName = getColumnName(stmt, i);
    cols.push_back(string(cName));
  }
  do {
    resultCode = sqlite3_step(stmt);
    LOG_DEBUG("step result %d", resultCode);
    if (resultCode == SQLITE_ROW) {
      DatabaseManager::result result;
      for (int i = 0; i < columnsCount; i++) {
        DatabaseManager::resultvalue val;
        // SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, or
        // SQLITE_NULL
        auto columnType = getColumnType(stmt, i);
        auto columnName = getColumnName(stmt, i);
        LOG_DEBUG("obtained col type %d to be pushed to resultset row",
                  columnType);
        switch (columnType) {
          case SQLITE_INTEGER:
            val = (int)sqlite3_column_int64(stmt, i);
            LOG_DEBUG("obtained result for col %s and value %d", columnName,
                      std::get<int>(val));
            result.push_back(std::make_pair(string(columnName), val));
            break;
          case SQLITE_FLOAT:
            val = sqlite3_column_double(stmt, i);
            LOG_DEBUG("obtained result for col %s and value %d", columnName,
                      std::get<double>(val));
            result.push_back(std::make_pair(string(columnName), val));
          case SQLITE_TEXT:
            val = string((const char *)sqlite3_column_text(stmt, i));
            LOG_DEBUG("obtained result for col %s and value %s", columnName,
                      std::get<string>(val).c_str());
            result.push_back(std::make_pair(string(columnName), val));
            break;
          case SQLITE_BLOB: {
            auto data = (const char *)sqlite3_column_blob(stmt, i);
            auto dataLen = sqlite3_column_bytes(stmt, i);
            string newVal;
            newVal.assign(data, dataLen);
            val = newVal;
            LOG_DEBUG("obtained result for col %s and value %s", columnName,
                      std::get<string>(val).c_str());
            result.push_back(std::make_pair(string(columnName), val));
          }
          case SQLITE_NULL:
            LOG_DEBUG("obtained NULL result for col %s", columnName);
            val = nullptr;
            result.push_back(std::make_pair(string(columnName), val));
            break;
          default:
            break;
        }
      }
      rs.push_back(result);
    }
  } while (resultCode == SQLITE_ROW);
  if (resultCode != SQLITE_DONE) {
    LOG_DEBUG("error step");
  }
  return resultCode;
}

int DatabaseManager::query(string sql, DatabaseManager::parameters params,
                           DatabaseManager::columns &cols,
                           DatabaseManager::resultset &rs) {
  DatabaseManager::statement stmt;
  int resultCode = DATABASE_STATUS_OK;
  // resultCode = prepareStmt(stmt, sql);
  LOG_DEBUG("preparing statement to execute sql: %s", sql.c_str());
  resultCode =
      sqlite3_prepare_v2(sqliteDatabase, sql.c_str(), -1, &stmt, nullptr);
  if (resultCode != DATABASE_STATUS_OK) {
    return resultCode;
  }
  resultCode = bindStmtParams(stmt, params);
  if (resultCode != DATABASE_STATUS_OK) {
    // Finalize statement to avoid memory leaks.
    finalizeStmt(stmt);
    return resultCode;
  }
  resultCode = queryStmt(stmt, cols, rs);
  if (resultCode != SQLITE_DONE) {
    // Finalize statement to avoid memory leaks.
    finalizeStmt(stmt);
    return resultCode;
  }
  return finalizeStmt(stmt);
}

int DatabaseManager::finalizeStmt(DatabaseManager::statement stmt) {
  LOG_DEBUG("finalizing prepared statement for sql");
  return sqlite3_finalize(stmt);
}

sqlite3 *DatabaseManager::getWritableDatabase() { return sqliteDatabase; }

sqlite3 *DatabaseManager::getReadableDatabase() { return sqliteDatabase; }

int DatabaseManager::execute(string sql, DatabaseManager::parameters params) {
  DatabaseManager::statement stmt;
  int resultCode = DATABASE_STATUS_OK;
  // resultCode = prepareStmt(stmt, sql);
  LOG_DEBUG("preparing statement to execute sql: %s", sql.c_str());
  resultCode =
      sqlite3_prepare_v2(sqliteDatabase, sql.c_str(), -1, &stmt, nullptr);
  if (resultCode != DATABASE_STATUS_OK) {
    return resultCode;
  }
  resultCode = bindStmtParams(stmt, params);
  if (resultCode != DATABASE_STATUS_OK) {
    // Finalize statement to avoid memory leaks.
    finalizeStmt(stmt);
    return resultCode;
  }
  resultCode = executeStmt(stmt);
  if (resultCode != SQLITE_DONE) {
    LOG_DEBUG("execute statement failed with code %d", resultCode);
    // Finalize statement to avoid memory leaks.
    finalizeStmt(stmt);
    return resultCode;
  }
  return finalizeStmt(stmt);
}
