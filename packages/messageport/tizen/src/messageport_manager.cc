// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messageport_manager.h"

#include <bundle.h>
#include <flutter/standard_message_codec.h>

#include <cstdint>
#include <vector>

#include "log.h"

MessagePortManager::MessagePortManager() {}

MessagePortManager::~MessagePortManager() {
  for (auto& pair : local_ports_) {
    int port = pair.second;
    bool is_trusted = pair.first.second;
    int ret = MESSAGE_PORT_ERROR_NONE;
    if (is_trusted) {
      ret = message_port_unregister_trusted_local_port(port);
    } else {
      ret = message_port_unregister_local_port(port);
    }

    if (MESSAGE_PORT_ERROR_NONE != ret) {
      LOG_ERROR("Failed to unregister port: %s", get_error_message(ret));
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

void MessagePortManager::OnMessageReceived(int local_port_id,
                                           const char* remote_app_id,
                                           const char* remote_port,
                                           bool trusted_remote_port,
                                           bundle* message, void* user_data) {
  MessagePortManager* manager = static_cast<MessagePortManager*>(user_data);
  if (manager->sinks_.find(local_port_id) != manager->sinks_.end()) {
    uint8_t* byte_array = nullptr;
    size_t size = 0;
    int ret = bundle_get_byte(message, "bytes",
                              reinterpret_cast<void**>(&byte_array), &size);
    if (ret != BUNDLE_ERROR_NONE) {
      manager->sinks_[local_port_id]->Error("Failed to parse a response");
    }

    std::vector<uint8_t> encoded(byte_array, byte_array + size);
    auto value =
        flutter::StandardMessageCodec::GetInstance().DecodeMessage(encoded);

    flutter::EncodableMap map;
    map[flutter::EncodableValue("message")] = *(value.get());
    if (remote_port) {
      map[flutter::EncodableValue("remotePort")] =
          flutter::EncodableValue(std::string(remote_port));
    }
    map[flutter::EncodableValue("remoteAppId")] =
        flutter::EncodableValue(remote_app_id);

    map[flutter::EncodableValue("trusted")] =
        flutter::EncodableValue(trusted_remote_port);

    manager->sinks_[local_port_id]->Success(flutter::EncodableValue(map));
  }
}

std::optional<MessagePortError> MessagePortManager::RegisterLocalPort(
    const std::string& port_name, std::unique_ptr<FlEventSink> sink,
    bool is_trusted) {
  int ret = -1;
  if (is_trusted) {
    ret = message_port_register_trusted_local_port(port_name.c_str(),
                                                   OnMessageReceived, this);
  } else {
    ret = message_port_register_local_port(port_name.c_str(), OnMessageReceived,
                                           this);
  }

  if (ret < MESSAGE_PORT_ERROR_NONE) {
    return MessagePortError(ret);
  }

  int local_port = ret;
  std::pair<std::string, bool> key = std::make_pair(port_name, is_trusted);
  local_ports_[key] = local_port;
  sinks_[local_port] = std::move(sink);

  return std::nullopt;
}

std::optional<MessagePortError> MessagePortManager::UnregisterLocalPort(
    const std::string& port_name, bool is_trusted) {
  ErrorOr<int> maybe_local_port = GetRegisteredLocalPort(port_name, is_trusted);
  if (maybe_local_port.has_error()) {
    return maybe_local_port.error();
  }
  int local_port = maybe_local_port.value();

  int ret = -1;
  if (is_trusted) {
    ret = message_port_unregister_trusted_local_port(local_port);
  } else {
    ret = message_port_unregister_local_port(local_port);
  }

  if (ret != MESSAGE_PORT_ERROR_NONE) {
    return MessagePortError(ret);
  }

  sinks_.erase(local_port);
  local_ports_.erase(std::make_pair(port_name, is_trusted));
  return std::nullopt;
}

ErrorOr<bool> MessagePortManager::CheckRemotePort(
    const std::string& remote_app_id, const std::string& port_name,
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

bool MessagePortManager::IsRegisteredLocalPort(const std::string& port_name,
                                               bool is_trusted) {
  auto key = std::make_pair(port_name, is_trusted);
  if (local_ports_.find(key) != local_ports_.end()) {
    return true;
  }
  return false;
}

ErrorOr<int> MessagePortManager::GetRegisteredLocalPort(
    const std::string& port_name, bool is_trusted) {
  auto key = std::make_pair(port_name, is_trusted);
  auto iter = local_ports_.find(key);
  if (iter != local_ports_.end()) {
    return iter->second;
  }
  return MessagePortError(MESSAGE_PORT_ERROR_PORT_NOT_FOUND);
}

std::optional<MessagePortError> MessagePortManager::Send(
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

std::optional<MessagePortError> MessagePortManager::Send(
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

ErrorOr<bundle*> MessagePortManager::PrepareBundle(
    flutter::EncodableValue& message) {
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
