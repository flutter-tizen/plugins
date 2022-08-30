// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "state_handler.h"

#include <bundle.h>
#include <tizen_error.h>

#include <any>
#include <cassert>
#include <utility>

#include "bluetooth_device_controller.h"
#include "bluetooth_manager.h"
#include "flutterblue.pb.h"
#include "log.h"
#include "proto_helper.h"

namespace flutter_blue_plus_tizen {

StateHandler::StateHandler() : system_event_handler_(kBtStateChangedEvent) {}

/*
 Handles a request to set up an event stream. Returns nullptr on success,
 or an error on failure.
 |arguments| is stream configuration arguments and
 |events| is an EventSink for emitting events to the Flutter receiver.
 */

std::unique_ptr<StateHandler::ErrorType> StateHandler::OnListenInternal(
    const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
  event_sink_ = std::move(events);

  system_event_handler_.SetCallback([event_sink = event_sink_](auto map) {
    try {
      auto state_str = std::any_cast<std::string>(map[kBtStateChangedKey]);

      assert(state_str == kBtStateOn || state_str == kBtStateOff);

      auto state = state_str == kBtStateOn
                       ? BluetoothManager::BluetoothState::kAdapterOn
                       : BluetoothManager::BluetoothState::kAdapterOff;

      proto::gen::BluetoothState proto_state;
      proto_state.set_state(ToProtoBluetoothState(state));

      event_sink->Success(
          flutter::EncodableValue(MessageToVector(proto_state)));
    } catch (const std::bad_any_cast& e) {
      LOG_ERROR("%s", e.what());
    }
  });

  return nullptr;
}

std::unique_ptr<StateHandler::ErrorType> StateHandler::OnCancelInternal(
    const flutter::EncodableValue* arguments) {
  event_sink_ = nullptr;

  return nullptr;
}

}  // namespace flutter_blue_plus_tizen
