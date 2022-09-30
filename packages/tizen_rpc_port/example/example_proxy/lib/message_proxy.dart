// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:tizen_log/tizen_log.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

// ignore_for_file: public_member_api_docs

const String _logTag = 'RPC_PORT_PROXY';
const String _tidlVersion = '1.9.1';

enum _DelegateId {
  notifyCB(1);

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
    sequenceId = _sequenceNum++;
  }

  int id = 0;
  bool once = false;
  int sequenceId = 0;
  static int _sequenceNum = 0;

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

abstract class NotifyCB extends _CallbackBase {
  NotifyCB({bool once = false}) : super(_DelegateId.notifyCB.id, once);

  Future<void> onReceived(String sender, String msg);

  @override
  Future<void> _onReceivedEvent(Parcel parcel) async {
    final String sender = parcel.readString();
    final String msg = parcel.readString();

    await onReceived(sender, msg);
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

      if (cmd != _MethodId.result.id)
        Log.error(_logTag, 'Received parcel cmd: $cmd');

      return parcel;
    } catch (e) {
      Log.error(_logTag, e.toString());
      return Parcel();
    }
  }

  @override
  Future<void> connect() async {
    Log.info(_logTag, 'connect()');
    await super.connect();
  }

  void disposeCallback(String tag) {
    Log.info(_logTag, 'disposeCallback($tag)');
    _delegateList.removeWhere((_CallbackBase element) => element.tag == tag);
  }

  Future<int> register(String name, NotifyCB cb) async {
    Log.info(_logTag, 'Register');

    if (!_online) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.register.id);

    parcel.writeString(name);
    cb.serialize(parcel);
    _delegateList.add(cb);

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
    Log.info(_logTag, 'Unregister');

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

  Future<int> send(String msg) async {
    Log.info(_logTag, 'Send');

    if (!_online) {
      throw StateError('Must be connected first');
    }

    final Port port = getPort(PortType.main);

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.send.id);

    parcel.writeString(msg);

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
