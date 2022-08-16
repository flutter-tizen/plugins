#include "state_handler.h"

#include <bundle.h>
#include <tizen_error.h>

#include "bluetooth_device_controller.h"
#include "bluetooth_manager.h"
#include "log.h"

namespace flutter_blue_tizen {
StateHandler::StateHandler(BluetoothStateChangedCallback callback)
    : callback_(std::move(callback)) {}

// Handles a request to set up an event stream. Returns nullptr on success,
// or an error on failure.
// |arguments| is stream configuration arguments and
// |events| is an EventSink for emitting events to the Flutter receiver.
std::unique_ptr<StateHandler::ErrorType> StateHandler::OnListenInternal(
    const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
  event_sink_ = std::move(events);

  auto ret = event_add_event_handler(
      kBtStateSystemEvent,
      [](auto event_name, auto event_data, auto user_data) {
        auto& state_handler = *static_cast<StateHandler*>(user_data);

        LOG_DEBUG("state_changed_callback_event_channel: %s", event_name);

        if (event_name == kBtStateSystemEvent) {
          enum BluetoothManager::BluetoothState state =
              BluetoothManager::BluetoothState::kAdapterOn;
          LOG_DEBUG("bt_le_state_event_called");

          char* status_str = nullptr;

          auto ret =
              bundle_get_str(event_data, kBtStateSystemEvent, &status_str);

          if (ret) {
            LOG_ERROR("bundle_get_str: %s", get_error_message(ret));
          } else {
		  	//state
            state_handler.callback_(state_handler.event_sink_);
          }
        }
      },
      this, &handle_);

  LOG_ERROR("event_add_event_handler %s", get_error_message(ret));

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
