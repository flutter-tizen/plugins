// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:tizen_log/tizen_log.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

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

/// Abstract class for creating a [CallbackBase] class for RPC.
abstract class CallbackBase extends Parcelable {
  /// Constructor for this class.
  CallbackBase(this._id, this._once) {
    _seqId = _seqNum++;
  }

  /// Creating a [CallbackBase] class from the parcel.
  CallbackBase.fromParcel(Parcel parcel) {
    deserialize(parcel);
  }

  int _id = 0;
  bool _once = false;
  int _seqId = 0;
  static int _seqNum = 0;

  /// Gets the tag.
  String get tag => '$_id::$_seqId';

  Future<void> _onReceivedEvent(Parcel parcel);

  @override
  void serialize(Parcel parcel) {
    parcel.writeInt32(_id);
    parcel.writeInt32(_seqId);
    parcel.writeBool(_once);
  }

  @override
  void deserialize(Parcel parcel) {
    _id = parcel.readInt32();
    _seqId = parcel.readInt32();
    _once = parcel.readBool();
  }
}

/// The NotifyCB class to invoke the delegate method.
abstract class NotifyCB extends CallbackBase {
  /// Constructor for this class.
  NotifyCB({bool once = false}) : super(_DelegateId.notifyCB.id, once);

  /// This abstract method will be called when the delegate is received event.
  Future<void> onReceived(String sender, String msg);

  @override
  Future<void> _onReceivedEvent(Parcel parcel) async {
    final String sender = parcel.readString();
    final String msg = parcel.readString();

    await onReceived(sender, msg);
  }
}

/// Abstract class for creating [Message] class for RPC.
abstract class Message extends ProxyBase {
  /// Constructor for this class.
  Message(String appid) : super(appid, 'Message');

  bool _online = false;
  final List<CallbackBase> _delegateList = <CallbackBase>[];

  /// This abstract method will be called when the connet() is succeed.
  Future<void> onConnected();

  /// This abstract method will be called when the connection with the stub is disconnected.
  Future<void> onDisconnected();

  /// This abstract method will be called when connect() is failed.
  Future<void> onRejected(String errorMessage);

  @override
  Future<void> onConnectedEvent() async {
    _online = true;
    await onConnected();
  }

  @override
  Future<void> onDisconnectedEvent() async {
    _online = false;
    await onDisconnected();
  }

  @override
  Future<void> onRejectedEvent(String errorMessage) async {
    await onRejected(errorMessage);
  }

  @override
  Future<void> onReceivedEvent(Parcel parcel) async {
    final int cmd = parcel.readInt32();
    if (cmd != _MethodId.callback.id) {
      return;
    }

    await _processReceivedEvent(parcel);
  }

  Future<void> _processReceivedEvent(Parcel parcel) async {
    final int id = parcel.readInt32();
    final int seqId = parcel.readInt32();
    final bool once = parcel.readBool();

    for (final CallbackBase delegate in _delegateList) {
      if (delegate._id == id && delegate._seqId == seqId) {
        await delegate._onReceivedEvent(parcel);
        if (delegate._once && once) {
          _delegateList.remove(delegate);
        }
        break;
      }
    }
  }

  Future<Parcel> _consumeCommand(Port port) async {
    while (true) {
      try {
        final Parcel parcel = Parcel.fromPort(port);
        final int cmd = parcel.readInt32();
        if (cmd == _MethodId.result.id) {
          return parcel;
        }
      } catch (e) {
        Log.error(_logTag, e.toString());
        return Parcel();
      }
    }
  }

  /// Connects with the stub application.
  @override
  Future<void> connect() async {
    Log.info(_logTag, 'connect()');
    await super.connect();
  }

  /// Dispose registered delegate interface.
  void disposeCallback(String tag) {
    Log.info(_logTag, 'disposeCallback($tag)');
    _delegateList.removeWhere((CallbackBase element) => element.tag == tag);
  }

  /// This method is used to send 'Register' request to the stub app.
  Future<int> register(String name, NotifyCB cb) async {
    Log.info(_logTag, 'Register');

    if (!_online) {
      throw Exception('NotConnectedSocketException');
    }

    final Port port = getPort(PortType.main);

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.register.id);

    parcel.writeString(name);
    cb.serialize(parcel);
    _delegateList.add(cb);

    port.send(parcel);

    late Parcel parcelReceived;
    while (true) {
      parcelReceived = await _consumeCommand(port);
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

  /// This method is used to send 'Unregister' request to the stub app.
  Future<void> unregister() async {
    Log.info(_logTag, 'Unregister');

    if (!_online) {
      throw Exception('NotConnectedSocketException');
    }

    final Port port = getPort(PortType.main);

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.unregister.id);

    port.send(parcel);
  }

  /// This method is used to send 'Send' request to the stub app.
  Future<int> send(String msg) async {
    Log.info(_logTag, 'Send');

    if (!_online) {
      throw Exception('NotConnectedSocketException');
    }

    final Port port = getPort(PortType.main);

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.header;
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.send.id);

    parcel.writeString(msg);

    port.send(parcel);

    late Parcel parcelReceived;
    while (true) {
      parcelReceived = await _consumeCommand(port);
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
