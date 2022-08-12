#include "state_handler.h"

#include <tizen_error.h>

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
  sink = std::move(events);

  auto ret = event_add_event_handler(
      kBtStateSystemEvent,
      [](auto event_name, auto event_data, auto user_data) {
        auto& state_handler = *static_cast<StateHandler*>(user_data);

        LOG_DEBUG("state_changed_callback_event_channel");

        if (event_name == kBtStateSystemEvent) {
          state_handler.callback_();
        }
      },
      this, &handle_);

  LOG_ERROR("event_add_event_handler %s", get_error_message(ret));

  return nullptr;
}

// Implementation of the public interface, to be provided by subclasses.
std::unique_ptr<StateHandler::ErrorType> StateHandler::OnCancelInternal(
    const flutter::EncodableValue* arguments) {
  sink = nullptr;

  if (handle_) {
    auto ret = event_remove_event_handler(handle_);
    LOG_ERROR("event_add_event_handler %s", get_error_message(ret));
  }

  return nullptr;
}

}  // namespace flutter_blue_tizen
