// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TIZEN_CION_SERVER_H
#define TIZEN_CION_SERVER_H

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

class CionServerManager {
 public:
  static void Init(EventSink sync);
  static CionResult Listen(cion_server_h handle);

 private:
  static void OnConnectionRequestEvent(const char* service_name,
                                       const cion_peer_info_h peer_info,
                                       void* user_data);
  static void OnReceivedEvent(const char* service_name,
                              const cion_peer_info_h peer_info,
                              const cion_payload_h payload,
                              cion_payload_transfer_status_e status,
                              void* user_data);
  static void OnDisconnectedEvent(const char* service_name,
                                  const cion_peer_info_h peer_info,
                                  void* user_data);

  static EventSink event_sink_;
};

}  // namespace tizen

#endif  // TIZEN_CION_SERVER_H
