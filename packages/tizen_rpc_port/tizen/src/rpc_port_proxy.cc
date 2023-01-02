// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rpc_port_proxy.h"

#include <rpc-port-parcel.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "log.h"

namespace tizen {

namespace {

flutter::EncodableMap CreateEncodableMap(const char* event,
                                         const char* receiver,
                                         const char* port_name,
                                         rpc_port_proxy_h handle) {
  return {{flutter::EncodableValue("event"),
           flutter::EncodableValue(std::string(event))},
          {flutter::EncodableValue("receiver"),
           flutter::EncodableValue(std::string(receiver))},
          {flutter::EncodableValue("portName"),
           flutter::EncodableValue(std::string(port_name))},
          {flutter::EncodableValue("handle"),
           flutter::EncodableValue(reinterpret_cast<int64_t>(handle))}};
}

}  // namespace

std::unique_ptr<FlEventSink> RpcPortProxyManager::event_sink_;

void RpcPortProxyManager::Init(std::unique_ptr<FlEventSink> event_sink) {
  event_sink_ = std::move(event_sink);
}

RpcPortResult RpcPortProxyManager::Connect(rpc_port_proxy_h handle,
                                           const std::string& appid,
                                           const std::string& port_name) {
  LOG_DEBUG("Connect: %s/%s", appid.c_str(), port_name.c_str());

  int ret =
      rpc_port_proxy_add_connected_event_cb(handle, OnConnectedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return RpcPortResult(ret);
  }
  ret = rpc_port_proxy_add_disconnected_event_cb(handle, OnDisconnectedEvent,
                                                 handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return RpcPortResult(ret);
  }
  ret = rpc_port_proxy_add_rejected_event_cb(handle, OnRejectedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return RpcPortResult(ret);
  }
  ret = rpc_port_proxy_add_received_event_cb(handle, OnReceivedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return RpcPortResult(ret);
  }

  ret = rpc_port_proxy_connect(handle, appid.c_str(), port_name.c_str());
  return RpcPortResult(ret);
}

void RpcPortProxyManager::OnConnectedEvent(const char* receiver,
                                           const char* port_name,
                                           rpc_port_h port, void* user_data) {
  LOG_DEBUG("Connected: receiver(%s), portName(%s)", receiver, port_name);

  rpc_port_proxy_h handle = static_cast<rpc_port_proxy_h>(user_data);
  flutter::EncodableMap map =
      CreateEncodableMap("connected", receiver, port_name, handle);
  event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxyManager::OnDisconnectedEvent(const char* receiver,
                                              const char* port_name,
                                              void* user_data) {
  LOG_DEBUG("Disconnected: receiver(%s), portName(%s)", receiver, port_name);

  rpc_port_proxy_h handle = static_cast<rpc_port_proxy_h>(user_data);
  flutter::EncodableMap map =
      CreateEncodableMap("disconnected", receiver, port_name, handle);
  event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxyManager::OnRejectedEvent(const char* receiver,
                                          const char* port_name,
                                          void* user_data) {
  LOG_DEBUG("Rejected: receiver(%s), portName(%s)", receiver, port_name);

  rpc_port_proxy_h handle = static_cast<rpc_port_proxy_h>(user_data);
  flutter::EncodableMap map =
      CreateEncodableMap("rejected", receiver, port_name, handle);
  map[flutter::EncodableValue("error")] = flutter::EncodableValue(
      std::string(get_error_message(get_last_result())));
  event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxyManager::OnReceivedEvent(const char* receiver,
                                          const char* port_name,
                                          void* user_data) {
  LOG_DEBUG("Received: receiver(%s), portName(%s)", receiver, port_name);

  rpc_port_proxy_h handle = static_cast<rpc_port_proxy_h>(user_data);
  rpc_port_h port = nullptr;
  int ret = rpc_port_proxy_get_port(handle, RPC_PORT_PORT_CALLBACK, &port);
  if (ret != RPC_PORT_ERROR_NONE) {
    event_sink_->Error("rpc_port_proxy_get_port failed",
                       std::string(get_error_message(ret)));
    return;
  }

  rpc_port_parcel_h parcel = nullptr;
  ret = rpc_port_parcel_create_from_port(&parcel, port);
  if (ret != RPC_PORT_ERROR_NONE) {
    event_sink_->Error("rpc_port_parcel_create_from_port failed",
                       std::string(get_error_message(ret)));
    return;
  }

  unsigned char* raw = nullptr;
  unsigned int size = 0;
  ret = rpc_port_parcel_get_raw(parcel, reinterpret_cast<void**>(&raw), &size);
  if (ret != RPC_PORT_ERROR_NONE) {
    rpc_port_parcel_destroy(parcel);
    event_sink_->Error("rpc_port_parcel_get_raw failed",
                       std::string(get_error_message(ret)));
    return;
  }
  std::vector<uint8_t> raw_data(raw, raw + size);
  rpc_port_parcel_destroy(parcel);

  flutter::EncodableMap map =
      CreateEncodableMap("received", receiver, port_name, handle);
  map[flutter::EncodableValue("rawData")] =
      flutter::EncodableValue(std::move(raw_data));
  event_sink_->Success(flutter::EncodableValue(map));
}

}  // namespace tizen
