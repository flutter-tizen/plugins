#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

const std::string METHOD_GET_PLATFORM_VERSION = "getPlatformVersion";
const std::string METHOD_GET_DATABASES_PATH = "getDatabasesPath";
const std::string METHOD_DEBUG = "debug";
const std::string METHOD_OPTIONS = "options";
const std::string METHOD_OPEN_DATABASE = "openDatabase";
const std::string METHOD_CLOSE_DATABASE = "closeDatabase";
const std::string METHOD_INSERT = "insert";
const std::string METHOD_EXECUTE = "execute";
const std::string METHOD_QUERY = "query";
const std::string METHOD_UPDATE = "update";
const std::string METHOD_BATCH = "batch";
const std::string METHOD_DELETE_DATABASE = "deleteDatabase";

const std::string PARAM_ID = "id";
const std::string PARAM_PATH = "path";
// when opening a database
const std::string PARAM_READ_ONLY = "readOnly";              // boolean
const std::string PARAM_SINGLE_INSTANCE = "singleInstance";  // boolean
const std::string PARAM_LOG_LEVEL = "logLevel";              // int
// true when entering, false when leaving, null otherwise
const std::string PARAM_IN_TRANSACTION = "inTransaction";
// Result when opening a database
const std::string PARAM_RECOVERED = "recovered";
// Result when opening a database
const std::string PARAM_RECOVERED_IN_TRANSACTION = "recoveredInTransaction";

const std::string PARAM_QUERY_AS_MAP_LIST = "queryAsMapList";  // boolean

const std::string PARAM_SQL = "sql";
const std::string PARAM_SQL_ARGUMENTS = "arguments";
const std::string PARAM_NO_RESULT = "noResult";
const std::string PARAM_CONTINUE_ON_ERROR = "continueOnError";

// debugMode
const std::string PARAM_CMD = "cmd";  // debugMode cmd: get/set
const std::string CMD_GET = "get";

// in batch
const std::string PARAM_OPERATIONS = "operations";
// in each operation
const std::string PARAM_METHOD = "method";

// Batch operation results
const std::string PARAM_RESULT = "result";
const std::string PARAM_ERROR = "error";  // map with code/message/data
const std::string PARAM_ERROR_CODE = "code";
const std::string PARAM_ERROR_MESSAGE = "message";
const std::string PARAM_ERROR_DATA = "data";

const std::string ERROR_DATABASE = "sqlite_error";            // code
const std::string ERROR_BAD_PARAM = "bad_param";              // internal only
const std::string ERROR_OPEN_FAILED = "open_failed";          // msg
const std::string ERROR_DATABASE_CLOSED = "database_closed";  // msg

// memory database path
const std::string MEMORY_DATABASE_PATH = ":memory:";

#endif  // CONSTANTS_H