// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_RPC_PORT_STUB_H_
#define FLUTTER_PLUGIN_RPC_PORT_STUB_H_

#include <flutter/encodable_value.h>
#include <flutter/event_sink.h>
#include <rpc-port.h>

#include <map>
#include <memory>
#include <string>

#include "rpc_port_result.h"

namespace tizen {

typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;

class RpcPortStubManager {
 public:
  static void Init(rpc_port_stub_h handle,
                   std::unique_ptr<FlEventSink> event_sink);

  static void Dispose(rpc_port_stub_h handle);

  static RpcPortResult Listen(rpc_port_stub_h handle);

 private:
  static void OnConnectedEvent(const char* sender, const char* instance,
                               void* user_data);
  static void OnDisconnectedEvent(const char* sender, const char* instance,
                                  void* user_data);
  static int OnReceivedEvent(const char* sender, const char* instance,
                             rpc_port_h port, void* user_data);

  static std::map<rpc_port_stub_h, std::unique_ptr<FlEventSink>> event_sinks_;
};

}  // namespace tizen

#endif  // FLUTTER_PLUGIN_RPC_PORT_STUB_H_
