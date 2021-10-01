#include "string"

template <typename T>
class Operation {
  std::string getMethod();

  T getArgument(std::string key);

  SqlCommand getSqlCommand();

  bool getNoResult();

  // In batch, means ignoring the error
  bool getContinueOnError();

  // Only for execute command
  bool getInTransaction();
};