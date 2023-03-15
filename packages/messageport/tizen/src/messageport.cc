// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messageport.h"

#include <bundle.h>
#include <flutter/standard_message_codec.h>

#include <cstdint>
#include <vector>

#include "log.h"

MessagePort::MessagePort() {}

MessagePort::~MessagePort() {
  for (const auto& [name, port] : local_ports_) {
    int ret = MESSAGE_PORT_ERROR_NONE;
    ret = message_port_unregister_local_port(port);

    if (ret != MESSAGE_PORT_ERROR_NONE) {
      LOG_ERROR("Failed to unregister local port: %s", get_error_message(ret));
    }
  }

  for (const auto& [name, port] : trusted_local_ports_) {
    int ret = MESSAGE_PORT_ERROR_NONE;
    ret = message_port_unregister_trusted_local_port(port);

    if (ret != MESSAGE_PORT_ERROR_NONE) {
      LOG_ERROR("Failed to unregister trusted local port: %s",
                get_error_message(ret));
    }
  }
}

static bool ConvertEncodableValueToBundle(flutter::EncodableValue& value,
                                          bundle* bundle) {
  if (!bundle) {
    return false;
  }
  std::unique_ptr<std::vector<uint8_t>> encoded =
      flutter::StandardMessageCodec::GetInstance().EncodeMessage(value);

  int ret = bundle_add_byte(bundle, "bytes", encoded->data(), encoded->size());
  if (ret != BUNDLE_ERROR_NONE) {
    return false;
  }
  return true;
}

void MessagePort::OnMessageReceived(int local_port_id,
                                    const char* remote_app_id,
                                    const char* remote_port,
                                    bool trusted_remote_port, bundle* bundle,
                                    void* user_data) {
  MessagePort* manager = static_cast<MessagePort*>(user_data);
  if (manager->on_messages_.find(local_port_id) !=
      manager->on_messages_.end()) {
    Message message{local_port_id,
                    remote_app_id ? remote_app_id : "",
                    remote_port ? remote_port : "",
                    trusted_remote_port,
                    bundle,
                    user_data};
    manager->on_messages_[local_port_id](message);
  }
}

