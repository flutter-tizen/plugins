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
  // Definition for key of SYSTEM_EVENT_BT_STATE.
  constexpr auto static kBtStateSystemEvent = EVENT_KEY_BT_LE_STATE;
  constexpr auto static kBtStateOff = EVENT_VAL_BT_LE_OFF;
  constexpr auto static kBtStateOn = EVENT_VAL_BT_LE_ON;

  using Base = flutter::StreamHandler<flutter::EncodableValue>;

  using ErrorType = flutter::StreamHandlerError<flutter::EncodableValue>;

  using EventSink = flutter::EventSink<flutter::EncodableValue>;

  using BluetoothStateChangedCallback =
      std::function<void(std::shared_ptr<EventSink>)>;

  StateHandler(BluetoothStateChangedCallback callback);

 private:
  std::shared_ptr<EventSink> event_sink_;

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
