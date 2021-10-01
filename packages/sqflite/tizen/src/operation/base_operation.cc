#include "string"

class OperationResult {
  void error(std::string errorCode, std::string errorMessage, void* data);
  void success(void* result);
};