std::optional<MessagePortError> MessagePort::RegisterLocalPort(
    const std::string& port_name, bool is_trusted, OnMessage on_message) {
  int ret = -1;
  if (is_trusted) {
    ret = message_port_register_trusted_local_port(port_name.c_str(),
                                                   OnMessageReceived, this);
    if (ret < MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    trusted_local_ports_[port_name] = ret;
  } else {
    ret = message_port_register_local_port(port_name.c_str(), OnMessageReceived,
                                           this);
    if (ret < MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    local_ports_[port_name] = ret;
  }

  on_messages_[ret] = std::move(on_message);
  return std::nullopt;
}

std::optional<MessagePortError> MessagePort::UnregisterLocalPort(
    const std::string& port_name, bool is_trusted) {
  ErrorOr<int> maybe_local_port = GetRegisteredLocalPort(port_name, is_trusted);
  if (maybe_local_port.has_error()) {
    return maybe_local_port.error();
  }
  int local_port = maybe_local_port.value();

  int ret = -1;
  if (is_trusted) {
    ret = message_port_unregister_trusted_local_port(local_port);
    if (ret != MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    trusted_local_ports_.erase(port_name);
  } else {
    ret = message_port_unregister_local_port(local_port);
    if (ret != MESSAGE_PORT_ERROR_NONE) {
      return MessagePortError(ret);
    }
    local_ports_.erase(port_name);
  }
  on_messages_.erase(local_port);
  return std::nullopt;
}

ErrorOr<bool> MessagePort::CheckRemotePort(const std::string& remote_app_id,
                                           const std::string& port_name,
                                           bool is_trusted) {
  int ret = MESSAGE_PORT_ERROR_NONE;
  bool exist = false;
  if (is_trusted) {
    ret = message_port_check_trusted_remote_port(remote_app_id.c_str(),
                                                 port_name.c_str(), &exist);
  } else {
    ret = message_port_check_remote_port(remote_app_id.c_str(),
                                         port_name.c_str(), &exist);
  }

  if (ret != MESSAGE_PORT_ERROR_NONE) {
    return MessagePortError(ret);
  }
  return exist;
}

bool MessagePort::IsRegisteredLocalPort(const std::string& port_name,
                                        bool is_trusted) {
  if (is_trusted) {
    if (trusted_local_ports_.find(port_name) != trusted_local_ports_.end()) {
      return true;
    }
  } else {
    if (local_ports_.find(port_name) != local_ports_.end()) {
      return true;
    }
  }

  return false;
}

ErrorOr<int> MessagePort::GetRegisteredLocalPort(const std::string& port_name,
                                                 bool is_trusted) {
  if (is_trusted) {
    auto iter = trusted_local_ports_.find(port_name);
    if (iter != trusted_local_ports_.end()) {
      return iter->second;
    }
  } else {
    auto iter = local_ports_.find(port_name);
    if (iter != local_ports_.end()) {
      return iter->second;
    }
  }

  return MessagePortError(MESSAGE_PORT_ERROR_PORT_NOT_FOUND);
}

std::optional<MessagePortError> MessagePort::Send(
    std::string& remote_app_id, std::string& port_name,
    flutter::EncodableValue& message, bool is_trusted) {
  ErrorOr<bundle*> maybe_bundle = PrepareBundle(message);
  if (maybe_bundle.has_error()) {
    return maybe_bundle.error();
  }
  bundle* bundle = maybe_bundle.value();

  int ret = MESSAGE_PORT_ERROR_NONE;
  if (is_trusted) {
    ret = message_port_send_trusted_message(remote_app_id.c_str(),
                                            port_name.c_str(), bundle);
  } else {
    ret = message_port_send_message(remote_app_id.c_str(), port_name.c_str(),
                                    bundle);
  }
  bundle_free(bundle);

  if (ret != MESSAGE_PORT_ERROR_NONE) {
    return MessagePortError(ret);
  }

  return std::nullopt;
}

std::optional<MessagePortError> MessagePort::Send(
    std::string& remote_app_id, std::string& port_name,
    flutter::EncodableValue& message, bool is_trusted,
    const std::string& local_port_name, bool local_port_is_trusted) {
  ErrorOr<int> maybe_local_port =
      GetRegisteredLocalPort(local_port_name, local_port_is_trusted);
  if (maybe_local_port.has_error()) {
    return maybe_local_port.error();
  }
  int local_port = maybe_local_port.value();

  ErrorOr<bundle*> maybe_bundle = PrepareBundle(message);
  if (maybe_bundle.has_error()) {
    return maybe_bundle.error();
  }
  bundle* bundle = maybe_bundle.value();

  int ret = MESSAGE_PORT_ERROR_NONE;
  if (is_trusted) {
    ret = message_port_send_trusted_message_with_local_port(
        remote_app_id.c_str(), port_name.c_str(), bundle, local_port);
  } else {
    ret = message_port_send_message_with_local_port(
        remote_app_id.c_str(), port_name.c_str(), bundle, local_port);
  }
  bundle_free(bundle);

  if (ret != MESSAGE_PORT_ERROR_NONE) {
    return MessagePortError(ret);
  }

  return std::nullopt;
}

ErrorOr<bundle*> MessagePort::PrepareBundle(flutter::EncodableValue& message) {
  bundle* bundle = bundle_create();
  if (!bundle) {
    return MessagePortError(MESSAGE_PORT_ERROR_OUT_OF_MEMORY);
  }

  bool result = ConvertEncodableValueToBundle(message, bundle);
  if (!result) {
    bundle_free(bundle);
    return MessagePortError(MESSAGE_PORT_ERROR_INVALID_PARAMETER);
  }

  return bundle;
}
