// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "permission_manager.h"

#ifndef TV_PROFILE
#include <privacy_privilege_manager.h>
#endif

#include "log.h"

#ifndef TV_PROFILE
const char* PermissionToString(Permission permission) {
  switch (permission) {
    case Permission::kCamera:
      return "http://tizen.org/privilege/camera";
    case Permission::kMediastorage:
      return "http://tizen.org/privilege/mediastorage";
    case Permission::kExternalstorage:
      return "http://tizen.org/privilege/externalstorage";
    case Permission::kContentRead:
      return "http://tizen.org/privilege/content.read";
    case Permission::kRecorder:
      return "http://tizen.org/privilege/recorder";
    case Permission::kContentWrite:
      return "http://tizen.org/privilege/content.write";
    default:
      LOG_WARN("Unknown permission!");
      return nullptr;
  }
}
#endif

void PermissionManager::RequestPermission(Permission permission,
                                          const OnSuccess& on_success,
                                          const OnFailure& on_failure) {
#ifdef TV_PROFILE
  on_success();
  return;
#else
  ppm_check_result_e result;
  const char* permission_string = PermissionToString(permission);
  int error = ppm_check_permission(permission_string, &result);

  if (error != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
    on_failure(get_error_message(error), "ppm_check_permission fail");
  } else {
    switch (result) {
      case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW: {
        on_success();
      } break;
      case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK: {
        struct Param {
          OnSuccess on_success;
          OnFailure on_failure;
        };
        Param* p = new Param;
        p->on_success = on_success;
        p->on_failure = on_failure;

        error = ppm_request_permission(
            permission_string,
            [](ppm_call_cause_e cause, ppm_request_result_e result,
               const char* privilege, void* data) {
              Param* p = (Param*)data;
              if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
                p->on_failure(get_error_message(cause),
                              "ppm_request_permission fail");
              } else {
                p->on_success();
              }
              delete p;
            },
            p);
        if (error != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
          on_failure(get_error_message(error), "ppm_request_permission fail");
          break;
        }
      } break;
      default:
        on_failure("unknwon", "RequestPermission fail");
        break;
    }
  }
  return;
#endif  // TV_PROFILE
}
