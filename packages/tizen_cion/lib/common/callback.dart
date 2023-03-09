// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'payload.dart';
import 'peer_info.dart';

/// The method type for receiving connection request event.
typedef OnConnectionRequest = Future<void> Function(PeerInfo);

/// The method type for receiving connection result.
typedef OnConnectionResult = Future<void> Function(PeerInfo);

/// The method type for receiving disconnected event.
typedef OnDisconnected = Future<void> Function(PeerInfo);

/// The method type for receiving the event discovering server peer.
typedef OnDiscovered = Future<void> Function(PeerInfo);

/// The method type for receiving the payload.
typedef OnReceived = Future<void> Function(
    PeerInfo, Payload, PayloadTransferStatus);

/// The method type for receiving joined group event.
typedef OnJoined = Future<void> Function(PeerInfo);

/// The method type for receiving left group event.
typedef OnLeft = Future<void> Function(PeerInfo);

/// The method type for receiving the payload from Group.
typedef OnGroupReceived = Future<void> Function(PeerInfo, DataPayload);
