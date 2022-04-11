/**
 * event_stream_handler.h does not include std::string, therefore it must be
 * included before the #include <flutter/event_stream_handler.h> clang format
 * complains in such case, so I had to put this file in clang-format excluded
 * directory with other generated files.
 */
#ifndef STATE_HANDLER_H
#define STATE_HANDLER_H

#include <string>
#include <functional>
#include <memory>
#include <flutter/event_stream_handler.h>


namespace btu {

class StateHandler : public flutter::StreamHandler<flutter::EncodableValue> {
  using T = flutter::EncodableValue;
  using Base = flutter::StreamHandler<T>;
  using err_type = flutter::StreamHandlerError<T>;

  std::shared_ptr<flutter::EventSink<T>> sink;

  virtual auto OnListenInternal(const T* arguments,
                                std::unique_ptr<flutter::EventSink<T>>&& events)
      -> std::unique_ptr<flutter::StreamHandlerError<T>> override;

  virtual auto OnCancelInternal(const T* arguments)
      -> std::unique_ptr<flutter::StreamHandlerError<T>> override;
};

}  // namespace btu
#endif  // STATE_HANDLER_H
