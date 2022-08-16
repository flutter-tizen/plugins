#include "state_handler.h"

#include <bundle.h>
#include <tizen_error.h>

#include <cassert>

#include "bluetooth_device_controller.h"
#include "bluetooth_manager.h"
#include "log.h"

namespace flutter_blue_tizen {

void StateHandler::BroadcastEvent(
    const google::protobuf::MessageLite& encodable) const noexcept {}

// Handles a request to set up an event stream. Returns nullptr on success,
// or an error on failure.
// |arguments| is stream configuration arguments and
// |events| is an EventSink for emitting events to the Flutter receiver.
std::unique_ptr<StateHandler::ErrorType> StateHandler::OnListenInternal(
    const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
  event_sink_ = std::move(events);

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
