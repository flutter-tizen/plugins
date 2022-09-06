import 'dart:async';

import 'package:rpc_port/rpc_port.dart';

const String _logTag = 'MessageProxy';

enum _DelegateId {
  notifyCB(1);

  const _DelegateId(this.code);
  final int code;
}

enum _MethodId {
  result(0),
  callback(1),
  register(2),
  unregister(3),
  send(4);

  const _MethodId(this.code);
  final int code;
}

abstract class MessageProxy extends ProxyBase {
  MessageProxy(String appId) : super(appId, "Message");

  static const String _tidlVersion = '1.9.1';
  bool _online = false;
  final List<_CallbackBase> _delegateList = <_CallbackBase>[];

  @override
  Future<void> onConnectedEvent(String appid, String portName) async {
    _online = true;
    onConnected();
  }

  @override
  Future<void> onDisconnectedEvent(String appid, String portName) async {
    _online = false;
    onDisconnected();
  }

  @override
  Future<void> onRejectedEvent(String appid, String portName) async {
    onRejected();
  }

  @override
  Future<void> onReceivedEvent(
      String appid, String portName, Parcel parcel) async {
    final int cmd = parcel.readInt32();
    if (cmd != _MethodId.callback.code) {
      parcel.dispose();
      return;
    }

    _processReceivedEvent(parcel);
  }

  Future<void> _processReceivedEvent(Parcel parcel) async {
    final int id = parcel.readInt32();
    final int seqId = parcel.readInt32();
    final bool once = parcel.readBool();

    for (final _CallbackBase delegate in _delegateList) {
      if (delegate._id == id && delegate._seqId == seqId) {
        await delegate._onReceivedEvent(parcel);
        if (delegate._once) {
          _delegateList.remove(delegate);
        }
        break;
      }
    }
  }

  Future<Parcel> _consumeCommand(Port port) async {
    do {
      final Parcel parcel = await port.receive();
      final int cmd = parcel.readInt32();
      if (cmd == _MethodId.result.code) {
        return parcel;
      }

      parcel.dispose();
    } while (true);
  }

  void disposeCallback(String tag) {
    _delegateList.removeWhere((_CallbackBase element) => element.tag == tag);
  }

  Future<int> register(String name, NotifyCB cb) async {
    if (!_online) {
      throw Exception('NotConnectedSocketException');
    }

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.getHeader();
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.register.code);
    parcel.writeString(name);
    cb.serialize(parcel);

    _delegateList.add(cb);
    final Port port = getPort(PortType.main);
    await port.send(parcel);

    late Parcel parcelReceived;
    do {
      parcelReceived = await _consumeCommand(port);
      final ParcelHeader headerReceived = parcelReceived.getHeader();
      if (headerReceived.tag.isEmpty) {
        break;
      } else if (headerReceived.sequenceNumber == header.sequenceNumber) {
        break;
      }

      parcelReceived.dispose();
    } while (true);

    final int ret = parcelReceived.readInt32();
    parcel.dispose();
    parcelReceived.dispose();
    return ret;
  }

  Future<void> unregister() async {
    if (!_online) {
      throw Exception("NotConnectedSocketException");
    }

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.getHeader();
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.unregister.code);
    final Port port = getPort(PortType.main);
    await port.send(parcel);
    parcel.dispose();
  }

  Future<int> send(String msg) async {
    if (!_online) {
      throw Exception("NotConnectedSocketException");
    }

    final Parcel parcel = Parcel();
    final ParcelHeader header = parcel.getHeader();
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.send.code);
    parcel.writeString(msg);
    final Port port = getPort(PortType.main);
    await port.send(parcel);

    Parcel? parcelReceived;
    do {
      parcelReceived = await _consumeCommand(port);
      final ParcelHeader headerReceived = parcelReceived.getHeader();
      if (headerReceived.tag.isEmpty) {
        break;
      } else if (headerReceived.sequenceNumber == header.sequenceNumber) {
        break;
      }

      parcelReceived.dispose();
    } while (true);

    final int ret = parcelReceived.readInt32();
    parcel.dispose();
    parcelReceived.dispose();
    return ret;
  }

  /// virtual fucntion
  void onConnected() {}
  void onDisconnected() {}
  void onRejected() {}
}

abstract class _CallbackBase extends Parcelable {
  _CallbackBase(this._id, this._once) {
    _seqId = _seqNum++;
  }

  String get tag => '$_id::$_seqId';

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

  Future<void> _onReceivedEvent(Parcel parcel);

  late final int _id;
  late int _seqId;
  late bool _once;
  static int _seqNum = 0;
}

abstract class NotifyCB extends _CallbackBase {
  NotifyCB({bool once = false}) : super(_DelegateId.notifyCB.code, once);

  Future<void> onReceived(String sender, String msg);

  @override
  Future<void> _onReceivedEvent(Parcel parcel) async {
    final String param1 = parcel.readString();
    final String param2 = parcel.readString();
    onReceived(param1, param2);
  }
}
