// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CAMERA_PERMISSOIN_H_
#define FLUTTER_PLUGIN_CAMERA_PERMISSOIN_H_

#include <functional>

enum class Permission {
  kCamera,
  kContentRead,
  kContentWrite,
  kExternalstorage,
  kRecorder,
  kMediastorage,
};

class PermissionManager {
 public:
  using OnSuccess = std::function<void()>;
  using OnFailure =
      std::function<void(const std::string &code, const std::string &message)>;

  void RequestPermssion(Permission permission, const OnSuccess &on_success,
                        const OnFailure &on_failure);
};

#endif
