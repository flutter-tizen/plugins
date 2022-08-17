#include "state_handler.h"

#include <bundle.h>
#include <tizen_error.h>

#include <any>
#include <cassert>

#include "bluetooth_device_controller.h"
#include "bluetooth_manager.h"
#include "flutterblue.pb.h"
#include "log.h"
#include "proto_helper.h"

namespace flutter_blue_tizen {
StateHandler::StateHandler() : system_event_handler_(kBtStateChangedEvent) {}

// Handles a request to set up an event stream. Returns nullptr on success,
// or an error on failure.
// |arguments| is stream configuration arguments and
// |events| is an EventSink for emitting events to the Flutter receiver.
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

// Implementation of the public interface, to be provided by subclasses.
std::unique_ptr<StateHandler::ErrorType> StateHandler::OnCancelInternal(
    const flutter::EncodableValue* arguments) {
  event_sink_ = nullptr;

  if (handle_) {
    auto ret = event_remove_event_handler(handle_);
    LOG_ERROR("event_add_event_handler %s", get_error_message(ret));
  }

  return nullptr;
}

}  // namespace flutter_blue_tizen
