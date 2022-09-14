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

class RpcPort {
 public:
  enum Type {
    Main,
    Callback,
  };

  RpcPort(rpc_port_h handle, int32_t type);

  RpcPortResult Read(std::vector<uint8_t>* data, int32_t size);
  RpcPortResult Write(const std::vector<uint8_t>& data);
  RpcPortResult SetPrivateSharing(const std::vector<std::string>& paths);
  RpcPortResult SetPrivateSharing(const std::string& path);
  RpcPortResult UnsetPrivateSharing();
  RpcPortResult PortDisconnect();

  int32_t GetType() const;
  rpc_port_h GetHandle() const;

 private:
  rpc_port_h handle_;
  int32_t type_;
};

class RpcPortParcel {
 public:
  RpcPortParcel();
  explicit RpcPortParcel(const std::vector<uint8_t>& raw);
  explicit RpcPortParcel(RpcPort* port);
  ~RpcPortParcel();

  RpcPortResult Send(RpcPort* port);
  RpcPortResult GetRaw(std::vector<uint8_t>* raw);
  rpc_port_parcel_h GetHandle() const;

 private:
  rpc_port_parcel_h handle_;
};

}  // namespace tizen

#endif  // COMMON_HH
