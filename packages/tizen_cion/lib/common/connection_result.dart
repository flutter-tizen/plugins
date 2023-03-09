// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

export 'dart:typed_data';

/// The connection result status enum.
enum ConnectionStatus {
  /// Connection success.
  ok(0),

  /// Connection is rejected by server.
  rejected(1),

  /// Connection failed.
  error(2);

  // ignore: public_member_api_docs
  const ConnectionStatus(this.id);
  // ignore: public_member_api_docs
  final int id;
}

/// The connection result class.
class ConnectionResult {
  // ignore: public_member_api_docs
  ConnectionResult(this.reason, this.status);

  /// The reason of the rejection.
  final String reason;

  /// The status of the connection result.
  final ConnectionStatus status;
}
