// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:tizen_rpc_port/tizen_rpc_port.dart';

const String _tidlVersion = '1.9.1';

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

abstract class _CallbackBase extends Parcelable {
  _CallbackBase(this.id, this.once) {
    sequenceId = sequenceNum++;
  }

  int id = 0;
  bool once = false;
  int sequenceId = 0;
  static int sequenceNum = 0;

  String get tag => '$id::$sequenceId';

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

  Future<void> disconnect() async {
    _port?.disconnect();
    _port = null;
  }

  Future<void> onCreate();

  Future<void> onTerminate();

  Future<int> onRegister(String name, NotifyCallback callback);

  Future<void> onUnregister();

  Future<int> onSend(String message);
}

class NotifyCallback extends _CallbackBase {
  NotifyCallback(this._port, this.service, {bool once = false})
      : super(_DelegateId.notifyCallback.id, once);
  final Port? _port;
  ServiceBase service;
  bool _valid = true;

  Future<void> invoke(String sender, String message) async {
    if (_port == null) {
      throw StateError('Must be connected first');
    }

    if (once && !_valid) {
      throw Exception('InvalidCallbackException');
    }

    final Parcel parcel = Parcel();
    parcel.writeInt32(_MethodId.callback.id);
    serialize(parcel);
    parcel.writeString(sender);
    parcel.writeString(message);
    if (_port != null) {
      parcel.send(_port!);
    }

    _valid = false;
  }
}

abstract class Message extends StubBase {
  Message() : super('Message') {
    _methodHandlers[_MethodId.register.id] = _onRegisterMethod;
    _methodHandlers[_MethodId.unregister.id] = _onUnregisterMethod;
    _methodHandlers[_MethodId.send.id] = _onSendMethod;
  }

  List<ServiceBase> services = <ServiceBase>[];
  final Map<int, dynamic> _methodHandlers = <int, dynamic>{};

  @override
  Future<void> listen() async {
    return await super.listen();
  }

  ServiceBase createInstance(String sender, String instance);

  @override
  @nonVirtual
  Future<void> onConnectedEvent(String sender, String instance) async {
    final ServiceBase service = createInstance(sender, instance);
    service._port = getPort(instance, PortType.callback);
    await service.onCreate();
    services.add(service);
  }

  @override
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
      ServiceBase service, Port port, Parcel parcel) async {
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
      ServiceBase service, Port port, Parcel parcel) async {
    service.onUnregister();
  }

  Future<void> _onSendMethod(
      ServiceBase service, Port port, Parcel parcel) async {
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
  @nonVirtual
  Future<void> onReceivedEvent(
      String sender, String instance, Parcel parcel) async {
    ServiceBase? invokeService;
    for (final ServiceBase service in services) {
      if (service.instance == instance) {
        invokeService = service;
        break;
      }
    }

    if (invokeService == null) {
      return;
    }

    final Port port = getPort(instance, PortType.main);
    final int cmd = parcel.readInt32();
    if (_methodHandlers.containsKey(cmd)) {
      await _methodHandlers[cmd](invokeService, port, parcel);
    }
  }
}
