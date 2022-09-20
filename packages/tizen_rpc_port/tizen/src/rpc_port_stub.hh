// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RPC_PORT_STUB_HH
#define RPC_PORT_STUB_HH

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <rpc-port-parcel.h>
#include <rpc-port.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "common.hh"

namespace tizen {

typedef std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> EventSink;

class RpcPortStub {
 public:
  explicit RpcPortStub(std::string port_name);
  ~RpcPortStub();

  RpcPortResult Listen(EventSink sink);
  RpcPortResult AddPrivilege(const std::string& privilege);
  RpcPortResult SetTrusted(const bool trusted);
  RpcPortResult GetPort(int32_t type, const std::string& instance,
                        std::unique_ptr<RpcPort>* port);

 private:
  static void OnConnectedEvent(const char* sender, const char* instance,
                               void* user_data);
  static void OnDisconnectedEvent(const char* sender, const char* instance,
                                  void* user_data);
  static int OnReceivedEvent(const char* sender, const char* instance,
                             rpc_port_h port, void* user_data);

 private:
  std::string port_name_;
  rpc_port_stub_h handle_ = nullptr;
  EventSink event_sink_;
};

}  // namespace tizen

#endif  // RPC_PORT_STUB_HH
