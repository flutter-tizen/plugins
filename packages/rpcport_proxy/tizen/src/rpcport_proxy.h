// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGEPORT_H
#define MESSAGEPORT_H

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <message_port.h>

#include <map>
#include <set>

typedef std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> EventSink;

struct RpcportProxyResult {
  RpcportProxyResult() : error_code(MESSAGE_PORT_ERROR_NONE){};
  RpcportProxyResult(int code) : error_code(code) {}

  // Returns false on error
  operator bool() const { return MESSAGE_PORT_ERROR_NONE == error_code; }

  std::string message() { return get_error_message(error_code); }

  int error_code;
};

class RpcportProxyManager {
 public:
  RpcportProxyManager();
  ~RpcportProxyManager();

  RpcportProxyResult CheckRemotePort(std::string& remote_app_id,
                                    std::string& port_name, bool is_trusted,
                                    bool* result);
  RpcportProxyResult RegisterLocalPort(const std::string& port_name,
                                      EventSink sink, bool is_trusted,
                                      int* local_port);
  RpcportProxyResult UnregisterLocalPort(int local_port_id);
  RpcportProxyResult Send(std::string& remote_app_id, std::string& port_name,
                         flutter::EncodableValue& message, bool is_trusted);
  RpcportProxyResult Send(std::string& remote_app_id, std::string& port_name,
                         flutter::EncodableValue& message, bool is_trusted,
                         int local_port);

 private:
  static void OnMessageReceived(int local_port_id, const char* remote_app_id,
                                const char* remote_port,
                                bool trusted_remote_port, bundle* message,
                                void* user_data);

  RpcportProxyResult CreateResult(int return_code);
  RpcportProxyResult PrepareBundle(flutter::EncodableValue& message, bundle*& b);

  std::map<int, EventSink> sinks_;
  std::set<int> trusted_ports_;
};

#endif  // MESSAGEPORT_H
