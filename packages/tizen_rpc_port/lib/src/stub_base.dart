import 'dart:async';
import 'dart:typed_data';

import 'package:tizen_log/tizen_log.dart';
import 'parcel.dart';
import 'port.dart';
import 'rpc_port_method_channel.dart';

const String _logTag = 'RPC_PORT';

final MethodChannelRpcPort _methodChannel = MethodChannelRpcPort.instance;

Finalizer<StubBase> _finalizer = Finalizer<StubBase>((StubBase stub) {
  _methodChannel.stubDestroy(stub.portName);
});

/// The base of RpcPort stub class.
abstract class StubBase {
  /// The constructor of StubBase.
  StubBase(this.portName) {
    _methodChannel.stubCreate(portName);
    _finalizer.attach(this, this);
  }

  /// The port name of connection with proxy.
  final String portName;

  late final StreamSubscription<dynamic> _streamSubscription;
  bool _isListening = false;

  /// Sets trusted flag.
  Future<void> setTrusted(bool trusted) async {
    await _methodChannel.stubSetTrusted(portName, trusted);
  }

  /// Adds privilege required when try to connect on proxy.
  Future<void> addPrivilege(String privilege) async {
    await _methodChannel.stubAddPrivilege(portName, privilege);
  }

  /// Listens to signal of connection request from the proxy.
  Future<void> listen() async {
    if (_isListening) {
      return;
    }

    final Stream<dynamic> stream = await _methodChannel.stubListen(portName);
    _streamSubscription = stream.listen((dynamic event) async {
      if (event is Map) {
        final Map<dynamic, dynamic> map = event;
        final String eventName = map['event'] as String;
        final String sender = map['sender'] as String;
        final String instance = map['instance'] as String;

        Log.info(
            _logTag, 'event: $eventName, sender: $sender, instance: $instance');
        if (eventName == 'connected') {
          await onConnectedEvent(sender, instance);
        } else if (eventName == 'disconnected') {
          await onDisconnectedEvent(sender, instance);
        } else if (eventName == 'received') {
          final Uint8List rawData = map['rawData'] as Uint8List;
          final Parcel parcel = Parcel.fromRaw(rawData);
          await onReceivedEvent(sender, instance, parcel);
        } else {
          Log.error(_logTag, 'Unknown event; $eventName');
        }
      }
    });
    _isListening = true;
  }

  /// Gets the port connected with proxy that has instance id.
  Port getPort(String instance, PortType portType) {
    return Port.fromProxy(
        portName: portName, instance: instance, portType: portType);
  }

  /// The callback function that is invoked when be connected with the proxy.
  Future<void> onConnectedEvent(String sender, String instance);

  /// The callback function that is invoked when be disconnected with the proxy.
  Future<void> onDisconnectedEvent(String sender, String instance);

  /// The callback function that is invoked when receive data from the proxy.
  Future<void> onReceivedEvent(String sender, String instance, Parcel parcel);
}
