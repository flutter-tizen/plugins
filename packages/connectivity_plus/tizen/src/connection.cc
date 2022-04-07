// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "connection.h"

#include "log.h"

static ConnectionType ToConnectionType(connection_type_e type) {
  switch (type) {
    case CONNECTION_TYPE_WIFI:
      return ConnectionType::kWiFi;
    case CONNECTION_TYPE_CELLULAR:
      return ConnectionType::kMobile;
    case CONNECTION_TYPE_ETHERNET:
      return ConnectionType::kEthernet;
    case CONNECTION_TYPE_DISCONNECTED:
    default:
      return ConnectionType::kNone;
  }
}

Connection::Connection() {
  int ret = connection_create(&connection_);
  if (ret != CONNECTION_ERROR_NONE) {
    LOG_ERROR("Failed to create handle: %s", get_error_message(ret));
    last_error_ = ret;
  }
}

Connection::~Connection() {
  if (connection_) {
    connection_unset_type_changed_cb(connection_);
    connection_destroy(connection_);
    connection_ = nullptr;
  }
}

bool Connection::StartListen(ConnectionTypeCallback callback) {
  int ret = connection_set_type_changed_cb(
      connection_,
      [](connection_type_e type, void *user_data) -> void {
        auto *self = static_cast<Connection *>(user_data);
        self->callback_(ToConnectionType(type));
      },
      this);
  if (ret != CONNECTION_ERROR_NONE) {
    LOG_ERROR("Failed to add callback: %s", get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  callback_ = callback;
  return true;
}

void Connection::StopListen() {
  int ret = connection_unset_type_changed_cb(connection_);
  if (ret != CONNECTION_ERROR_NONE) {
    LOG_ERROR("Failed to remove callback: %s", get_error_message(ret));
    last_error_ = ret;
  }
}

ConnectionType Connection::GetType() {
  connection_type_e type;
  int ret = connection_get_type(connection_, &type);
  if (ret != CONNECTION_ERROR_NONE) {
    LOG_ERROR("Failed to get connection type: %s", get_error_message(ret));
    last_error_ = ret;
    return ConnectionType::kError;
  }
  return ToConnectionType(type);
}
