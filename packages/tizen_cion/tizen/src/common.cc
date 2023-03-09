// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common.h"

#include "log.h"

namespace tizen {

CionResult CreateResult(int return_code) {
  CionResult result(return_code);
  if (!result) {
    LOG_ERROR("Failed: %s", result.message());
  }

  return result;
}

}  // namespace tizen
