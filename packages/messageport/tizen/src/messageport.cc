// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messageport.h"

#include <bundle.h>

void LocalPort::OnMessageReceived(int local_port_id, const char* remote_app_id,
                                  const char* remote_port,
                                  bool trusted_remote_port, bundle* bundle,
                                  void* user_data) {
  LocalPort* self = static_cast<LocalPort*>(user_data);

  uint8_t* bytes = nullptr;
  size_t size = 0;
  int ret =
      bundle_get_byte(bundle, "bytes", reinterpret_cast<void**>(&bytes), &size);
  if (ret != BUNDLE_ERROR_NONE) {
    self->error_callback_(ret, "Failed to get data from bundle.");
    return;
  }

  std::vector<uint8_t> message(bytes, bytes + size);
  if (remote_port) {
    RemotePort port(remote_app_id, remote_port, trusted_remote_port);
    self->message_callback_(message, &port);
  } else {
    self->message_callback_(message, nullptr);
  }
}

LocalPort::~LocalPort() {
  if (port_ != -1) {
    Unregister();
  }
}

std::optional<MessagePortError> LocalPort::Register(MessageCallback on_message,
                                                    ErrorCallback on_error) {
  if (is_trusted_) {
    int ret = message_port_register_trusted_local_port(name_.c_str(),
                                                       OnMessageReceived, this);
    if (ret < MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    port_ = ret;
  } else {
    int ret = message_port_register_local_port(name_.c_str(), OnMessageReceived,
                                               this);
    if (ret < MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    port_ = ret;
  }

  message_callback_ = std::move(on_message);
  error_callback_ = std::move(on_error);
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
    const std::vector<uint8_t>& message) {
  ErrorOr<bundle*> maybe_bundle = PrepareBundle(message);
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
    const std::vector<uint8_t>& message, LocalPort* local_port) {
  ErrorOr<bundle*> maybe_bundle = PrepareBundle(message);
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
    const std::vector<uint8_t>& message) {
  bundle* bundle = bundle_create();
  if (!bundle) {
    return MessagePortError(MESSAGE_PORT_ERROR_OUT_OF_MEMORY);
  }

  int ret = bundle_add_byte(bundle, "bytes", message.data(), message.size());
  if (ret != BUNDLE_ERROR_NONE) {
    bundle_free(bundle);
    return MessagePortError(ret);
  }

  return bundle;
}
