// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_cion_server.h"

#include <app_common.h>
#include <bundle.h>
#include <flutter/standard_message_codec.h>
#include <unistd.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "log.h"

namespace tizen {

namespace {

flutter::EncodableMap CreateEncodableMap(const char* event,
                                         const char* service_name,
                                         cion_server_h server,
                                         cion_peer_info_h peer_info) {
  return {{flutter::EncodableValue("event"),
           flutter::EncodableValue(std::string(event))},
          {flutter::EncodableValue("serviceName"),
           flutter::EncodableValue(std::string(service_name))},
          {flutter::EncodableValue("handle"),
           flutter::EncodableValue(reinterpret_cast<int64_t>(server))},
          {flutter::EncodableValue("peerHandle"),
           flutter::EncodableValue(reinterpret_cast<int64_t>(peer_info))}};
}

}  // namespace

EventSink CionServerManager::event_sink_;

void CionServerManager::Init(EventSink sync) { event_sink_ = std::move(sync); }

CionResult CionServerManager::Listen(cion_server_h handle) {
  LOG_ERROR("Listen");
  int ret =
      cion_server_add_payload_received_cb(handle, OnReceivedEvent, handle);
  if (ret != CION_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = cion_server_add_disconnected_cb(handle, OnDisconnectedEvent, handle);
  if (ret != CION_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = cion_server_listen(handle, OnConnectionRequestEvent, handle);

  return CreateResult(CION_ERROR_NONE);
}

void CionServerManager::OnConnectionRequestEvent(
    const char* service_name, const cion_peer_info_h peer_info,
    void* user_data) {
  cion_peer_info_h peer_info_cloned;
  int ret = cion_peer_info_clone(peer_info, &peer_info_cloned);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_peer_info_clone() is failed. error(%d)", ret);
    return;
  }

  auto map = CreateEncodableMap("connectionRequest", service_name, user_data,
                                peer_info_cloned);
  event_sink_->Success(flutter::EncodableValue(map));
}

void CionServerManager::OnReceivedEvent(const char* service_name,
                                        const cion_peer_info_h peer_info,
                                        const cion_payload_h payload,
                                        cion_payload_transfer_status_e status,
                                        void* user_data) {
  LOG_ERROR("OnReceivedEvent");
  cion_peer_info_h peer_info_cloned;
  int ret = cion_peer_info_clone(peer_info, &peer_info_cloned);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_peer_info_clone() is failed. error(%d)", ret);
    return;
  }

  auto peer_info_auto = std::unique_ptr<std::remove_pointer_t<cion_peer_info_h>,
                                        decltype(cion_peer_info_destroy)*>(
      peer_info_cloned, cion_peer_info_destroy);

  auto map =
      CreateEncodableMap("received", service_name, user_data, peer_info_cloned);
  cion_payload_type_e type;
  ret = cion_payload_get_type(payload, &type);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_payload_get_type() is failed. error(%d)", ret);
    return;
  }

  map[flutter::EncodableValue("type")] =
      flutter::EncodableValue(static_cast<int32_t>(type));
  map[flutter::EncodableValue("status")] =
      flutter::EncodableValue(static_cast<int32_t>(status));
  switch (type) {
    case CION_PAYLOAD_TYPE_DATA: {
      unsigned char* data;
      unsigned int size;
      ret = cion_payload_get_data(payload, &data, &size);
      if (ret != CION_ERROR_NONE) {
        LOG_ERROR("cion_payload_get_data() is failed. error(%d)", ret);
        return;
      }

      auto data_auto =
          std::unique_ptr<unsigned char, decltype(free)*>(data, free);
      std::vector<uint8_t> raw_data(data, data + size);
      map[flutter::EncodableValue("data")] =
          flutter::EncodableValue((raw_data));
      break;
    }

    case CION_PAYLOAD_TYPE_FILE: {
      char* file_name;
      ret = cion_payload_get_received_file_name(payload, &file_name);
      if (ret != CION_ERROR_NONE) {
        LOG_ERROR("cion_payload_get_received_file_name() is failed. error(%d)",
                  ret);

        return;
      }
      auto file_name_auto =
          std::unique_ptr<char, decltype(free)*>(file_name, free);

      uint64_t received_bytes;
      ret = cion_payload_get_received_bytes(payload, &received_bytes);
      if (ret != CION_ERROR_NONE) {
        LOG_ERROR("cion_payload_get_received_bytes() is failed. error(%d)",
                  ret);

        return;
      }

      uint64_t total_bytes;
      ret = cion_payload_get_total_bytes(payload, &total_bytes);
      if (ret != CION_ERROR_NONE) {
        LOG_ERROR("cion_payload_get_received_bytes() is failed. error(%d)",
                  ret);

        return;
      }

      char* app_data_path = app_get_data_path();
      if (app_data_path == nullptr) {
        LOG_ERROR("app_get_data_path() is failed. error(%d)", -1);
        return;
      }
      auto app_data_path_auto =
          std::unique_ptr<char, decltype(free)*>(app_data_path, free);
      std::string new_file_name =
          std::string(app_data_path) + "/.cion/.temp/flutter_cion_" + file_name;

      if (status == CION_PAYLOAD_TRANSFER_STATUS_SUCCESS) {
        ret = cion_payload_save_as_file(payload, new_file_name.c_str());
        if (ret != CION_ERROR_NONE) {
          LOG_ERROR("cion_payload_save_as_file() is failed. error(%d)", ret);
          return;
        }
      }

      map[flutter::EncodableValue("path")] =
          flutter::EncodableValue(new_file_name);
      map[flutter::EncodableValue("receivedBytes")] =
          flutter::EncodableValue(static_cast<int64_t>(received_bytes));
      map[flutter::EncodableValue("totalBytes")] =
          flutter::EncodableValue(static_cast<int64_t>(total_bytes));
      break;
    }
  }

  peer_info_auto.release();
  event_sink_->Success(flutter::EncodableValue(map));
}

void CionServerManager::OnDisconnectedEvent(const char* service_name,
                                            const cion_peer_info_h peer_info,
                                            void* user_data) {
  cion_peer_info_h peer_info_cloned;
  int ret = cion_peer_info_clone(peer_info, &peer_info_cloned);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_peer_info_clone() is failed. error(%d)", ret);
    return;
  }

  auto map = CreateEncodableMap("disconnected", service_name, user_data,
                                peer_info_cloned);
  event_sink_->Success(flutter::EncodableValue(map));
}
}  // namespace tizen
