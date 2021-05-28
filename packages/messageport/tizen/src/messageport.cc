// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "messageport.h"

#include <bundle.h>
#include <flutter/standard_message_codec.h>

#include <cstdint>
#include <vector>

#include "log.h"

MessagePortManager::MessagePortManager() {}

MessagePortManager::~MessagePortManager() {
  for (const auto& m : sinks_) {
    int ret;
    bool is_trusted = trusted_ports_.find(m.first) != trusted_ports_.end();
    if (is_trusted) {
      ret = message_port_unregister_trusted_local_port(m.first);
    } else {
      ret = message_port_unregister_local_port(m.first);
    }
    if (MESSAGE_PORT_ERROR_NONE != ret) {
      LOG_ERROR("Failed: message_port_unregister_%s_local_port",
                is_trusted ? "trusted" : "");
    }
  }
}

static bool ConvertEncodableValueToBundle(flutter::EncodableValue& v,
                                          bundle* b) {
  if (nullptr == b) {
    LOG_ERROR("Invalid bundle handle");
    return false;
  }
  std::unique_ptr<std::vector<uint8_t>> encoded =
      flutter::StandardMessageCodec::GetInstance().EncodeMessage(v);

  int ret = bundle_add_byte(b, "bytes", encoded->data(), encoded->size());
  if (BUNDLE_ERROR_NONE != ret) {
    return false;
  }
  return true;
}

void MessagePortManager::OnMessageReceived(int local_port_id,
                                           const char* remote_app_id,
                                           const char* remote_port,
                                           bool trusted_remote_port,
                                           bundle* message, void* user_data) {
  LOG_DEBUG(
      "OnMessageReceived, local_port_id: %d, remote_app_id: %s, "
      "remote_port: %s, trusted: %s",
      local_port_id, remote_app_id, remote_port,
      trusted_remote_port ? "yes" : "no");

  MessagePortManager* manager = static_cast<MessagePortManager*>(user_data);

  if (manager->sinks_.find(local_port_id) != manager->sinks_.end()) {
    uint8_t* byte_array = NULL;
    size_t size = 0;

    int ret = bundle_get_byte(message, "bytes", (void**)&byte_array, &size);
    if (ret != BUNDLE_ERROR_NONE) {
      manager->sinks_[local_port_id]->Error("Failed to parse response");
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

    map[flutter::EncodableValue("isTrusted")] =
        flutter::EncodableValue(trusted_remote_port);

    manager->sinks_[local_port_id]->Success(flutter::EncodableValue(map));
  }
}

MessagePortResult MessagePortManager::RegisterLocalPort(
    const std::string& port_name, EventSink sink, bool is_trusted,
    int* local_port) {
  LOG_DEBUG("RegisterLocalPort: %s, is_trusted: %s", port_name.c_str(),
            is_trusted ? "yes" : "no");
  int ret = -1;
  if (is_trusted) {
    ret = message_port_register_trusted_local_port(port_name.c_str(),
                                                   OnMessageReceived, this);
  } else {
    ret = message_port_register_local_port(port_name.c_str(), OnMessageReceived,
                                           this);
  }

  if (ret < 0) {
    return CreateResult(ret);
  }

  *local_port = ret;
  LOG_DEBUG("Successfully opened local %s port, native id: %d",
            port_name.c_str(), *local_port);

  sinks_[*local_port] = std::move(sink);

  if (is_trusted) {
    trusted_ports_.insert(*local_port);
  }

  return CreateResult(MESSAGE_PORT_ERROR_NONE);
}

MessagePortResult MessagePortManager::UnregisterLocalPort(int local_port_id) {
  LOG_DEBUG("UnregisterLocalPort: %d", local_port_id);
  bool is_trusted = trusted_ports_.find(local_port_id) != trusted_ports_.end();
  int ret = -1;

  if (is_trusted) {
    ret = message_port_unregister_trusted_local_port(local_port_id);
  } else {
    ret = message_port_unregister_local_port(local_port_id);
  }

  if (MESSAGE_PORT_ERROR_NONE == ret) {
    sinks_.erase(local_port_id);
    if (is_trusted) {
      trusted_ports_.erase(local_port_id);
    }
  }

  return CreateResult(ret);
}

MessagePortResult MessagePortManager::CheckRemotePort(
    std::string& remote_app_id, std::string& port_name, bool is_trusted,
    bool* port_check) {
  LOG_DEBUG("CheckRemotePort remote_app_id: %s, port_name: %s",
            remote_app_id.c_str(), port_name.c_str());

  int ret;
  if (is_trusted) {
    ret = message_port_check_trusted_remote_port(remote_app_id.c_str(),
                                                 port_name.c_str(), port_check);
  } else {
    ret = message_port_check_remote_port(remote_app_id.c_str(),
                                         port_name.c_str(), port_check);
  }

  LOG_DEBUG("message_port_check_%s_remote_port (%s): %s",
            is_trusted ? "trusted" : "", port_name.c_str(),
            *port_check ? "true" : "false");

  return CreateResult(ret);
}

MessagePortResult MessagePortManager::Send(std::string& remote_app_id,
                                           std::string& port_name,
                                           flutter::EncodableValue& message,
                                           bool is_trusted) {
  LOG_DEBUG("Send (%s, %s)", remote_app_id.c_str(), port_name.c_str());
  bundle* b = bundle_create();
  if (nullptr == b) {
    return CreateResult(MESSAGE_PORT_ERROR_OUT_OF_MEMORY);
  }

  bool result = ConvertEncodableValueToBundle(message, b);
  if (!result) {
    LOG_ERROR("Failed to parse EncodableValue");
    bundle_free(b);
    return CreateResult(MESSAGE_PORT_ERROR_INVALID_PARAMETER);
  }

  int ret;
  if (is_trusted) {
    ret = message_port_send_trusted_message(remote_app_id.c_str(),
                                            port_name.c_str(), b);
  } else {
    ret =
        message_port_send_message(remote_app_id.c_str(), port_name.c_str(), b);
  }
  bundle_free(b);
  return CreateResult(ret);
}

MessagePortResult MessagePortManager::Send(std::string& remote_app_id,
                                           std::string& port_name,
                                           flutter::EncodableValue& message,
                                           bool is_trusted, int local_port) {
  LOG_DEBUG("Send (%s, %s), port: %d", remote_app_id.c_str(), port_name.c_str(),
            local_port);
  bundle* b = bundle_create();
  if (nullptr == b) {
    return CreateResult(MESSAGE_PORT_ERROR_OUT_OF_MEMORY);
  }

  bool result = ConvertEncodableValueToBundle(message, b);
  if (!result) {
    LOG_ERROR("Failed to parse EncodableValue");
    bundle_free(b);
    return CreateResult(MESSAGE_PORT_ERROR_INVALID_PARAMETER);
  }

  int ret;
  if (is_trusted) {
    ret = message_port_send_trusted_message_with_local_port(
        remote_app_id.c_str(), port_name.c_str(), b, local_port);
  } else {
    ret = message_port_send_message_with_local_port(
        remote_app_id.c_str(), port_name.c_str(), b, local_port);
  }

  bundle_free(b);

  return CreateResult(ret);
}

MessagePortResult MessagePortManager::CreateResult(int return_code) {
  MessagePortResult result(return_code);

  if (!result) {
    LOG_ERROR("Failed: %s", result.message().c_str());
  }
  return result;
}
