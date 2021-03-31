#ifndef PERMISSION_MANAGER_H_
#define PERMISSION_MANAGER_H_

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <privacy_privilege_manager.h>

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
                             OnPermissionChecked successCallback,
                             OnPermissionError errorCallback);
  void RequestPermissions(std::vector<int> permissions,
                          OnPermissionRequested successCallback,
                          OnPermissionError errorCallback);

 private:
  int ParsePermissionName(const std::string &permission);
  void GetPermissionName(int permission,
                         std::vector<const char *> &permissionNames);
  int DeterminePermissionStatus(int permission, int *status);

  static void OnRequestPermissionsResponse(ppm_call_cause_e cause,
                                           const ppm_request_result_e *results,
                                           const char **privileges,
                                           size_t privileges_count,
                                           void *user_data);

  bool _ongoing;
  OnPermissionRequested _requestSuccessCallback;
  OnPermissionError _requestErrorCallback;
  std::map<int, int> _requestResults;
};

#endif  // PERMISSION_MANAGER_H_
