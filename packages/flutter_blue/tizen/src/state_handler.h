#ifndef FLUTTER_BLUE_TIZEN_STATE_HANDLER_H
#define FLUTTER_BLUE_TIZEN_STATE_HANDLER_H

#include <app_event.h>

#include <functional>
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
 public:
  constexpr auto static kBtStateSystemEvent = "tizen.system.event.bt_state";

  using Base = flutter::StreamHandler<flutter::EncodableValue>;

  using ErrorType = flutter::StreamHandlerError<flutter::EncodableValue>;

  using BluetoothStateChangedCallback = std::function<void()>;

  StateHandler(BluetoothStateChangedCallback callback);

 private:
  std::shared_ptr<flutter::EventSink<flutter::EncodableValue>> sink;

  BluetoothStateChangedCallback callback_;

  event_handler_h handle_{nullptr};

  virtual std::unique_ptr<ErrorType> OnListenInternal(
      const flutter::EncodableValue* arguments,
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events)
      override;

  virtual std::unique_ptr<ErrorType> OnCancelInternal(
      const flutter::EncodableValue* arguments) override;
};

}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_STATE_HANDLER_H
