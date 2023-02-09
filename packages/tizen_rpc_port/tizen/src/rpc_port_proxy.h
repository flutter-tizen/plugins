// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_RPC_PORT_PROXY_H_
#define FLUTTER_PLUGIN_RPC_PORT_PROXY_H_

#include <flutter/encodable_value.h>
#include <flutter/event_sink.h>
#include <rpc-port.h>

#include <memory>
#include <string>

#include "rpc_port_result.h"

namespace tizen {

typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;

class RpcPortProxyManager {
 public:
  static void Init(std::unique_ptr<FlEventSink> event_sink);

  static RpcPortResult Connect(rpc_port_proxy_h handle,
                               const std::string& appid,
                               const std::string& port_name);

 private:
  static void OnConnectedEvent(const char* receiver, const char* port_name,
                               rpc_port_h port, void* data);
  static void OnDisconnectedEvent(const char* receiver, const char* port_name,
                                  void* data);
  static void OnRejectedEvent(const char* receiver, const char* port_name,
                              void* data);
  static void OnReceivedEvent(const char* receiver, const char* port_name,
                              void* data);

  static std::unique_ptr<FlEventSink> event_sink_;
};

}  // namespace tizen

#endif  // FLUTTER_PLUGIN_RPC_PORT_PROXY_H_
