// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CONNECTION_H_
#define FLUTTER_PLUGIN_CONNECTION_H_

#include <net_connection.h>
#include <tizen.h>

#include <functional>
#include <string>

enum class ConnectionType { kNone, kEthernet, kWiFi, kMobile, kError };

typedef std::function<void(ConnectionType)> ConnectionTypeCallback;

class Connection {
 public:
  Connection();
  ~Connection();

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

  bool StartListen(ConnectionTypeCallback callback);

  void StopListen();

  ConnectionType GetType();

 private:
  int last_error_ = TIZEN_ERROR_NONE;
  ConnectionTypeCallback callback_ = nullptr;
  connection_h connection_;
};

#endif  // FLUTTER_PLUGIN_CONNECTION_H_
