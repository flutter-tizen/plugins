// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rpc_port_proxy.hh"

#include <bundle.h>
#include <flutter/standard_message_codec.h>
#include <rpc-port-parcel.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "log.h"

namespace tizen {

namespace {

flutter::EncodableMap CreateEncodableMap(const char* event,
                                         const char* receiver,
                                         const char* port_name) {
  return {{flutter::EncodableValue("event"),
           flutter::EncodableValue(std::string(event))},
          {flutter::EncodableValue("receiver"),
           flutter::EncodableValue(std::string(receiver))},
          {flutter::EncodableValue("portName"),
           flutter::EncodableValue(std::string(port_name))}};
}

}  // namespace

RpcPortProxy::RpcPortProxy(std::string appid, std::string port_name)
    : appid_(std::move(appid)), port_name_(std::move(port_name)) {
  LOG_DEBUG("RpcPortProxy: %s/%s", appid_.c_str(), port_name_.c_str());
  int ret = rpc_port_proxy_create(&handle_);
  if (ret != RPC_PORT_ERROR_NONE)
    LOG_ERROR("rpc_port_proxy_create() is failed. error: %d", ret);
}

RpcPortProxy::~RpcPortProxy() {
  if (handle_ != nullptr) rpc_port_proxy_destroy(handle_);
}

RpcPortResult RpcPortProxy::Connect(EventSink sink) {
  LOG_DEBUG("Connect: %s/%s", appid_.c_str(), port_name_.c_str());
  event_sink_ = std::move(sink);
  int ret =
      rpc_port_proxy_add_connected_event_cb(handle_, OnConnectedEvent, this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_disconnected_event_cb(handle_, OnDisconnectedEvent,
                                                 this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_rejected_event_cb(handle_, OnRejectedEvent, this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_received_event_cb(handle_, OnReceivedEvent, this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_connect(handle_, appid_.c_str(), port_name_.c_str());
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  return CreateResult(RPC_PORT_ERROR_NONE);
}

RpcPortResult RpcPortProxy::ProxyConnectSync(EventSink sink) {
  LOG_DEBUG("ProxyConnectSync: %s/%s", appid_.c_str(), port_name_.c_str());
  event_sink_ = std::move(sink);
  int ret =
      rpc_port_proxy_add_connected_event_cb(handle_, OnConnectedEvent, this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_disconnected_event_cb(handle_, OnDisconnectedEvent,
                                                 this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_rejected_event_cb(handle_, OnRejectedEvent, this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = rpc_port_proxy_add_received_event_cb(handle_, OnReceivedEvent, this);
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret =
      rpc_port_proxy_connect_sync(handle_, appid_.c_str(), port_name_.c_str());
  if (ret != RPC_PORT_ERROR_NONE) {
    return CreateResult(ret);
  }

  return CreateResult(RPC_PORT_ERROR_NONE);
}

RpcPortResult RpcPortProxy::GetPort(int32_t type,
                                    std::unique_ptr<RpcPort>* port) {
  rpc_port_h port_native = nullptr;
  int ret = rpc_port_proxy_get_port(
      handle_, static_cast<rpc_port_port_type_e>(type), &port_native);
  if (ret != RPC_PORT_ERROR_NONE) return CreateResult(ret);

  port->reset(new RpcPort(port_native, type));
  return CreateResult(ret);
}

void RpcPortProxy::OnConnectedEvent(const char* receiver, const char* port_name,
                                    rpc_port_h port, void* data) {
  LOG_DEBUG("OnConnectedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || port == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<RpcPortProxy*>(data);
  auto map = CreateEncodableMap("connected", receiver, port_name);
  proxy->event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxy::OnDisconnectedEvent(const char* receiver,
                                       const char* port_name, void* data) {
  LOG_DEBUG("OnDisconnectedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || data == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<RpcPortProxy*>(data);
  auto map = CreateEncodableMap("disconnected", receiver, port_name);
  proxy->event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxy::OnRejectedEvent(const char* receiver, const char* port_name,
                                   void* data) {
  LOG_DEBUG("OnDisconnectedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || data == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<RpcPortProxy*>(data);
  auto map = CreateEncodableMap("rejected", receiver, port_name);
  proxy->event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortProxy::OnReceivedEvent(const char* receiver, const char* port_name,
                                   void* data) {
  LOG_DEBUG("OnReceivedEvent, receiver(%s), port_name(%s)", receiver,
            port_name);
  if (receiver == nullptr || port_name == nullptr || data == nullptr) {
    LOG_ERROR("Invalid parameter");
    return;
  }

  auto* proxy = static_cast<RpcPortProxy*>(data);
  rpc_port_h port = nullptr;
  int ret =
      rpc_port_proxy_get_port(proxy->handle_, RPC_PORT_PORT_CALLBACK, &port);
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

  auto map = CreateEncodableMap("received", receiver, port_name);
  map[flutter::EncodableValue("rawData")] =
      flutter::EncodableValue(std::move(raw_data));

  proxy->event_sink_->Success(std::move(flutter::EncodableValue(map)));
}

}  // namespace tizen
