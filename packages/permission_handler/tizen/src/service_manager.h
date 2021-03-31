#ifndef SERVICE_MANAGER_H_
#define SERVICE_MANAGER_H_

#include <functional>
#include <memory>

using OnServiceChecked = std::function<void(int status)>;
using OnServiceError =
    std::function<void(const std::string &code, const std::string &message)>;

class ServiceManager {
 public:
  ServiceManager();
  ~ServiceManager();

  void CheckServiceStatus(int permission, OnServiceChecked successCallback,
                          OnServiceError errorCallback);
};

#endif  // SERVICE_MANAGER_H_
