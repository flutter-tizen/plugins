// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_BLUE_PLUS_TIZEN_STATE_HANDLER_H
#define FLUTTER_BLUE_PLUS_TIZEN_STATE_HANDLER_H

#include <app_event.h>
#include <flutter/event_channel.h>

#include <functional>
#include <memory>
#include <string>

#include "flutterblue.pb.h"
/**
 * it is included with non angle brackets in order to avoid sorting in
 * clang-format. This include is lacking <memory> and <string> and they have to
 * be included before it.
 */
#include "flutter/event_stream_handler.h"
#include "system_event_handler.h"

namespace flutter_blue_plus_tizen {

class StateHandler : public flutter::StreamHandler<flutter::EncodableValue> {
 public:
  using Base = flutter::StreamHandler<flutter::EncodableValue>;

  using ErrorType = flutter::StreamHandlerError<flutter::EncodableValue>;

  using EventSink = flutter::EventSink<flutter::EncodableValue>;

  constexpr static auto kBtStateChangedEvent = SYSTEM_EVENT_BT_STATE;
  /* SYSTEM_EVENT_BT_STATE does not contain for some reason
   EVENT_KEY_BT_LE_STATE. They are disabled simultaneously though. */
  constexpr static auto kBtStateChangedKey =
      EVENT_KEY_BT_STATE; /* EVENT_KEY_BT_LE_STATE; */
  constexpr static auto kBtStateOn = EVENT_VAL_BT_LE_ON;
  constexpr static auto kBtStateOff = EVENT_VAL_BT_LE_OFF;

  StateHandler();

  void BroadcastEvent(
      const google::protobuf::MessageLite& encodable) const noexcept;

 private:
  std::shared_ptr<EventSink> event_sink_;

  SystemEventHandler system_event_handler_;

  virtual std::unique_ptr<ErrorType> OnListenInternal(
      const flutter::EncodableValue* arguments,
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events)
      override;

  virtual std::unique_ptr<ErrorType> OnCancelInternal(
      const flutter::EncodableValue* arguments) override;
};

}  // namespace flutter_blue_plus_tizen
#endif  // FLUTTER_BLUE_PLUS_TIZEN_STATE_HANDLER_H
