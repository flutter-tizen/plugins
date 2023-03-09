// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TIZEN_CION_GROUP_H
#define TIZEN_CION_GROUP_H

#include <cion.h>
#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "common.h"

namespace tizen {

typedef std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> EventSink;

class CionGroupManager {
 public:
  static void Init(EventSink sync);
  static CionResult Subscribe(cion_group_h handle);

 private:
  static void OnLeftEvent(const char* topic_name,
                          const cion_peer_info_h peer_info, void* user_data);
  static void OnJoinedEvent(const char* topic_name,
                            const cion_peer_info_h peer_info, void* user_data);
  static void OnReceivedEvent(const char* service_name,
                              const cion_peer_info_h peer_info,
                              const cion_payload_h payload, void* user_data);

  static EventSink event_sink_;
};

}  // namespace tizen

#endif  // TIZEN_CION_SERVER_H
