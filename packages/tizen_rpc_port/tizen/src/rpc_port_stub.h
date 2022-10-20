// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RPC_PORT_STUB_H
#define RPC_PORT_STUB_H

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <rpc-port-parcel.h>
#include <rpc-port.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "common.h"

namespace tizen {

typedef std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> EventSink;

class RpcPortStubManager {
 public:
  static void Init(EventSink sync);

  static RpcPortResult Listen(rpc_port_stub_h handle);
  static RpcPortResult AddPrivilege(rpc_port_stub_h handle,
                                    const std::string& privilege);
  static RpcPortResult SetTrusted(rpc_port_stub_h handle, const bool trusted);

 private:
  static void OnConnectedEvent(const char* sender, const char* instance,
                               void* user_data);
  static void OnDisconnectedEvent(const char* sender, const char* instance,
                                  void* user_data);
  static int OnReceivedEvent(const char* sender, const char* instance,
                             rpc_port_h port, void* user_data);

 private:
  static EventSink event_sink_;
};

}  // namespace tizen

#endif  // RPC_PORT_STUB_H
