/// Generated by tidlc 1.9.1

import 'package:tizen_log/tizen_log.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

const String _logTag = 'RPC_PORT_STUB';
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

/// Abstract class for creating a service base class for RPC.
abstract class ServiceBase {
  /// Constructor for this class.
  ServiceBase(this.sender, this.instance);

  /// The client application ID.
  final String sender;

  /// The client instance ID.
  final String instance;

  /// The delegate port of the client.
  Port? _port;

  /// Disconnects from the client application.
  Future<void> disconnect() async {
    await _port?.disconnect();
    _port = null;
  }

  /// This abstract method will be called when the client is connected.
  Future<void> onCreate();

  /// This abstract method will be called when the client is disconnected.
  Future<void> onTerminate();

  /// This abstract method will be called when the 'Register' request is delivered.
  Future<int> onRegister(String name, NotifyCB cb);

  /// This abstract method will be called when the 'Unregister' request is delivered.
  Future<void> onUnregister();

  /// This abstract method will be called when the 'Send' request is delivered.
  Future<int> onSend(String msg);
}

/// The 'NotifyCB class to invoke the delegate method.
class NotifyCB extends CallbackBase {
  /// Constructor for this class.
  NotifyCB(this._port, this.service, {bool once = false})
      : super(_DelegateId.notifyCB.id, once);

  final Port? _port;

  /// The client service.
  ServiceBase service;
  bool _valid = true;

  /// Invokes the delegate method of the client.
  Future<void> invoke(String sender, String msg) async {
    if (_port == null) {
      Log.error(_logTag, 'port is null');
      throw Exception('NotConnectedSocketException');
    }

    if (_once && !_valid) {
      Log.error(_logTag, 'invalid');
      throw Exception('InvalidCallbackException');
    }

    final Parcel parcel = Parcel();
    parcel.writeInt32(_MethodId.callback.id);
    serialize(parcel);

    parcel.writeString(sender);
    parcel.writeString(msg);

    await _port?.send(parcel);
    _valid = false;
  }
}

/// Abstract class for creating [Message] class for RPC.
abstract class Message extends StubBase {
  /// Constructor for this class.
  Message() : super('Message') {
    _methodHandlers[_MethodId.register.id] = _onRegisterMethod;
    _methodHandlers[_MethodId.unregister.id] = _onUnregisterMethod;
    _methodHandlers[_MethodId.send.id] = _onSendMethod;
  }

  /// The indexable collection of [ServiceBase] class.
  List<ServiceBase> services = <ServiceBase>[];
  final Map<int, dynamic> _methodHandlers = <int, dynamic>{};

  /// Listens to the requests for connections.
  @override
  Future<void> listen() async {
    Log.warn(_logTag, 'listen. portName: $portName');
    return await super.listen();
  }

  /// Abstract method for creating an instance of the client.
  ServiceBase createInstance(String sender, String instance);

  @override
  Future<void> onConnectedEvent(String sender, String instance) async {
    Log.info(_logTag, 'sender: $sender, instance: $instance');
    final ServiceBase service = createInstance(sender, instance);
    service._port = getPort(instance, PortType.callback);
    await service.onCreate();
    services.add(service);
  }

  @override
  Future<void> onDisconnectedEvent(String sender, String instance) async {
    Log.info(_logTag, 'sender: $sender, instance: $instance');
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
    Log.info(_logTag, 'Register');
    final String name = parcel.readString();
    final NotifyCB cb = NotifyCB(service._port, service);
    cb.deserialize(parcel);

    final int ret = await service.onRegister(name, cb);

    final Parcel result = Parcel();
    final ParcelHeader header = parcel.header;
    final ParcelHeader resultHeader = result.header;
    resultHeader.tag = _tidlVersion;
    resultHeader.sequenceNumber = header.sequenceNumber;

    result.writeInt32(_MethodId.result.id);
    result.writeInt32(ret);

    await port.send(result);
  }

  Future<void> _onUnregisterMethod(
      ServiceBase service, Port port, Parcel parcel) async {
    Log.info(_logTag, 'Unregister');

    service.onUnregister();
  }

  Future<void> _onSendMethod(
      ServiceBase service, Port port, Parcel parcel) async {
    Log.info(_logTag, 'Send');
    final String msg = parcel.readString();

    final int ret = await service.onSend(msg);

    final Parcel result = Parcel();
    final ParcelHeader header = parcel.header;
    final ParcelHeader resultHeader = result.header;
    resultHeader.tag = _tidlVersion;
    resultHeader.sequenceNumber = header.sequenceNumber;

    result.writeInt32(_MethodId.result.id);
    result.writeInt32(ret);

    await port.send(result);
  }

  @override
  Future<void> onReceivedEvent(
      String sender, String instance, Parcel parcel) async {
    Log.info(_logTag, 'sender: $sender, instance: $instance');
    ServiceBase? service;
    for (final ServiceBase s in services) {
      if (s.instance == instance) {
        service = s;
        break;
      }
    }

    if (service == null) {
      Log.error(_logTag, 'service is null');
      return;
    }

    final Port port = getPort(instance, PortType.main);
    final int cmd = parcel.readInt32();
    if (_methodHandlers.containsKey(cmd)) {
      await _methodHandlers[cmd](service, port, parcel);
    } else {
      Log.error(_logTag, 'Unknown cmd: $cmd');
    }
  }
}