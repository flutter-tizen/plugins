// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messageport.h"

#include <bundle.h>

#include <cstdint>
#include <vector>

#include "log.h"

void LocalPort::OnMessageReceived(int local_port_id, const char* remote_app_id,
                                  const char* remote_port,
                                  bool trusted_remote_port, bundle* bundle,
                                  void* user_data) {
  LocalPort* self = static_cast<LocalPort*>(user_data);
  uint8_t* byte_array = nullptr;
  size_t size = 0;
  int ret = bundle_get_byte(bundle, "bytes",
                            reinterpret_cast<void**>(&byte_array), &size);
  if (ret != BUNDLE_ERROR_NONE) {
    Message message{error : "Failed to get data from bundle"};
    self->message_callback_(message);
  } else {
    RemotePort port(remote_app_id ? remote_app_id : "",
                    remote_port ? remote_port : "", trusted_remote_port);
    Message message{port, std::vector<uint8_t>(byte_array, byte_array + size)};
    self->message_callback_(message);
  }
}

LocalPort::~LocalPort() {
  if (port_ != -1) {
    Unregister();
  }
}

std::optional<MessagePortError> LocalPort::Register(
    OnMessage message_callback) {
  int ret = -1;
  if (is_trusted_) {
    ret = message_port_register_trusted_local_port(name_.c_str(),
                                                   OnMessageReceived, this);
    if (ret < MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    port_ = ret;
  } else {
    ret = message_port_register_local_port(name_.c_str(), OnMessageReceived,
                                           this);
    if (ret < MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    port_ = ret;
  }

  message_callback_ = std::move(message_callback);
  return std::nullopt;
}

std::optional<MessagePortError> LocalPort::Unregister() {
  if (is_trusted_) {
    int ret = message_port_unregister_trusted_local_port(port_);
    if (ret != MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
  } else {
    int ret = message_port_unregister_local_port(port_);
    if (ret != MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
  }
  port_ = -1;
  return std::nullopt;
}

ErrorOr<bool> RemotePort::CheckRemotePort() {
  bool exist = false;
  if (is_trusted_) {
    int ret = message_port_check_trusted_remote_port(app_id_.c_str(),
                                                     name_.c_str(), &exist);
    if (ret != MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
  } else {
    int ret =
        message_port_check_remote_port(app_id_.c_str(), name_.c_str(), &exist);
    if (ret != MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
  }

  return exist;
}

std::optional<MessagePortError> RemotePort::Send(
    const std::vector<uint8_t>& encoded_message) {
  ErrorOr<bundle*> maybe_bundle = PrepareBundle(encoded_message);
  if (maybe_bundle.has_error()) {
    return maybe_bundle.error();
  }
  bundle* bundle = maybe_bundle.value();

  int ret = MESSAGE_PORT_ERROR_NONE;
  if (is_trusted_) {
    ret = message_port_send_trusted_message(app_id_.c_str(), name_.c_str(),
                                            bundle);
  } else {
    ret = message_port_send_message(app_id_.c_str(), name_.c_str(), bundle);
  }
  bundle_free(bundle);

  if (ret != MESSAGE_PORT_ERROR_NONE) {
    return MessagePortError(ret);
  }

  return std::nullopt;
}

std::optional<MessagePortError> RemotePort::SendWithLocalPort(
    const std::vector<uint8_t>& encoded_message, LocalPort* local_port) {
  ErrorOr<bundle*> maybe_bundle = PrepareBundle(encoded_message);
  if (maybe_bundle.has_error()) {
    return maybe_bundle.error();
  }
  bundle* bundle = maybe_bundle.value();

  int ret = MESSAGE_PORT_ERROR_NONE;
  if (is_trusted_) {
    ret = message_port_send_trusted_message_with_local_port(
        app_id_.c_str(), name_.c_str(), bundle, local_port->port());
  } else {
    ret = message_port_send_message_with_local_port(
        app_id_.c_str(), name_.c_str(), bundle, local_port->port());
  }
  bundle_free(bundle);

  if (ret != MESSAGE_PORT_ERROR_NONE) {
    return MessagePortError(ret);
  }

  return std::nullopt;
}

ErrorOr<bundle*> RemotePort::PrepareBundle(
    const std::vector<uint8_t>& encoded_message) {
  bundle* bundle = bundle_create();
  if (!bundle) {
    return MessagePortError(MESSAGE_PORT_ERROR_OUT_OF_MEMORY);
  }

  int ret = bundle_add_byte(bundle, "bytes", encoded_message.data(),
                            encoded_message.size());

  if (ret != BUNDLE_ERROR_NONE) {
    bundle_free(bundle);
    return MessagePortError(MESSAGE_PORT_ERROR_INVALID_PARAMETER);
  }

  return bundle;
}
