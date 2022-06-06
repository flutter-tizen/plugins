// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_SERVICE_MANAGER_H_
#define FLUTTER_PLUGIN_SERVICE_MANAGER_H_

#include "permissions.h"

// The status of a service associated with a specific permission.
//
// Originally defined in service_status.dart of the platform interface pacakge.
enum class ServiceStatus {
  kDisabled = 0,
  kEnabled = 1,
  kNotApplicable = 2,
};

class ServiceManager {
 public:
  ServiceManager() {}
  ~ServiceManager() {}

  ServiceStatus CheckServiceStatus(Permission permission);
};

#endif  // FLUTTER_PLUGIN_SERVICE_MANAGER_H_
