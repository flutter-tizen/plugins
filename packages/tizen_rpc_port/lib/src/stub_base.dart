// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.5/tizen.dart';

import 'package:tizen_log/tizen_log.dart';
import 'parcel.dart';
import 'port.dart';
import 'rpc_port_method_channel.dart';

export 'package:meta/meta.dart' show nonVirtual;

/// The abstract class for creating a stub class for RPC.
abstract class StubBase {
  /// The constructor for this class.
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

  static const String _logTag = 'RPC_PORT';

  static final MethodChannelRpcPort _methodChannel =
      MethodChannelRpcPort.instance;

  final Finalizer<StubBase> _finalizer = Finalizer<StubBase>((StubBase stub) {
    stub._streamSubscription?.cancel();
    tizen.rpc_port_stub_destroy(stub._handle);
  });

  /// The port name of the connection with the proxy.
  final String portName;

  /// ignore: cancel_subscriptions
  StreamSubscription<dynamic>? _streamSubscription;

  /// Sets a trusted proxy to the stub.
  void setTrusted(bool trusted) {
    final int ret = tizen.rpc_port_stub_set_trusted(_handle, trusted);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Adds a privilege which is required when the proxy connects this stub.
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

  /// Listens to the requests for connections.
  Future<void> listen() async {
    if (_streamSubscription != null) {
      return;
    }

    final Stream<dynamic> stream =
        await _methodChannel.stubListen(_handle.address);
    _streamSubscription = stream.listen((dynamic event) async {
      if (event is Map) {
        final Map<dynamic, dynamic> map = event;
        final int handle = map['handle'] as int;
        if (handle != _handle.address) {
          return;
        }

        final String eventName = map['event'] as String;
        final String sender = map['sender'] as String;
        final String instance = map['instance'] as String;

        Log.info(
            _logTag, 'event: $eventName, sender: $sender, instance: $instance');
        if (eventName == 'connected') {
          await onConnectedEvent(sender, instance);
        } else if (eventName == 'disconnected') {
          await onDisconnectedEvent(sender, instance);
        } else if (eventName == 'received') {
          final Uint8List rawData = map['rawData'] as Uint8List;
          final Parcel parcel = Parcel.fromRaw(rawData);
          await onReceivedEvent(sender, instance, parcel);
        } else {
          Log.error(_logTag, 'Unknown event: $eventName');
        }
      }
    });
  }

  /// Gets a port connected with proxy that has instance id.
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

  /// The abstract method for receiving connected event.
  Future<void> onConnectedEvent(String sender, String instance);

  /// The abstract method for receiving disconnected event.
  Future<void> onDisconnectedEvent(String sender, String instance);

  /// The abstract method called when the stub receives data from proxy.
  Future<void> onReceivedEvent(String sender, String instance, Parcel parcel);
}
