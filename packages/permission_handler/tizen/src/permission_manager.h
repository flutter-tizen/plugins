#ifndef PERMISSION_MANAGER_H_
#define PERMISSION_MANAGER_H_

#include <privacy_privilege_manager.h>

#include <functional>
#include <map>
#include <memory>
#include <vector>

using OnPermissionChecked = std::function<void(int status)>;
using OnPermissionRequested =
    std::function<void(const std::map<int, int> &results)>;
using OnPermissionError =
    std::function<void(const std::string &code, const std::string &message)>;

class PermissionManager {
 public:
  PermissionManager();
  ~PermissionManager();

  void CheckPermissionStatus(int permission,
                             OnPermissionChecked success_callback,
                             OnPermissionError error_callback);
  void RequestPermissions(std::vector<int> permissions,
                          OnPermissionRequested success_callback,
                          OnPermissionError error_callback);

 private:
  int ConvertToPermission(const std::string &privilege);
  void ConvertToPrivileges(int permission,
                           std::vector<const char *> &privileges);
  int DeterminePermissionStatus(int permission, int *status);

  bool on_going_;
  std::map<int, int> request_results_;
};

#endif  // PERMISSION_MANAGER_H_
