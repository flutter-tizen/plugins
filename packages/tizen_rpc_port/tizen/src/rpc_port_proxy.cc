// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rpc_port_proxy.h"

#include <bundle.h>
#include <flutter/standard_message_codec.h>
#include <rpc-port-parcel.h>
#include <unistd.h>

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

EventSink RpcPortProxyManager::event_sink_;

void RpcPortProxyManager::Init(EventSink sync) {
  event_sink_ = std::move(sync);
}

RpcPortResult RpcPortProxyManager::Connect(rpc_port_proxy_h handle,
                                           const std::string& appid,
                                           const std::string& port_name) {
  LOG_DEBUG("Connect: %p %s/%s", handle, appid.c_str(), port_name.c_str());
  int ret =
      rpc_port_proxy_add_connected_event_cb(handle, OnConnectedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_disconnected_event_cb(handle, OnDisconnectedEvent,
                                                 handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_rejected_event_cb(handle, OnRejectedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_received_event_cb(handle, OnReceivedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_connect(handle, appid.c_str(), port_name.c_str());
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  return CreateResult(RPC_PORT_ERROR_NONE);
}

void RpcPortProxyManager::OnConnectedEvent(const char* receiver,
                                           const char* port_name,
                                           rpc_port_h port, void* data) {
  LOG_DEBUG("OnConnectedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || port == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<rpc_port_proxy_h>(data);
  auto map = CreateEncodableMap("connected", receiver, port_name, proxy);
  event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxyManager::OnDisconnectedEvent(const char* receiver,
                                              const char* port_name,
                                              void* data) {
  LOG_DEBUG("OnDisconnectedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || data == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<rpc_port_proxy_h>(data);
  auto map = CreateEncodableMap("disconnected", receiver, port_name, proxy);
  event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxyManager::OnRejectedEvent(const char* receiver,
                                          const char* port_name, void* data) {
  LOG_DEBUG("OnDisconnectedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || data == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<rpc_port_proxy_h>(data);
  auto map = CreateEncodableMap("rejected", receiver, port_name, proxy);
  map[flutter::EncodableValue("error")] =
      flutter::EncodableValue(get_error_message(get_last_result()));
  event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxyManager::OnReceivedEvent(const char* receiver,
                                          const char* port_name, void* data) {
  LOG_DEBUG("OnReceivedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || data == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<rpc_port_proxy_h>(data);
  rpc_port_h port = nullptr;
  int ret = rpc_port_proxy_get_port(proxy, RPC_PORT_PORT_CALLBACK, &port);
  if (ret != RPC_PORT_ERROR_NONE) {
    LOG_ERROR("rpc_port_proxy_get_port() is failed. error(%d)", ret);
    return;
  }

  rpc_port_parcel_h parcel = nullptr;
  ret = rpc_port_parcel_create_from_port(&parcel, port);
  if (ret != RPC_PORT_ERROR_NONE) {
    LOG_ERROR("rpc_port_parcel_create_from_port() is failed. error(%d)", ret);
    return;
  }

  unsigned char* raw = nullptr;
  unsigned int size = 0;
  ret = rpc_port_parcel_get_raw(parcel, reinterpret_cast<void**>(&raw), &size);
  if (ret != RPC_PORT_ERROR_NONE) {
    LOG_ERROR("rpc_port_parcel_get_raw() is failed. error(%d)", ret);
    return;
  }

  LOG_INFO("size: %u", size);
  std::vector<uint8_t> raw_data(raw, raw + size);
  rpc_port_parcel_destroy(parcel);

  auto map = CreateEncodableMap("received", receiver, port_name, proxy);
  map[flutter::EncodableValue("rawData")] =
      flutter::EncodableValue(std::move(raw_data));

  event_sink_->Success(std::move(flutter::EncodableValue(map)));
}

}  // namespace tizen
