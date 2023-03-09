// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_cion_group.h"

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
                                         const char* topic_name,
                                         cion_group_h group,
                                         cion_peer_info_h peer_info) {
  return {{flutter::EncodableValue("event"),
           flutter::EncodableValue(std::string(event))},
          {flutter::EncodableValue("topicName"),
           flutter::EncodableValue(std::string(topic_name))},
          {flutter::EncodableValue("handle"),
           flutter::EncodableValue(reinterpret_cast<int64_t>(group))},
          {flutter::EncodableValue("peerHandle"),
           flutter::EncodableValue(reinterpret_cast<int64_t>(peer_info))}};
}
}  // namespace

EventSink CionGroupManager::event_sink_;

void CionGroupManager::Init(EventSink sync) { event_sink_ = std::move(sync); }

CionResult CionGroupManager::Subscribe(cion_group_h handle) {
  LOG_ERROR("Subscribe");

  int ret = cion_group_add_payload_received_cb(handle, OnReceivedEvent, handle);
  if (ret != CION_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = cion_group_add_joined_cb(handle, OnJoinedEvent, handle);
  if (ret != CION_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = cion_group_add_left_cb(handle, OnLeftEvent, handle);
  if (ret != CION_ERROR_NONE) {
    return CreateResult(ret);
  }

  ret = cion_group_subscribe(handle);
  if (ret != CION_ERROR_NONE) {
    return CreateResult(ret);
  }

  return CreateResult(CION_ERROR_NONE);
}

void CionGroupManager::OnLeftEvent(const char* topic_name,
                                   const cion_peer_info_h peer_info,
                                   void* user_data) {
  LOG_ERROR("OnLeftEvent");

  cion_peer_info_h peer_info_cloned;
  int ret = cion_peer_info_clone(peer_info, &peer_info_cloned);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_peer_info_clone() is failed. error(%d)", ret);
    return;
  }

  auto map =
      CreateEncodableMap("left", topic_name, user_data, peer_info_cloned);

  event_sink_->Success(flutter::EncodableValue(map));
}

void CionGroupManager::OnJoinedEvent(const char* topic_name,
                                     const cion_peer_info_h peer_info,
                                     void* user_data) {
  LOG_ERROR("OnJoinedEvent");

  cion_peer_info_h peer_info_cloned;
  int ret = cion_peer_info_clone(peer_info, &peer_info_cloned);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_peer_info_clone() is failed. error(%d)", ret);
    return;
  }

  auto map =
      CreateEncodableMap("joined", topic_name, user_data, peer_info_cloned);

  event_sink_->Success(flutter::EncodableValue(map));
}

void CionGroupManager::OnReceivedEvent(const char* topic_name,
                                       const cion_peer_info_h peer_info,
                                       const cion_payload_h payload,
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
      CreateEncodableMap("received", topic_name, user_data, peer_info_cloned);
  cion_payload_type_e type;
  ret = cion_payload_get_type(payload, &type);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_payload_get_type() is failed. error(%d)", ret);
    return;
  }

  map[flutter::EncodableValue("type")] =
      flutter::EncodableValue(static_cast<int32_t>(type));
  unsigned char* data;
  unsigned int size;
  ret = cion_payload_get_data(payload, &data, &size);
  if (ret != CION_ERROR_NONE) {
    LOG_ERROR("cion_payload_get_data() is failed. error(%d)", ret);
    return;
  }

  /* Group can use data payload only. */
  auto data_auto = std::unique_ptr<unsigned char, decltype(free)*>(data, free);
  std::vector<uint8_t> raw_data(data, data + size);
  map[flutter::EncodableValue("data")] = flutter::EncodableValue((raw_data));

  peer_info_auto.release();
  event_sink_->Success(flutter::EncodableValue(map));
}

}  // namespace tizen
