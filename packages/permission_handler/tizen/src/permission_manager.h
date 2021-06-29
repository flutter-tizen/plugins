#ifndef PERMISSION_MANAGER_H_
#define PERMISSION_MANAGER_H_

#include <privacy_privilege_manager.h>

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "type.h"

using OnPermissionChecked = std::function<void(PermissionStatus status)>;
using OnPermissionRequested = std::function<void(
    const std::map<PermissionGroup, PermissionStatus> &results)>;
using OnPermissionError =
    std::function<void(const std::string &code, const std::string &message)>;

class PermissionManager {
 public:
  PermissionManager();
  ~PermissionManager();

  void CheckPermissionStatus(PermissionGroup permission,
                             OnPermissionChecked success_callback,
                             OnPermissionError error_callback);
  void RequestPermissions(std::vector<PermissionGroup> permissions,
                          OnPermissionRequested success_callback,
                          OnPermissionError error_callback);
  inline std::map<PermissionGroup, PermissionStatus> &RequestResults() {
    return request_results_;
  }

 private:
  bool on_going_;
  std::map<PermissionGroup, PermissionStatus> request_results_;
};

#endif  // PERMISSION_MANAGER_H_
