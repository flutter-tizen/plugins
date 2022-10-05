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

  Future<void> _onReceivedEvent(Parcel parcel);

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

abstract class NotifyCallback extends _CallbackBase {
  NotifyCallback({bool once = false})
      : super(_DelegateId.notifyCallback.id, once);

  Future<void> onReceived(String sender, String message);

  @override
  Future<void> _onReceivedEvent(Parcel parcel) async {
    final String sender = parcel.readString();
    final String message = parcel.readString();
    await onReceived(sender, message);
  }
}

abstract class Message extends ProxyBase {
  Message(String appid) : super(appid, 'Message');

  bool _online = false;
  final List<_CallbackBase> _delegateList = <_CallbackBase>[];

  Future<void> onConnected();

  Future<void> onDisconnected();

  Future<void> onRejected(String errorMessage);

  @override
  @nonVirtual
  Future<void> onConnectedEvent() async {
    _online = true;
    await onConnected();
  }

  @override
  @nonVirtual
  Future<void> onDisconnectedEvent() async {
    _online = false;
    await onDisconnected();
  }

  @override
  @nonVirtual
  Future<void> onRejectedEvent(String errorMessage) async {
    await onRejected(errorMessage);
  }

  @override
  @nonVirtual
  Future<void> onReceivedEvent(Parcel parcel) async {
    final int cmd = parcel.readInt32();
    if (cmd != _MethodId.callback.id) {
      return;
    }

    final int id = parcel.readInt32();
    final int sequenceId = parcel.readInt32();
    final bool once = parcel.readBool();
    for (final _CallbackBase delegate in _delegateList) {
      if (delegate.id == id && delegate.sequenceId == sequenceId) {
        await delegate._onReceivedEvent(parcel);
        if (delegate.once && once) {
          _delegateList.remove(delegate);
        }
        break;
      }
    }
  }

  Parcel _consumeCommand(Port port) {
    try {
      final Parcel parcel = Parcel.fromPort(port);
      final int cmd = parcel.readInt32();
      if (cmd != _MethodId.result.id) {
        throw Exception('Received parcel is invalid. $cmd');
      }
      return parcel;
    } catch (error) {
      return Parcel();
    }
  }

  void disposeCallback(String tag) {
    _delegateList.removeWhere((_CallbackBase element) => element.tag == tag);
  }

  Future<int> register(String name, NotifyCallback callback) async {
    if (!_online) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);
    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.register.id);
    parcel.writeString(name);
    callback.serialize(parcel);
    _delegateList.add(callback);
    parcel.send(port);

    late Parcel parcelReceived;
    while (true) {
      parcelReceived = _consumeCommand(port);
      final ParcelHeader headerReceived = parcelReceived.header;
      if (headerReceived.tag.isEmpty) {
        break;
      } else if (headerReceived.sequenceNumber == header.sequenceNumber) {
        break;
      }
    }

    return parcelReceived.readInt32();
  }

  Future<void> unregister() async {
    if (!_online) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);
    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.unregister.id);
    parcel.send(port);
  }

  Future<int> send(String message) async {
    if (!_online) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);
    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.send.id);
    parcel.writeString(message);
    parcel.send(port);

    late Parcel parcelReceived;
    while (true) {
      parcelReceived = _consumeCommand(port);
      final ParcelHeader headerReceived = parcelReceived.header;
      if (headerReceived.tag.isEmpty) {
        break;
      } else if (headerReceived.sequenceNumber == header.sequenceNumber) {
        break;
      }
    }

    return parcelReceived.readInt32();
  }
}
