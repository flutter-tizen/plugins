#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

const std::string kPluginKey = "com.tekartik.sqflite";
const std::string kMethodGetDatabasesPath = "getDatabasesPath";
const std::string kMethodDebug = "debug";
const std::string kMethodOptions = "options";
const std::string kMethodOpenDatabase = "openDatabase";
const std::string kMethodCloseDatabase = "closeDatabase";
const std::string kMethodInsert = "insert";
const std::string kMethodExecute = "execute";
const std::string kMethodQuery = "query";
const std::string kMethodUpdate = "update";
const std::string kMethodBatch = "batch";
const std::string kMethodDeleteDatabase = "deleteDatabase";

const std::string kParamId = "id";
const std::string kParamPath = "path";
// when opening a database
const std::string kParamReadOnly = "readOnly";              // boolean
const std::string kParamSingleInstance = "singleInstance";  // boolean
const std::string kParamLogLevel = "logLevel";              // int
// true when entering, false when leaving, null otherwise
const std::string kParamInTransaction = "inTransaction";
// Result when opening a database
const std::string kParamRecovered = "recovered";
// Result when opening a database
const std::string kParamRecoveredInTransaction = "recoveredInTransaction";

const std::string kParamQueryAsMapList = "queryAsMapList";  // boolean

const std::string kParamSql = "sql";
const std::string kParamSqlArguments = "arguments";
const std::string kParamNoResult = "noResult";
const std::string kParamContinueOnError = "continueOnError";
const std::string kParamColumns = "columns";
const std::string kParamRows = "rows";
const std::string kParamDatabases = "databases";

// debugMode
const std::string kParamCmd = "cmd";  // debugMode cmd: get/set
const std::string kCmdGet = "get";

// in batch
const std::string kParamOperations = "operations";
// in each operation
const std::string kParamMethod = "method";

// Batch operation results
const std::string kParamResult = "result";
const std::string kParamError = "error";  // map with code/message/data
const std::string kParamErrorCode = "code";
const std::string kParamErrorMessage = "message";
const std::string kParamErrorData = "data";

const std::string kErrorDatabase = "sqlite_error";           // code
const std::string kErrorBadParam = "bad_param";              // internal only
const std::string kErrorOpenFailed = "open_failed";          // msg
const std::string kErrorDatabaseClosed = "database_closed";  // msg

// memory database path
const std::string kMemoryDatabasePath = ":memory:";

#endif  // CONSTANTS_H
