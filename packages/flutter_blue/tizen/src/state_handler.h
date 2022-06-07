#ifndef FLUTTER_BLUE_TIZEN_STATE_HANDLER_H
#define FLUTTER_BLUE_TIZEN_STATE_HANDLER_H

#include <memory>
#include <string>

/**
 * it is included with non angle brackets in order to avoid sorting in
 * clang-format. This include is lacking <memory> and <string> and they have to
 * be included before it.
 */
#include "flutter/event_stream_handler.h"

namespace flutter_blue_tizen {

class StateHandler : public flutter::StreamHandler<flutter::EncodableValue> {
  using Base = flutter::StreamHandler<flutter::EncodableValue>;

  using err_type = flutter::StreamHandlerError<flutter::EncodableValue>;

  std::shared_ptr<flutter::EventSink<flutter::EncodableValue>> sink;

  virtual std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
  OnListenInternal(
      const flutter::EncodableValue* arguments,
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events)
      override;

  virtual std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>>
  OnCancelInternal(const flutter::EncodableValue* arguments) override;
};

}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_STATE_HANDLER_H
