// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef RPC_PORT_PROXY_H
#define RPC_PORT_PROXY_H

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <rpc-port.h>
#include <rpc-port-parcel.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "common.h"

namespace tizen {

typedef std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> EventSink;

class RpcPortProxy {
 public:
  RpcPortProxy(std::string appid, std::string port_name);
  ~RpcPortProxy();
  RpcPortResult Connect(EventSink sync);
  RpcPortResult ProxyConnectSync(EventSink sync);
  RpcPortResult GetPort(int32_t type, std::unique_ptr<RpcPort>* port);

 private:
  static void OnConnectedEvent(const char* receiver, const char* port_name,
                               rpc_port_h port, void* data);
  static void OnDisconnectedEvent(const char* receiver, const char* port_name,
                                  void* data);
  static void OnRejectedEvent(const char* receiver, const char* port_name,
                              void* data);
  static void OnReceivedEvent(const char* receiver, const char* port_name,
                              void* data);

 private:
  std::string appid_;
  std::string port_name_;
  rpc_port_proxy_h handle_ = nullptr;
  EventSink event_sink_;
};

}  // namespace tizen

#endif  // RPC_PORT_PROXY_H