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
import 'rpc_port_platform_interface.dart';

const String _logTag = 'RPC_PORT';

/// The base of RpcPort stub class.
abstract class StubBase extends Disposable {
  /// The constructor of StubBase.
  StubBase(this._portName) {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    manager.create(_portName);
  }

  final String _portName;
  late final StreamSubscription<dynamic> _streamSubscription;
  bool _isListening = false;

  String get portName => _portName;

  /// Sets trusted flag.
  void setTrusted(bool trusted) {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    manager.setTrusted(_portName, trusted);
  }

  /// Adds privilege required when try to connect on proxy.
  void addPrivilege(String privilege) {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    manager.addPrivilege(_portName, privilege);
  }

  /// Listens to signal of connection request from the proxy.
  void listen() {
    if (_isListening) {
      return;
    }

    final RpcPortPlatform manager = RpcPortPlatform.instance;
    final Stream<dynamic> stream = manager.listen(_portName);
    _streamSubscription = stream.listen((dynamic event) {
      if (event is Map) {
        final Map<dynamic, dynamic> map = event;
        final String eventName = map['event'] as String;
        final String sender = map['sender'] as String;
        final String instance = map['instance'] as String;

        Log.info(
            _logTag, 'event: $eventName, sender: $sender, instance: $instance');
        if (eventName == 'connected') {
          onConnectedEvent(sender, instance);
        } else if (eventName == 'disconnected') {
          onDisconnectedEvent(sender, instance);
        } else if (eventName == 'received') {
          final Uint8List rawData = map['rawData'] as Uint8List;
          final Parcel parcel = Parcel.fromRaw(rawData);
          onReceivedEvent(sender, instance, parcel);
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

    final RpcPortPlatform manager = RpcPortPlatform.instance;
    manager.destroy(_portName);
    isDisposed = true;
  }

  /// Gets the port connected with proxy that has instance id.
  Port getPort(String instance, PortType portType) {
    return Port.fromProxy(_portName, instance, portType);
  }

  /// The callback function that is invoked when be connected with the proxy.
  void onConnectedEvent(String sender, String instance);

  /// The callback function that is invoked when be disconnected with the proxy.
  void onDisconnectedEvent(String sender, String instance);

  /// The callback function that is invoked when receive data from the proxy.
  void onReceivedEvent(String sender, String instance, Parcel parcel);
}
