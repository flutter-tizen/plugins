#include <StateHandler.h>

namespace btu {
auto StateHandler::OnListenInternal(
    const T* arguments, std::unique_ptr<flutter::EventSink<T>>&& events)
    -> std::unique_ptr<flutter::StreamHandlerError<T>> {
  sink = std::move(events);
  return nullptr;
}

auto StateHandler::OnCancelInternal(const T* arguments)
    -> std::unique_ptr<flutter::StreamHandlerError<T>> {
  sink = nullptr;
  return nullptr;
}
}  // namespace btu
