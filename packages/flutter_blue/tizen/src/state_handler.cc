#include <state_handler.h>

namespace flutter_blue_tizen {

std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
StateHandler::OnListenInternal(
    const flutter::EncodableValue* arguments,
    std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events) {
  sink = std::move(events);
  return nullptr;
}

std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
StateHandler::OnCancelInternal(const flutter::EncodableValue* arguments) {
  sink = nullptr;
  return nullptr;
}

}  // namespace flutter_blue_tizen