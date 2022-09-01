import 'dart:async';

import 'package:rpc_port/rpc_port.dart';

const _logTag = "RpcPort";

abstract class MessageProxy extends ProxyBase {
  static const String _tidlVersion = "1.9.1";
  bool _online = false;
  final List<CallbackBase> _delegateList = [];

  MessageProxy(String appId) : super(appId, "Message");

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
    final cmd = parcel.readInt32();
    if (cmd != _MethodId.callback) {
      parcel.dispose();
      return;
    }

    _processReceivedEvent(parcel);
  }

  Future<void> _processReceivedEvent(Parcel parcel) async {
    final id = parcel.readInt32();
    final seqId = parcel.readInt32();
    final once = parcel.readBool();

    for (final delegate in _delegateList) {
      if (delegate._id == id && delegate._seqId == seqId) {
        await delegate._onReceivedEvent(parcel);
        if (delegate._once) _delegateList.remove(delegate);
        break;
      }
    }
  }

  Future<Parcel> _consumeCommand(Port port) async {
    do {
      Parcel parcel = await port.receive();
      int cmd = parcel.readInt32();
      if (cmd == _MethodId.result) return parcel;

      parcel.dispose();
    } while (true);
  }

  void disposeCallback(String tag) {
    _delegateList.removeWhere((element) => element.tag == tag);
  }

  Future<int> register(String name, NotifyCB cb) async {
    if (!_online) throw Exception('NotConnectedSocketException');

    final parcel = Parcel();
    ParcelHeader header = parcel.getHeader();
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.register);
    parcel.writeString(name);
    cb.serialize(parcel);

    _delegateList.add(cb);
    final port = getPort(PortType.main);
    port.send(parcel);

    late Parcel parcelReceived;
    do {
      parcelReceived = await _consumeCommand(port);
      final headerReceived = parcelReceived.getHeader();
      if (headerReceived.tag.isEmpty) {
        break;
      } else if (headerReceived.sequenceNumber == header.sequenceNumber) {
        break;
      }

      parcelReceived.dispose();
    } while (true);

    int ret = parcelReceived.readInt32();
    parcel.dispose();
    parcelReceived.dispose();
    return ret;
  }

  Future<void> unregister() async {
    if (!_online) throw Exception("NotConnectedSocketException");

    final parcel = Parcel();
    final header = parcel.getHeader();
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.unregister);
    final port = getPort(PortType.main);
    await port.send(parcel);
    parcel.dispose();
  }

  Future<int> send(String msg) async {
    if (!_online) throw Exception("NotConnectedSocketException");

    final parcel = Parcel();
    final header = parcel.getHeader();
    header.tag = _tidlVersion;
    parcel.writeInt32(_MethodId.send);
    parcel.writeString(msg);
    final port = getPort(PortType.main);
    port.send(parcel);

    Parcel? parcelReceived;
    do {
      parcelReceived = await _consumeCommand(port);
      final headerReceived = parcelReceived.getHeader();
      if (headerReceived.tag.isEmpty) {
        break;
      } else if (headerReceived.sequenceNumber == header.sequenceNumber) {
        break;
      }

      parcelReceived.dispose();
    } while (true);

    int ret = parcelReceived.readInt32();
    parcel.dispose();
    parcelReceived.dispose();
    return ret;
  }

  /// virtual fucntion
  void onConnected() {}
  void onDisconnected() {}
  void onRejected() {}
}

class _DelegateId {
  static const int notifyCB = 1;
}

class _MethodId {
  static const int result = 0;
  static const int callback = 1;
  static const int register = 2;
  static const int unregister = 3;
  static const int send = 4;
}

abstract class CallbackBase extends Parcelable {
  late final int _id;
  late int _seqId;
  late bool _once;
  static int _seqNum = 0;

  String get tag => "$_id::$_seqId";

  CallbackBase(this._id, this._once) {
    _seqId = _seqNum++;
  }

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

abstract class NotifyCB extends CallbackBase {
  NotifyCB({bool once = false}) : super(_DelegateId.notifyCB, once);

  Future<void> onReceived(String sender, String msg);

  @override
  Future<void> _onReceivedEvent(Parcel parcel) async {
    final appid = parcel.readString();
    final portName = parcel.readString();
    onReceived(appid, portName);
  }
}
