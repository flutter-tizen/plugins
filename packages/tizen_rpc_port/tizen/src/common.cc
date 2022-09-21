// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common.h"

#include "log.h"

namespace tizen {

RpcPortResult CreateResult(int return_code) {
  RpcPortResult result(return_code);
  if (!result) {
    LOG_ERROR("Failed: %s", result.message());
  }

  return result;
}

RpcPort::RpcPort(rpc_port_h handle, int32_t type)
    : handle_(handle), type_(type) {}

RpcPortResult RpcPort::Read(std::vector<uint8_t>* data, int32_t size) {
  LOG_DEBUG("Read. data: %p, size: %d", data, size);
  if (data == nullptr) {
    LOG_ERROR("Invalid parameter");
    return RpcPortResult(RPC_PORT_ERROR_INVALID_PARAMETER);
  }

  data->resize(size);
  int ret =
      rpc_port_read(handle_, data->data(), static_cast<unsigned int>(size));
  RpcPortResult result(ret);
  if (!result)
    LOG_ERROR("rpc_port_read() is failed. error: %s", result.message());

  return result;
}

RpcPortResult RpcPort::Write(const std::vector<uint8_t>& data) {
  LOG_DEBUG("Write. size: %u", data.size());
  int ret = rpc_port_write(handle_, data.data(), data.size());
  RpcPortResult result(ret);
  if (!result)
    LOG_ERROR("rpc_port_write() is failed. error: %s", result.message());

  return result;
}

RpcPortResult RpcPort::SetPrivateSharing(
    const std::vector<std::string>& paths) {
  LOG_DEBUG("SetPrivateSharing");
  std::vector<const char*> path_array;
  for (auto const& path : paths) path_array.push_back(path.c_str());

  int ret = rpc_port_set_private_sharing_array(handle_, path_array.data(),
                                               path_array.size());
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_set_private_sharing_array() is failed. error: %s",
              result.message());
  }

  return result;
}

RpcPortResult RpcPort::SetPrivateSharing(const std::string& path) {
  LOG_DEBUG("SetPrivateSharing");
  int ret = rpc_port_set_private_sharing(handle_, path.c_str());
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_set_private_sharing() is failed. error: %s",
              result.message());
  }

  return result;
}

RpcPortResult RpcPort::UnsetPrivateSharing() {
  LOG_DEBUG("UnsetPrivateSharing");
  int ret = rpc_port_unset_private_sharing(handle_);
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_unset_private_sharing() is failed. error: %s",
              result.message());
  }

  return result;
}

RpcPortResult RpcPort::PortDisconnect() {
  LOG_DEBUG("PortDisconnect");
  int ret = rpc_port_disconnect(handle_);
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_disconnect() is failed. error: %s", result.message());
  }

  return result;
}

int32_t RpcPort::GetType() const { return type_; }

rpc_port_h RpcPort::GetHandle() const { return handle_; }

RpcPortParcel::RpcPortParcel() {
  LOG_DEBUG("RpcPortParcel");
  int ret = rpc_port_parcel_create(&handle_);
  if (ret != RPC_PORT_ERROR_NONE)
    LOG_ERROR("rpc_port_parcel_create() is failed. error: %d", ret);
}

RpcPortParcel::RpcPortParcel(const std::vector<uint8_t>& raw) {
  LOG_DEBUG("RpcPortParcel with raw");
  int ret = rpc_port_parcel_create_from_raw(&handle_, raw.data(), raw.size());
  if (ret != RPC_PORT_ERROR_NONE)
    LOG_ERROR("rpc_port_parcel_create_from_raw() is failed. error: %d", ret);
}

RpcPortParcel::RpcPortParcel(RpcPort* port) {
  LOG_DEBUG("RpcPortParcel with port");
  int ret = rpc_port_parcel_create_from_port(&handle_, port->GetHandle());
  if (ret != RPC_PORT_ERROR_NONE)
    LOG_ERROR("rpc_port_parcel_create_from_port() is failed. error: %d", ret);
}

RpcPortParcel::~RpcPortParcel() {
  if (handle_ != nullptr) rpc_port_parcel_destroy(handle_);
}

RpcPortResult RpcPortParcel::Send(RpcPort* port) {
  LOG_DEBUG("Send");
  int ret = rpc_port_parcel_send(handle_, port->GetHandle());
  RpcPortResult result(ret);
  if (!result) {
    LOG_ERROR("rpc_port_parcel_send() is failed. error: %s", result.message());
  }

  return result;
}

RpcPortResult RpcPortParcel::GetRaw(std::vector<uint8_t>* raw) {
  LOG_DEBUG("GetRaw");
  uint8_t* raw_data = nullptr;
  unsigned int size = 0;
  int ret = rpc_port_parcel_get_raw(handle_,
                                    reinterpret_cast<void**>(&raw_data), &size);
  if (ret != RPC_PORT_ERROR_NONE) {
    LOG_ERROR("rpc_port_parcel_get_raw() is failed. error: %d", ret);
    return CreateResult(ret);
  }

  raw->assign(raw_data, raw_data + size);
  return CreateResult(RPC_PORT_ERROR_NONE);
}

rpc_port_parcel_h RpcPortParcel::GetHandle() const { return handle_; }

}  // namespace tizen
