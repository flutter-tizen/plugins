// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rpc_port_stub.h"

#include <flutter/standard_method_codec.h>
#include <rpc-port-parcel.h>

#include <cstdint>
#include <utility>

#include "log.h"

namespace tizen {
namespace {

flutter::EncodableMap CreateEncodableMap(const char* event, const char* sender,
                                         const char* instance,
                                         rpc_port_stub_h handle) {
  return {{flutter::EncodableValue("event"),
           flutter::EncodableValue(std::string(event))},
          {flutter::EncodableValue("sender"),
           flutter::EncodableValue(std::string(sender))},
          {flutter::EncodableValue("instance"),
           flutter::EncodableValue(std::string(instance))},
          {flutter::EncodableValue("handle"),
           flutter::EncodableValue(reinterpret_cast<int64_t>(handle))}};
}

}  // namespace

EventSink RpcPortStubManager::event_sink_;

void RpcPortStubManager::Init(EventSink sync) { event_sink_ = std::move(sync); }

RpcPortResult RpcPortStubManager::Listen(rpc_port_stub_h handle) {
  LOG_DEBUG("Listen: %p", handle);
  rpc_port_stub_add_connected_event_cb(handle, OnConnectedEvent, handle);
  rpc_port_stub_add_disconnected_event_cb(handle, OnDisconnectedEvent, handle);
  rpc_port_stub_add_received_event_cb(handle, OnReceivedEvent, handle);

  int ret = rpc_port_stub_listen(handle);
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_stub_listen() is failed. error: %s", result.message());
  }

  return result;
}

RpcPortResult RpcPortStubManager::AddPrivilege(rpc_port_stub_h handle,
                                               const std::string& privilege) {
  LOG_DEBUG("AddPrivilege: %p, privilege: %s", handle, privilege.c_str());
  int ret = rpc_port_stub_add_privilege(handle, privilege.c_str());
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_stub_add_privilege() is failed. error: %s",
              result.message());
  }

  return result;
}

RpcPortResult RpcPortStubManager::SetTrusted(rpc_port_stub_h handle,
                                             const bool trusted) {
  LOG_DEBUG("SetTrusted: %p, trusted: %s", handle, trusted ? "true" : "false");
  int ret = rpc_port_stub_set_trusted(handle, trusted);
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_stub_set_trusted() is failed. error: %s",
              result.message());
  }

  return result;
}

void RpcPortStubManager::OnConnectedEvent(const char* sender,
                                          const char* instance,
                                          void* user_data) {
  LOG_DEBUG("OnConnectedEvent. sender: %s, instance: %s", sender, instance);
  auto* stub = reinterpret_cast<rpc_port_stub_h>(user_data);
  auto map = CreateEncodableMap("connected", sender, instance, stub);
  event_sink_->Success(std::move(flutter::EncodableValue(map)));
}

void RpcPortStubManager::OnDisconnectedEvent(const char* sender,
                                             const char* instance,
                                             void* user_data) {
  LOG_DEBUG("OnDisconnectedEvent. sender: %s, instance: %s", sender, instance);
  auto* stub = reinterpret_cast<rpc_port_stub_h>(user_data);
  auto map = CreateEncodableMap("disconnected", sender, instance, stub);
  event_sink_->Success(std::move(flutter::EncodableValue(map)));
}

int RpcPortStubManager::OnReceivedEvent(const char* sender,
                                        const char* instance, rpc_port_h port,
                                        void* user_data) {
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

  auto* stub = reinterpret_cast<rpc_port_stub_h>(user_data);
  auto map = CreateEncodableMap("received", sender, instance, stub);
  map[flutter::EncodableValue("rawData")] =
      flutter::EncodableValue(std::move(raw_data));

  event_sink_->Success(std::move(flutter::EncodableValue(map)));
  return 0;
}

}  // namespace tizen
