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
  _CallbackBase(this.id, this.once, this.callback) {
    sequenceId = sequenceNum++;
  }

  int id = 0;
  bool once = false;
  int sequenceId = 0;
  Function? callback;
  static int sequenceNum = 0;

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

typedef NotifyCallback = void Function(String, String);

class NotifyCallbackBase extends _CallbackBase {
  NotifyCallbackBase(NotifyCallback callback, {bool once = false})
      : super(_DelegateId.notifyCallback.id, once, callback);

  @override
  Future<void> _onReceivedEvent(Parcel parcel) async {
    final String sender = parcel.readString();
    final String message = parcel.readString();
    callback?.call(sender, message);
  }
}

typedef OnDisconnected = Future<void> Function();

class Message extends ProxyBase {
  Message(String appid) : super(appid, 'Message');

  bool _isOnline = false;
  final List<_CallbackBase> _delegateList = <_CallbackBase>[];

  OnDisconnected? _onDisconnected;

  @override
  @nonVirtual
  Future<void> onDisconnectedEvent() async {
    await super.onDisconnectedEvent();
    _isOnline = false;
    await _onDisconnected?.call();
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

  @override
  Future<void> connect({OnDisconnected? onDisconnected}) async {
    await super.connect();
    _onDisconnected = onDisconnected;
    _isOnline = true;
  }

  @override
  Future<void> disconnect() async {
    await super.disconnect();
    _isOnline = false;
  }

  void disposeCallback(Function callback) {
    _delegateList
        .removeWhere((_CallbackBase element) => element.callback == callback);
  }

  Future<int> register(String name, NotifyCallback callback) async {
    if (!_isOnline) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);
    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.register.id);
    parcel.writeString(name);
    final NotifyCallbackBase callbackBase = NotifyCallbackBase(callback);
    callbackBase.serialize(parcel);
    _delegateList.add(callbackBase);
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
    if (!_isOnline) {
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
    if (!_isOnline) {
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
