// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.5/tizen.dart';

import 'parcel.dart';
import 'port.dart';
import 'proxy_base.dart' show OnError;

export 'package:meta/meta.dart' show nonVirtual, visibleForOverriding;

/// The base class for creating a custom stub object.
abstract class StubBase {
  /// Creates a [StubBase] instance with the port name.
  StubBase(this.portName) {
    _handle = using((Arena arena) {
      final Pointer<rpc_port_stub_h> pStub = arena();
      final Pointer<Char> pPortName = portName.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_stub_create(pStub, pPortName);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pStub.value;
    });

    _finalizer.attach(this, this);
  }

  late final rpc_port_stub_h _handle;

  final Finalizer<StubBase> _finalizer = Finalizer<StubBase>((StubBase stub) {
    stub._streamSubscription?.cancel();
    tizen.rpc_port_stub_destroy(stub._handle);
  });

  /// A port name to use when listening for connections.
  final String portName;

  static const EventChannel _eventChannel = EventChannel('tizen/rpc_port_stub');

  // ignore: cancel_subscriptions
  StreamSubscription<dynamic>? _streamSubscription;

  /// Sets whether this stub should only allow trusted connections.
  ///
  /// If [trusted] is true, this stub only allows connections from trusted
  /// applications signed with the same certificate.
  void setTrusted(bool trusted) {
    final int ret = tizen.rpc_port_stub_set_trusted(_handle, trusted);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Adds a privilege which is required when a proxy connects to this stub.
  void addPrivilege(String privilege) {
    using((Arena arena) {
      final Pointer<Char> pPrivilege = privilege.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_stub_add_privilege(_handle, pPrivilege);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// Listens to requests for connections.
  Future<void> listen({OnError? onError}) async {
    if (_streamSubscription != null) {
      return;
    }

    final Stream<dynamic> stream = _eventChannel
        .receiveBroadcastStream(<String, Object>{'handle': _handle.address});
    _streamSubscription = stream.listen((dynamic map) async {
      final int handle = map['handle'] as int;
      if (handle != _handle.address) {
        return;
      }
      final String event = map['event'] as String;
      final String sender = map['sender'] as String;
      final String instance = map['instance'] as String;
      if (event == 'connected') {
        await onConnectedEvent(sender, instance);
      } else if (event == 'disconnected') {
        await onDisconnectedEvent(sender, instance);
      } else if (event == 'received') {
        final Uint8List rawData = map['rawData'] as Uint8List;
        final Parcel parcel = Parcel.fromRaw(rawData);
        await onReceivedEvent(sender, instance, parcel);
      } else {
        print('Unknown event: $event');
      }
    }, onError: onError);
  }

  /// Gets a [Port] associated with a proxy instance with the specified name
  /// and port type.
  Port getPort(String instance, PortType portType) {
    return using((Arena arena) {
      final Pointer<Char> pInstance = instance.toNativeChar();
      final Pointer<rpc_port_h> pPort = arena();
      final int ret = tizen.rpc_port_stub_get_port(
          _handle, portType.index, pInstance, pPort);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return Port.fromNativeHandle(pPort.value);
    });
  }

  /// Called when a connection is established by a proxy instance.
  Future<void> onConnectedEvent(String sender, String instance);

  /// Called when disconnected from a proxy instance.
  Future<void> onDisconnectedEvent(String sender, String instance);

  /// Called when data are received from a proxy instance.
  Future<void> onReceivedEvent(String sender, String instance, Parcel parcel);
}
