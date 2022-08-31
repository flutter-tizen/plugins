// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rpc_port_stub.hh"

#include <rpc-port-parcel.h>

#include <flutter/standard_method_codec.h>

#include <cstdint>

#include "log.h"

namespace tizen {
namespace {

flutter::EncodableMap CreateEncodableMap(const char* event, const char* sender,
    const char* instance) {
  return {
    { flutter::EncodableValue("event"),
      flutter::EncodableValue(std::string(event)) },
    { flutter::EncodableValue("sender"),
      flutter::EncodableValue(std::string(sender)) },
    { flutter::EncodableValue("instance"),
      flutter::EncodableValue(std::string(instance)) }
  };
}

}  // namespace

RpcPortStub::RpcPortStub(std::string port_name)
    : port_name_(std::move(port_name)) {
  LOG_DEBUG("RpcPortStub: %s", port_name_.c_str());
  int ret = rpc_port_stub_create(&handle_, port_name_.c_str());
  if (ret != RPC_PORT_ERROR_NONE)
    LOG_ERROR("rpc_port_stub_create() is failed. error: %d", ret);
}

RpcPortStub::~RpcPortStub() {
  if (handle_ != nullptr)
    rpc_port_stub_destroy(handle_);
}

RpcPortResult RpcPortStub::Listen(EventSink sink) {
  LOG_DEBUG("Listen: %s", port_name_.c_str());
  event_sink_ = std::move(sink);
  rpc_port_stub_add_connected_event_cb(handle_, OnConnectedEvent, this);
  rpc_port_stub_add_disconnected_event_cb(handle_, OnDisconnectedEvent, this);
  rpc_port_stub_add_received_event_cb(handle_, OnReceivedEvent, this);

  int ret = rpc_port_stub_listen(handle_);
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_stub_listen() is failed. error: %s",
        result.message());
  }

  return result;
}

RpcPortResult RpcPortStub::AddPrivilege(const std::string& privilege) {
  LOG_DEBUG("AddPrivilege: %s, privilege: %s",
      port_name_.c_str(), privilege.c_str());
  int ret = rpc_port_stub_add_privilege(handle_, privilege.c_str());
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_stub_add_privilege() is failed. error: %s",
        result.message());
  }

  return result;
}

RpcPortResult RpcPortStub::SetTrusted(const bool trusted) {
  LOG_DEBUG("SetTrusted: %s, trusted: %s",
      port_name_.c_str(), trusted ? "true" : "false");
  int ret = rpc_port_stub_set_trusted(handle_, trusted);
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_stub_set_trusted() is failed. error: %s",
        result.message());
  }

  return result;
}

RpcPortResult RpcPortStub::GetPort(int32_t type, const std::string& instance,
    RpcPort** port) {
  LOG_DEBUG("GetPort: %s", port_name_.c_str());
  rpc_port_h h = nullptr;
  int ret = rpc_port_stub_get_port(handle_,
      static_cast<rpc_port_port_type_e>(type), instance.c_str(), &h);
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_stub_get_port() is failed. error: %s",
        result.message());
    return result;
  }

  *port = new RpcPort(h, type);
  return result;
}

void RpcPortStub::OnConnectedEvent(const char* sender, const char* instance,
    void* user_data) {
  LOG_DEBUG("OnConnectedEvent. sender: %s, instance: %s", sender, instance);
  auto map = CreateEncodableMap("connected", sender, instance);
  auto* stub = static_cast<RpcPortStub*>(user_data);
  auto& sink = stub->event_sink_;
  sink->Success(std::move(flutter::EncodableValue(map)));
}

void RpcPortStub::OnDisconnectedEvent(const char* sender, const char* instance,
    void* user_data) {
  LOG_DEBUG("OnDisconnectedEvent. sender: %s, instance: %s", sender, instance);
  auto map = CreateEncodableMap("disconnected", sender, instance);
  auto* stub = static_cast<RpcPortStub*>(user_data);
  auto& sink = stub->event_sink_;
  sink->Success(std::move(flutter::EncodableValue(map)));
}

int RpcPortStub::OnReceivedEvent(const char* sender, const char* instance,
    rpc_port_h port, void* user_data) {
  LOG_DEBUG("OnReceivedEvent. sender: %s, instance: %s", sender, instance);
  rpc_port_parcel_h parcel = nullptr;
  int ret = rpc_port_parcel_create_from_port(&parcel, port);
  if (ret != RPC_PORT_ERROR_NONE) {
    LOG_ERROR("rpc_port_parcel_create_from_port() is failed. error(%d)", ret);
    return ret;
  }

  unsigned char* raw = nullptr;
  unsigned int size = 0;
  rpc_port_parcel_get_raw(parcel, reinterpret_cast<void**>(&raw), &size);
  std::vector<uint8_t> raw_data(raw, raw + size);
  rpc_port_parcel_destroy(parcel);

  auto map = CreateEncodableMap("received", sender, instance);
  map[flutter::EncodableValue("rawData")] =
      flutter::EncodableValue(std::move(raw_data));

  auto* stub = static_cast<RpcPortStub*>(user_data);
  auto& sink = stub->event_sink_;
  sink->Success(std::move(flutter::EncodableValue(map)));
  return 0;
}

}  // namespace tizen
