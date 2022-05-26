// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "permission_manager.h"

#ifndef TV_PROFILE
#include <Ecore.h>
#include <privacy_privilege_manager.h>
#include <tizen.h>
#endif
#include <tizen_error.h>

#include <string>

#include "log.h"

PermissionStatus PermissionManager::CheckPermission(
    const std::string &privilege) {
#ifdef TV_PROFILE
  return PermissionStatus::kAlways;
#else
  ppm_check_result_e result;
  int ret = ppm_check_permission(privilege.c_str(), &result);
  if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("Permission check failed [%s]: %s", privilege.c_str(),
              get_error_message(ret));
    return PermissionStatus::kError;
  }

  switch (result) {
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
      return PermissionStatus::kDenied;
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
    default:
      return PermissionStatus::kAlways;
  }
#endif
}

PermissionStatus PermissionManager::RequestPermission(
    const std::string &privilege) {
#ifdef TV_PROFILE
  return PermissionStatus::kAlways;
#else
  struct Response {
    bool received = false;
    ppm_call_cause_e cause;
    ppm_request_result_e result;
  } response;

  int ret = ppm_request_permission(
      privilege.c_str(),
      [](ppm_call_cause_e cause, ppm_request_result_e result,
         const char *privilege, void *user_data) {
        auto *response = static_cast<Response *>(user_data);
        response->received = true;
        response->cause = cause;
        response->result = result;
      },
      &response);

  if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("c[%s]: %s", privilege.c_str(), get_error_message(ret));
    return PermissionStatus::kError;
  }

  // Wait until ppm_request_permission() completes with a response.
  while (!response.received) {
    ecore_main_loop_iterate();
  }

  if (response.cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
    LOG_ERROR("Received an error response [%s].", privilege.c_str());
    return PermissionStatus::kError;
  }

  switch (response.result) {
    case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
      return PermissionStatus::kDeniedForever;
    case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
      return PermissionStatus::kDenied;
    case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
      return PermissionStatus::kAlways;
    default:
      return PermissionStatus::kError;
  }
#endif  // TV_PROFILE
}
