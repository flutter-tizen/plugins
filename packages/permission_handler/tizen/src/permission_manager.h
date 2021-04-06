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

  static void OnRequestPermissionsResponse(ppm_call_cause_e cause,
                                           const ppm_request_result_e *results,
                                           const char **privileges,
                                           size_t privileges_count,
                                           void *user_data);

  bool on_going_;
  OnPermissionRequested request_success_callback_;
  OnPermissionError request_error_callback_;
  std::map<int, int> request_results_;
};

#endif  // PERMISSION_MANAGER_H_
