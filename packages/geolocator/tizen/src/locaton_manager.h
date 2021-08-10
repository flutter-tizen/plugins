// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LOCATON_MANAGER_H_
#define LOCATON_MANAGER_H_

#include "tizen_result.h"

class LocationManager {
 public:
  LocationManager();
  ~LocationManager();

  TizenResult IsLocationServiceEnabled(bool *is_enabled);
};
#endif  // LOCATON_MANAGER_H_
