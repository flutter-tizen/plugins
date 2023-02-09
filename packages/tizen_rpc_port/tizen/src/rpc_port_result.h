// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_RPC_PORT_RESULT_H_
#define FLUTTER_PLUGIN_RPC_PORT_RESULT_H_

#include <rpc-port.h>
#include <tizen_error.h>

#include <string>

namespace tizen {

struct RpcPortResult {
  RpcPortResult() : error_code(RPC_PORT_ERROR_NONE) {}
  explicit RpcPortResult(int code) : error_code(code) {}

  operator bool() const { return error_code == RPC_PORT_ERROR_NONE; }

  std::string message() { return get_error_message(error_code); }

  int error_code;
};

}  // namespace tizen

#endif  // FLUTTER_PLUGIN_RPC_PORT_RESULT_H_
