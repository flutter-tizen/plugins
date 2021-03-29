// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CAMERA_PERMISSOIN_H_
#define FLUTTER_PLUGIN_CAMERA_PERMISSOIN_H_

#include <functional>

using OnSuccess = std::function<void()>;
using OnFailure =
    std::function<void(const std::string &code, const std::string &message)>;

enum class Permission {
  kCamera,
  kContentRead,
  kContentWrite,
  kExternalstorage,
  kMediastorage,
};

class PermissionManager {
 public:
  void RequestPermssion(Permission permission, OnSuccess on_success,
                        OnFailure on_failure);
};

#endif
