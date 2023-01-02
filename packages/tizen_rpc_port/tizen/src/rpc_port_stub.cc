// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rpc_port_stub.h"

#include <rpc-port-parcel.h>

#include <cstdint>
#include <utility>
#include <vector>

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

std::unique_ptr<FlEventSink> RpcPortStubManager::event_sink_;

void RpcPortStubManager::Init(std::unique_ptr<FlEventSink> event_sink) {
  event_sink_ = std::move(event_sink);
}

RpcPortResult RpcPortStubManager::Listen(rpc_port_stub_h handle) {
  LOG_DEBUG("Listen: %p", handle);

  int ret =
      rpc_port_stub_add_connected_event_cb(handle, OnConnectedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return RpcPortResult(ret);
  }
  ret = rpc_port_stub_add_disconnected_event_cb(handle, OnDisconnectedEvent,
                                                handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return RpcPortResult(ret);
  }
  ret = rpc_port_stub_add_received_event_cb(handle, OnReceivedEvent, handle);
  if (ret != RPC_PORT_ERROR_NONE) {
    return RpcPortResult(ret);
  }

  ret = rpc_port_stub_listen(handle);
  return RpcPortResult(ret);
}

void RpcPortStubManager::OnConnectedEvent(const char* sender,
                                          const char* instance,
                                          void* user_data) {
  LOG_DEBUG("Connected: sender(%s), instance(%s)", sender, instance);

  rpc_port_stub_h handle = reinterpret_cast<rpc_port_stub_h>(user_data);
  flutter::EncodableMap map =
      CreateEncodableMap("connected", sender, instance, handle);
  event_sink_->Success(flutter::EncodableValue(map));
}

void RpcPortStubManager::OnDisconnectedEvent(const char* sender,
                                             const char* instance,
                                             void* user_data) {
  LOG_DEBUG("Disconnected: sender(%s), instance(%s)", sender, instance);

  rpc_port_stub_h handle = reinterpret_cast<rpc_port_stub_h>(user_data);
  flutter::EncodableMap map =
      CreateEncodableMap("disconnected", sender, instance, handle);
  event_sink_->Success(flutter::EncodableValue(map));
}

int RpcPortStubManager::OnReceivedEvent(const char* sender,
                                        const char* instance, rpc_port_h port,
                                        void* user_data) {
  LOG_DEBUG("Received: sender(%s), instance(%s)", sender, instance);

  rpc_port_parcel_h parcel = nullptr;
  int ret = rpc_port_parcel_create_from_port(&parcel, port);
  if (ret != RPC_PORT_ERROR_NONE) {
    event_sink_->Error("rpc_port_parcel_create_from_port failed",
                       std::string(get_error_message(ret)));
    return 0;
  }

  unsigned char* raw = nullptr;
  unsigned int size = 0;
  ret = rpc_port_parcel_get_raw(parcel, reinterpret_cast<void**>(&raw), &size);
  if (ret != RPC_PORT_ERROR_NONE) {
    rpc_port_parcel_destroy(parcel);
    event_sink_->Error("rpc_port_parcel_get_raw failed",
                       std::string(get_error_message(ret)));
    return 0;
  }
  std::vector<uint8_t> raw_data(raw, raw + size);
  rpc_port_parcel_destroy(parcel);

  rpc_port_stub_h handle = reinterpret_cast<rpc_port_stub_h>(user_data);
  flutter::EncodableMap map =
      CreateEncodableMap("received", sender, instance, handle);
  map[flutter::EncodableValue("rawData")] =
      flutter::EncodableValue(std::move(raw_data));
  event_sink_->Success(flutter::EncodableValue(map));

  return 0;
}

}  // namespace tizen
