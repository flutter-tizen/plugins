// You have generated a new plugin project without specifying the `--platforms`
// flag. A plugin project with no platform support was generated. To add a
// platform, run `flutter create -t plugin --platforms <platforms> .` under the
// same directory. You can also find a detailed instruction on how to add
// platforms in the `pubspec.yaml` at
// https://flutter.dev/docs/development/packages-and-plugins/developing-packages#plugin-platforms.

import 'dart:async';
import 'dart:typed_data';

import 'package:tizen_log/tizen_log.dart';
import 'disposable.dart';
import 'parcel.dart';
import 'port.dart';
import 'rpc_port_method_channel.dart';

const String _logTag = 'RPC_PORT';

Finalizer<StubBase> _finalizer = Finalizer<StubBase>((StubBase stub) {
  stub.dispose();
});

/// The base of RpcPort stub class.
abstract class StubBase extends Disposable {
  /// The constructor of StubBase.
  StubBase(this._portName) {
    final MethodChannelRpcPort manager = MethodChannelRpcPort.instance;
    manager.stubCreate(_portName);
  }

  final String _portName;
  late final StreamSubscription<dynamic> _streamSubscription;
  bool _isListening = false;

  /// The port name of connection with proxy.
  String get portName => _portName;

  /// Sets trusted flag.
  Future<void> setTrusted(bool trusted) async {
    final MethodChannelRpcPort manager = MethodChannelRpcPort.instance;
    await manager.stubSetTrusted(_portName, trusted);
  }

  /// Adds privilege required when try to connect on proxy.
  Future<void> addPrivilege(String privilege) async {
    final MethodChannelRpcPort manager = MethodChannelRpcPort.instance;
    await manager.stubAddPrivilege(_portName, privilege);
  }

  /// Listens to signal of connection request from the proxy.
  Future<void> listen() async {
    if (_isListening) {
      return;
    }

    final MethodChannelRpcPort manager = MethodChannelRpcPort.instance;
    final Stream<dynamic> stream = await manager.stubListen(_portName);
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

  @override
  void dispose() {
    if (isDisposed) {
      return;
    }

    final MethodChannelRpcPort manager = MethodChannelRpcPort.instance;
    _finalizer.detach(this);
    manager.stubDestroy(_portName);
    isDisposed = true;
  }

  /// Gets the port connected with proxy that has instance id.
  Port getPort(String instance, PortType portType) {
    return Port.fromProxy(_portName, instance, portType);
  }

  /// The callback function that is invoked when be connected with the proxy.
  Future<void> onConnectedEvent(String sender, String instance);

  /// The callback function that is invoked when be disconnected with the proxy.
  Future<void> onDisconnectedEvent(String sender, String instance);

  /// The callback function that is invoked when receive data from the proxy.
  Future<void> onReceivedEvent(String sender, String instance, Parcel parcel);
}
