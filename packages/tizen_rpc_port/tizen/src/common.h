// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_HH
#define COMMON_HH

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <rpc-port-parcel.h>
#include <rpc-port.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace tizen {

typedef std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> EventSink;

struct RpcPortResult {
  RpcPortResult() : error_code(RPC_PORT_ERROR_NONE) {}
  explicit RpcPortResult(int code) : error_code(code) {}

  // Returns false on error
  operator bool() const { return error_code == RPC_PORT_ERROR_NONE; }

  const char* message() { return get_error_message(error_code); }

  int error_code;
};

RpcPortResult CreateResult(int return_code);

}  // namespace tizen

#endif  // COMMON_HH
