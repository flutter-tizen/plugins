// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

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
  _Delegate(this.id, this.once, this.callback) {
    sequenceId = sequenceNum++;
  }

  int id = 0;
  bool once = false;
  int sequenceId = 0;
  Function? callback;
  static int sequenceNum = 0;

  Future<void> onReceivedEvent(Parcel parcel);

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

class _NotifyCallback extends _Delegate {
  _NotifyCallback(NotifyCallback callback, {bool once = false})
      : super(_DelegateId.notifyCallback.id, once, callback);

  @override
  Future<void> onReceivedEvent(Parcel parcel) async {
    final String sender = parcel.readString();
    final String message = parcel.readString();

    callback?.call(sender, message);
  }
}

class Message extends ProxyBase {
  Message(String appid) : super(appid, 'Message');

  final List<_Delegate> _delegates = <_Delegate>[];

  @override
  @visibleForOverriding
  @nonVirtual
  Future<void> onReceivedEvent(Parcel parcel) async {
    final int cmd = parcel.readInt32();
    if (cmd != _MethodId.callback.id) {
      return;
    }

    final int id = parcel.readInt32();
    final int sequenceId = parcel.readInt32();
    final bool once = parcel.readBool();

    for (final _Delegate delegate in _delegates) {
      if (delegate.id == id && delegate.sequenceId == sequenceId) {
        await delegate.onReceivedEvent(parcel);
        if (delegate.once && once) {
          _delegates.remove(delegate);
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
        print('Received parcel is invalid. $cmd');
      }
      return parcel;
    } catch (error) {
      print(error);
      return Parcel();
    }
  }

  void disposeCallback(Function callback) {
    _delegates
        .removeWhere((_Delegate delegate) => delegate.callback == callback);
  }

  int register(String name, NotifyCallback callback) {
    if (!isConnected) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);
    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.register.id);

    parcel.writeString(name);

    {
      final _NotifyCallback delegate = _NotifyCallback(callback);
      delegate.serialize(parcel);
      _delegates.add(delegate);
    }

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

    final int ret = parcelReceived.readInt32();

    return ret;
  }

  Future<void> unregister() async {
    if (!isConnected) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);
    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.unregister.id);

    parcel.send(port);
  }

  int send(String message) {
    if (!isConnected) {
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

    final int ret = parcelReceived.readInt32();

    return ret;
  }
}
