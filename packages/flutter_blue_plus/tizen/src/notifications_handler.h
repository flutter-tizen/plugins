// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_BLUE_PLUS_TIZEN_NOTIFICATIONS_HANDLER_H
#define FLUTTER_BLUE_PLUS_TIZEN_NOTIFICATIONS_HANDLER_H

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/method_channel.h>

#include <memory>

#include "flutterblue.pb.h"
#include "state_handler.h"

namespace flutter_blue_plus_tizen {

using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;

class NotificationsHandler {
 public:
  explicit NotificationsHandler(std::shared_ptr<MethodChannel> method_channel);

  void NotifyUIThread(
      const std::string& method,
      const google::protobuf::MessageLite& encodable) const noexcept;

 private:
  std::shared_ptr<MethodChannel> method_channel_;
};

}  // namespace flutter_blue_plus_tizen
#endif  // FLUTTER_BLUE_PLUS_TIZEN_NOTIFICATIONS_HANDLER_H
