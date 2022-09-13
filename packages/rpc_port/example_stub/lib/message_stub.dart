import 'package:rpc_port/tizen_rpc_port.dart';
import 'package:tizen_log/tizen_log.dart';

String _logTag = 'MessageStub';

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

abstract class ServiceBase {
  final String sender;
  final String instance;
  Port? _port;

  ServiceBase(this.sender, this.instance);

  void disconnect() {
    _port?.disconnect();
    _port = null;
  }

  void onCreate();
  void onTerminate();
  int onRegister(String name, NotifyCB cb);
  void onUnregister();
  int onSend(String msg);
}

class CallbackBase extends Parcelable {
  int id;
  bool once;
  int seqId = 0;
  static int _seqNum = 0;

  CallbackBase(this.id, this.once) {
    seqId = _seqNum++;
  }

  String get tag => '$id::$seqId';

  @override
  void serialize(Parcel parcel) {
    parcel.writeInt32(id);
    parcel.writeInt32(seqId);
    parcel.writeBool(once);
  }

  @override
  void deserialize(Parcel parcel) {
    id = parcel.readInt32();
    seqId = parcel.readInt32();
    once = parcel.readBool();
  }
}

class NotifyCB extends CallbackBase {
  final Port? _port;
  ServiceBase service;
  bool _valid = true;

  NotifyCB(this._port, this.service) : super(_DelegateId.notifyCB.code, false);

  void invoke(String sender, String msg) {
    Log.info(_logTag, "invoke");
    if (_port == null) {
      Log.error(_logTag, "port is null");
      throw Exception('NotConnectedSocketException');
    }

    if (once && !_valid) {
      Log.error(_logTag, "invalid");
      throw Exception('InvalidCallbackException');
    }

    Parcel parcel = Parcel();
    parcel.writeInt32(_MethodId.callback.code);
    serialize(parcel);
    parcel.writeString(sender);
    parcel.writeString(msg);

    _port?.send(parcel);
    _valid = false;
    parcel.dispose();
  }
}

abstract class MessageStub extends StubBase {
  List<ServiceBase> services = [];
  final Map<int, dynamic> _methodHandlers = {};

  MessageStub() : super('Message') {
    _methodHandlers[_MethodId.register.code] = onRegisterMethod;
    _methodHandlers[_MethodId.unregister.code] = onUnregisterMethod;
    _methodHandlers[_MethodId.send.code] = onSendMethod;
  }

  ServiceBase createInstance(String sender, String instance);

  @override
  void onConnectedEvent(String sender, String instance) {
    Log.info(_logTag, "OnConnectedEvent. sender: $sender, instance: $instance");
    var port = getPort(instance, PortType.callback);
    ServiceBase service = createInstance(sender, instance);
    service._port = port;
    service.onCreate();
    services.add(service);
  }

  @override
  void onDisconnectedEvent(String sender, String instance) {
    Log.info(
        _logTag, "onDisconnectedEvent. sender: $sender, instance: $instance");
    for (var service in services) {
      if (service.instance == instance) {
        service.onTerminate();
        services.remove(service);
        break;
      }
    }
  }

  void onRegisterMethod(ServiceBase service, Port port, Parcel parcel) {
    Log.info(_logTag, "Register");
    String name = parcel.readString();
    NotifyCB cb = NotifyCB(service._port, service);
    cb.deserialize(parcel);
    var ret = service.onRegister(name, cb);

    Parcel result = Parcel();
    ParcelHeader header = parcel.getHeader();
    ParcelHeader resultHeader = result.getHeader();
    resultHeader.tag = "1.0";
    resultHeader.sequenceNumber = header.sequenceNumber;
    result.writeInt32(_MethodId.result.code);
    result.writeInt32(ret);
    port.send(result);
    result.dispose();
  }

  void onUnregisterMethod(ServiceBase service, Port port, Parcel parcel) {
    Log.info(_logTag, "Unregister");
    service.onUnregister();
  }

  void onSendMethod(ServiceBase service, Port port, Parcel parcel) {
    Log.info(_logTag, "Send");
    String msg = parcel.readString();
    var ret = service.onSend(msg);

    Parcel result = Parcel();
    ParcelHeader header = parcel.getHeader();
    ParcelHeader resultHeader = result.getHeader();
    resultHeader.tag = "1.0";
    resultHeader.sequenceNumber = header.sequenceNumber;
    result.writeInt32(_MethodId.result.code);
    result.writeInt32(ret);
    port.send(result);
    result.dispose();
  }

  @override
  void onReceivedEvent(String sender, String instance, Parcel parcel) {
    Log.info(_logTag, "onReceivedEvent. sender: $sender, instance: $instance");
    ServiceBase? service;
    for (var s in services) {
      if (s.instance == instance) {
        service = s;
        break;
      }
    }

    if (service == null) {
      Log.error(_logTag, 'service is null');
      return;
    }

    var port = getPort(instance, PortType.main);
    int cmd = parcel.readInt32();
    if (_methodHandlers.containsKey(cmd)) {
      _methodHandlers[cmd](service, port, parcel);
    } else {
      Log.error(_logTag, "Unknown cmd: $cmd");
    }
  }
}
