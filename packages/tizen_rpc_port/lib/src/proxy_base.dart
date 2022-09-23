import 'dart:async';
import 'dart:typed_data';

import 'package:tizen_log/tizen_log.dart';

import 'parcel.dart';
import 'port.dart';
import 'rpc_port_method_channel.dart';

const String _logTag = 'RPC_PORT';

final MethodChannelRpcPort _methodChannel = MethodChannelRpcPort.instance;

Finalizer<ProxyBase> _finalizer = Finalizer<ProxyBase>((ProxyBase proxy) {
  _methodChannel.proxyDestroy(proxy);
});

/// The base of RpcPort proxy class.
abstract class ProxyBase {
  /// The constructor of ProxyBase.
  ProxyBase(this.appid, this.portName) {
    _finalizer.attach(this, this);
  }

  /// The appid of stub app.
  final String appid;

  /// The port name of connection with stub.
  final String portName;

  bool _connected = false;
  StreamSubscription<dynamic>? _streamSubscription;

  Future<void> _handleEvent(dynamic event) async {
    if (event is Map) {
      final Map<dynamic, dynamic> map = event;
      if (map.containsKey('event')) {
        final String event = map['event'] as String;
        final String appid = map['receiver'] as String;
        final String portName = map['portName'] as String;
        Log.info(_logTag, 'event: $event, appid:$appid, portName:$portName');
        if (event == 'connected') {
          _connected = true;
          await onConnectedEvent();
        } else if (event == 'disconnected') {
          _connected = false;
          await onDisconnectedEvent();
          _streamSubscription?.cancel();
          _streamSubscription = null;
        } else if (event == 'rejected') {
          _connected = false;
          final String error = map['error'] as String;
          await onRejectedEvent(error);
          _streamSubscription?.cancel();
          _streamSubscription = null;
        } else if (event == 'received') {
          final Uint8List rawData = map['rawData'] as Uint8List;
          final Parcel parcel = Parcel.fromRaw(rawData);
          await onReceivedEvent(parcel);
        } else {
          throw Exception('Not supported event');
        }
      }
    }
  }

  /// Connects to the stub.
  Future<void> connect() async {
    if (_connected) {
      throw Exception('Proxy $appid/$portName already connected to stub');
    }

    final Stream<dynamic> stream = await _methodChannel.proxyConnect(this);
    _streamSubscription = stream.listen(_handleEvent);
  }

  /// Gets a port.
  Port getPort(PortType portType) {
    return Port.fromStub(appid: appid, portName: portName, portType: portType);
  }

  /// The callback function that is invoked when be connected with the stub.
  Future<void> onConnectedEvent();

  /// The callback function that is invoked when be disconnected with the stub.
  Future<void> onDisconnectedEvent();

  /// The callback function that is invoked when connection is failed.
  Future<void> onRejectedEvent(String errorMessage);

  /// The callback function that is invoked when receive data from the stub.
  Future<void> onReceivedEvent(Parcel parcel);
}
