// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MESSAGEPORT_H
#define MESSAGEPORT_H

#include <flutter/event_channel.h>
#include <flutter/standard_method_codec.h>
#include <message_port.h>

#include <map>
#include <set>

typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;

class MessagePortError {
 public:
  explicit MessagePortError(int error_code) : error_code_(error_code) {}

  const std::string& message() const {
    return std::string(get_error_message(error_code_));
  }

 private:
  int error_code_;
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

class MessagePortManager {
 public:
  static MessagePortManager& GetInstance() {
    static MessagePortManager instance;
    return instance;
  }

  MessagePortManager();
  ~MessagePortManager();

  MessagePortManager(MessagePortManager const&) = delete;
  MessagePortManager& operator=(MessagePortManager const&) = delete;

  ErrorOr<bool> CheckRemotePort(std::string& remote_app_id,
                                std::string& port_name, bool is_trusted);

  ErrorOr<int> RegisterLocalPort(const std::string& port_name,
                                 std::unique_ptr<FlEventSink> sink,
                                 bool is_trusted);

  std::optional<MessagePortError> UnregisterLocalPort(int local_port_id);

  std::optional<MessagePortError> Send(std::string& remote_app_id,
                                       std::string& port_name,
                                       flutter::EncodableValue& message,
                                       bool is_trusted);

  std::optional<MessagePortError> Send(std::string& remote_app_id,
                                       std::string& port_name,
                                       flutter::EncodableValue& message,
                                       bool is_trusted, int local_port);

 private:
  static void OnMessageReceived(int local_port_id, const char* remote_app_id,
                                const char* remote_port,
                                bool trusted_remote_port, bundle* message,
                                void* user_data);

  ErrorOr<bundle*> PrepareBundle(flutter::EncodableValue& message);

  std::map<int, std::unique_ptr<FlEventSink>> sinks_;
  std::set<int> trusted_ports_;
};

#endif  // MESSAGEPORT_H
