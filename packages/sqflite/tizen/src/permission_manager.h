// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_SQFLITE_PERMISSION_H_
#define FLUTTER_PLUGIN_SQFLITE_PERMISSION_H_

#include <functional>
#include <string>

enum class Permission {
  kMediastorage,
};

struct NotAllowedPermissionError : public std::runtime_error {
  NotAllowedPermissionError(std::string message, std::string code)
      : std::runtime_error(message + " (code " + code + ")") {}
};

class PermissionManager {
 public:
  using OnSuccess = std::function<void()>;
  using OnFailure =
      std::function<void(const std::string &message, const std::string &code)>;

  void RequestPermission(Permission permission, const OnSuccess &on_success,
                         const OnFailure &on_failure);
};

#endif
