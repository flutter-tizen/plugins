// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "permission_manager.h"

#include <privacy_privilege_manager.h>

#include "log.h"

namespace {
constexpr char kPrivilegeLocation[] = "http://tizen.org/privilege/location";
}  // namespace

PermissionManager::PermissionManager() {}
PermissionManager::~PermissionManager() {}

TizenResult PermissionManager::CheckPermissionStatus(
    PermissionStatus* permission_status) {
  PermissionStatus status;
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
