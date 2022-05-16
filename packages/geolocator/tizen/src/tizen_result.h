// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_RESULT_H_
#define FLUTTER_PLUGIN_TIZEN_RESULT_H_

#include <tizen.h>

#include <string>

struct TizenResult {
  TizenResult() : error_code(TIZEN_ERROR_NONE){};
  TizenResult(int code) : error_code(code) {}

  // Returns false on error.
  operator bool() const { return (error_code == TIZEN_ERROR_NONE); }

  std::string message() { return get_error_message(error_code); }

  int error_code = TIZEN_ERROR_NONE;
};

#endif  // FLUTTER_PLUGIN_TIZEN_RESULT_H_
