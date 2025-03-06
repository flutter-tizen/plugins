// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs, omit_local_variable_types

import 'package:tizen_rpc_port/tizen_rpc_port.dart';

const String _tidlVersion = '1.10.6';

enum _DelegateId {
  notifyCallback(1);

  const _DelegateId(this.id);
  final int id;
}

enum _MethodId {
  result(0),
  callback(1),
  register(2),
  unregister(3),
  send(4);

  const _MethodId(this.id);
  final int id;
}

abstract class _Delegate extends Parcelable {
  _Delegate(this.id, this.once);

  int id = 0;
  bool once = false;
  int sequenceId = 0;

  @override
  void serialize(Parcel parcel) {
    parcel.writeInt32(id);
    parcel.writeInt32(sequenceId);
    parcel.writeBool(once);
  }

  @override
  void deserialize(Parcel parcel) {
    id = parcel.readInt32();
    sequenceId = parcel.readInt32();
    once = parcel.readBool();
  }
}

abstract class ServiceBase {
  ServiceBase(this.sender, this.instance);

  final String sender;
  final String instance;
  Port? _port;

  void disconnect() {
    _port?.disconnect();
    _port = null;
  }

  Future<void> onCreate();

  Future<void> onTerminate();

  Future<int> onRegister(String name, NotifyCallback callback);

  Future<void> onUnregister();

  Future<int> onSend(String message);
}

class NotifyCallback extends _Delegate {
  NotifyCallback(this._port, this.service, {bool once = false})
    : super(_DelegateId.notifyCallback.id, once);

  final Port? _port;

  ServiceBase service;
  bool _wasInvoked = false;

  Future<void> invoke(String sender, String message) async {
    if (_port == null) {
      throw StateError('Must be connected first');
    }

    if (once && _wasInvoked) {
      throw Exception('Can be invoked only once');
    }

    final Parcel parcel = Parcel();
    parcel.writeInt32(_MethodId.callback.id);
    serialize(parcel);

    parcel.writeString(sender);
    parcel.writeString(message);

    if (_port != null) {
      parcel.send(_port!);
    }

    _wasInvoked = true;
  }
}

typedef ServiceBuilder = ServiceBase Function(String sender, String instance);

typedef _MethodHandler = Future<void> Function(ServiceBase, Port, Parcel);

class Message extends StubBase {
  Message({required ServiceBuilder serviceBuilder})
    : _serviceBuilder = serviceBuilder,
      super('Message') {
    _methodHandlers[_MethodId.register.id] = _onRegisterMethod;
    _methodHandlers[_MethodId.unregister.id] = _onUnregisterMethod;
    _methodHandlers[_MethodId.send.id] = _onSendMethod;
  }

  final List<ServiceBase> services = <ServiceBase>[];
  final Map<int, _MethodHandler> _methodHandlers = <int, _MethodHandler>{};
  final ServiceBuilder _serviceBuilder;

  @override
  @visibleForOverriding
  @nonVirtual
  Future<void> onConnectedEvent(String sender, String instance) async {
    final ServiceBase service = _serviceBuilder(sender, instance);
    service._port = getPort(instance, PortType.callback);
    await service.onCreate();
    services.add(service);
  }

  @override
  @visibleForOverriding
  @nonVirtual
  Future<void> onDisconnectedEvent(String sender, String instance) async {
    for (final ServiceBase service in services) {
      if (service.instance == instance) {
        await service.onTerminate();
        services.remove(service);
        break;
      }
    }
  }

  Future<void> _onRegisterMethod(
    ServiceBase service,
    Port port,
    Parcel parcel,
  ) async {
    final String name = parcel.readString();
    final NotifyCallback callback = NotifyCallback(service._port, service);
    callback.deserialize(parcel);

    final int ret = await service.onRegister(name, callback);

    final Parcel result = Parcel();
    final ParcelHeader header = parcel.header;
    final ParcelHeader resultHeader = result.header;
    resultHeader.tag = _tidlVersion;
    resultHeader.sequenceNumber = header.sequenceNumber;

    result.writeInt32(_MethodId.result.id);
    result.writeInt32(ret);

    result.send(port);
  }

  Future<void> _onUnregisterMethod(
    ServiceBase service,
    Port port,
    Parcel parcel,
  ) async {
    service.onUnregister();
  }

  Future<void> _onSendMethod(
    ServiceBase service,
    Port port,
    Parcel parcel,
  ) async {
    final String message = parcel.readString();

    final int ret = await service.onSend(message);

    final Parcel result = Parcel();
    final ParcelHeader header = parcel.header;
    final ParcelHeader resultHeader = result.header;
    resultHeader.tag = _tidlVersion;
    resultHeader.sequenceNumber = header.sequenceNumber;

    result.writeInt32(_MethodId.result.id);
    result.writeInt32(ret);

    result.send(port);
  }

  @override
  @visibleForOverriding
  @nonVirtual
  Future<void> onReceivedEvent(
    String sender,
    String instance,
    Parcel parcel,
  ) async {
    ServiceBase? service;
    for (final ServiceBase s in services) {
      if (s.instance == instance) {
        service = s;
        break;
      }
    }

    if (service == null) {
      return;
    }

    final Port port = getPort(instance, PortType.main);
    final int cmd = parcel.readInt32();
    if (_methodHandlers.containsKey(cmd)) {
      await _methodHandlers[cmd]!(service, port, parcel);
    }
  }
}
