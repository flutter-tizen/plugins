// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_MESSAGEPORT_H_
#define FLUTTER_PLUGIN_MESSAGEPORT_H_

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <message_port.h>

#include <map>
#include <optional>
#include <string>
#include <variant>

typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;

class MessagePortError {
 public:
  explicit MessagePortError(int error_code)
      : error_code_(error_code),
        error_message_(get_error_message(error_code_)) {}

  const std::string& message() const { return error_message_; }

  int code() { return error_code_; }

 private:
  int error_code_;
  std::string error_message_;
};

template <class T>
class ErrorOr {
 public:
  ErrorOr(const T& other) { new (&vlaue_or_error_) T(other); }

  ErrorOr(const T&& other) { vlaue_or_error_ = std::move(other); }

  ErrorOr(const MessagePortError& other) {
    new (&vlaue_or_error_) MessagePortError(other);
  }

  ErrorOr(const MessagePortError&& other) {
    vlaue_or_error_ = std::move(other);
  }

  bool has_error() const {
    return std::holds_alternative<MessagePortError>(vlaue_or_error_);
  }

  const T& value() const { return std::get<T>(vlaue_or_error_); };

  const MessagePortError& error() const {
    return std::get<MessagePortError>(vlaue_or_error_);
  };

 private:
  ErrorOr() = default;

  T TakeValue() && { return std::get<T>(std::move(vlaue_or_error_)); }

  std::variant<T, MessagePortError> vlaue_or_error_;
};

class MessagePort {
 public:
  static MessagePort& GetInstance() {
    static MessagePort instance;
    return instance;
  }

  ~MessagePort();

  MessagePort(MessagePort const&) = delete;
  MessagePort& operator=(MessagePort const&) = delete;

  ErrorOr<bool> CheckRemotePort(const std::string& remote_app_id,
                                const std::string& port_name, bool is_trusted);

  bool IsRegisteredLocalPort(const std::string& port_name, bool is_trusted);

  std::optional<MessagePortError> RegisterLocalPort(
      const std::string& port_name, std::unique_ptr<FlEventSink> sink,
      bool is_trusted);

  std::optional<MessagePortError> UnregisterLocalPort(
      const std::string& port_name, bool is_trusted);

  std::optional<MessagePortError> Send(std::string& remote_app_id,
                                       std::string& port_name,
                                       flutter::EncodableValue& message,
                                       bool is_trusted);

  std::optional<MessagePortError> Send(std::string& remote_app_id,
                                       std::string& port_name,
                                       flutter::EncodableValue& message,
                                       bool is_trusted,
                                       const std::string& local_port_name,
                                       bool local_is_trusted);

 private:
  MessagePort();

  static void OnMessageReceived(int local_port_id, const char* remote_app_id,
                                const char* remote_port,
                                bool trusted_remote_port, bundle* message,
                                void* user_data);

  ErrorOr<bundle*> PrepareBundle(flutter::EncodableValue& message);

  ErrorOr<int> GetRegisteredLocalPort(const std::string& port_name,
                                      bool is_trusted);

  // map<port, event_sink>
  std::map<int, std::unique_ptr<FlEventSink>> sinks_;

  // map<pair<port_name, trused>, port>
  std::map<std::pair<std::string, bool>, int> local_ports_;
};

#endif  // FLUTTER_PLUGIN_MESSAGEPORT_H_
