// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "permission_manager.h"

#include <Ecore.h>
#include <privacy_privilege_manager.h>

#include <vector>

#include "log.h"

namespace {

constexpr char kPrivilegeLocation[] = "http://tizen.org/privilege/location";
constexpr char kPrivilegeLocationCoarse[] =
    "http://tizen.org/privilege/location.coarse";

struct PermissionResponse {
  ppm_call_cause_e cause;
  ppm_request_result_e result;
  bool received = false;
};

}  // namespace

PermissionManager::PermissionManager() {}
PermissionManager::~PermissionManager() {}

TizenResult PermissionManager::CheckPermissionStatus(
    PermissionStatus *permission_status) {
  ppm_check_result_e check_result;

  int result = ppm_check_permission(kPrivilegeLocation, &check_result);
  if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
    return TizenResult(result);
  }

  switch (check_result) {
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
      *permission_status = PermissionStatus::kDenied;
      break;
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
    default:
      *permission_status = PermissionStatus::kAlways;
      break;
  }
  return TizenResult();
}

void PermissionManager::RequestPermssion(const OnSuccess &on_success,
                                         const OnFailure &on_failure) {
  std::vector<std::string> permissions = {kPrivilegeLocation,
                                          kPrivilegeLocationCoarse};
  std::vector<PermissionStatus> results;

  int ret = 0;
  for (size_t i = 0; i < permissions.size(); i++) {
    const char *permission = permissions[i].c_str();
    PermissionResponse response;
    ret = ppm_request_permission(
        permission,
        [](ppm_call_cause_e cause, ppm_request_result_e result,
           const char *privilege, void *data) {
          PermissionResponse *response =
              static_cast<PermissionResponse *>(data);
          response->cause = cause;
          response->result = result;
          response->received = true;
        },
        &response);
    if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      LOG_ERROR("Failed to call ppm_request_permission with [%s].", permission);
      break;
    }

    // Wait until ppm_request_permission is done.
    while (!response.received) {
      ecore_main_loop_iterate();
    }

    if (response.cause != PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ANSWER) {
      LOG_ERROR("permission[%s] request failed with an error.", permission);
      ret = response.cause;
      break;
    }

    switch (response.result) {
      case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
        results.push_back(PermissionStatus::kAlways);
        break;
      case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
        results.push_back(PermissionStatus::kDenied);
        break;
      case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
        results.push_back(PermissionStatus::kDeniedForever);
        break;
      default:
        LOG_ERROR("Unknown ppm_request_result_e.");
        break;
    }
  }

  if (ret || results.size() != permissions.size()) {
    on_failure(TizenResult(ret));
  } else {
    on_success(results[0]);
  }
}
