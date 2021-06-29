#ifndef SERVICE_MANAGER_H_
#define SERVICE_MANAGER_H_

#include <functional>
#include <memory>

#include "type.h"

using OnServiceChecked = std::function<void(ServiceStatus status)>;
using OnServiceError =
    std::function<void(const std::string &code, const std::string &message)>;

class ServiceManager {
 public:
  ServiceManager();
  ~ServiceManager();

  void CheckServiceStatus(PermissionGroup permission,
                          OnServiceChecked success_callback,
                          OnServiceError error_callback);
};

#endif  // SERVICE_MANAGER_H_
