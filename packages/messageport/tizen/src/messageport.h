// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_MESSAGEPORT_H_
#define FLUTTER_PLUGIN_MESSAGEPORT_H_

#include <message_port.h>

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>

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

  std::variant<T, MessagePortError> vlaue_or_error_;
};

class LocalPort;

class RemotePort {
 public:
  explicit RemotePort(const std::string& app_id, const std::string& name,
                      bool is_trusted)
      : app_id_(app_id), name_(name), is_trusted_(is_trusted) {}

  ErrorOr<bool> CheckRemotePort();

  std::optional<MessagePortError> Send(
      const std::vector<uint8_t>& encoded_message);

  std::optional<MessagePortError> SendWithLocalPort(
      const std::vector<uint8_t>& encoded_message, LocalPort* local_port);

  std::string app_id() const { return app_id_; }

  std::string name() const { return name_; }

  bool is_trusted() const { return is_trusted_; }

 private:
  ErrorOr<bundle*> PrepareBundle(const std::vector<uint8_t>& encoded_message);

  const std::string app_id_;
  const std::string name_;
  const bool is_trusted_ = false;
};

struct Message {
  std::optional<const RemotePort> remote_port = std::nullopt;
  const std::vector<uint8_t> encoded_message;
  const std::string error;
};

typedef std::function<void(const Message&)> OnMessage;

class LocalPort {
 public:
  LocalPort(const std::string& name, bool is_trusted)
      : name_(name), is_trusted_(is_trusted) {}
  ~LocalPort();

  std::optional<MessagePortError> Register(OnMessage message_callback);

  std::optional<MessagePortError> Unregister();

  std::string name() const { return name_; }

  bool is_trusted() const { return is_trusted_; }

  int port() const { return port_; }

 private:
  static void OnMessageReceived(int local_port_id, const char* remote_app_id,
                                const char* remote_port,
                                bool trusted_remote_port, bundle* message,
                                void* user_data);

  const std::string name_;
  const bool is_trusted_ = false;
  OnMessage message_callback_;
  int port_ = -1;
};

#endif  // FLUTTER_PLUGIN_MESSAGEPORT_H_
