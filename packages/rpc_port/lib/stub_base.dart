// You have generated a new plugin project without specifying the `--platforms`
// flag. A plugin project with no platform support was generated. To add a
// platform, run `flutter create -t plugin --platforms <platforms> .` under the
// same directory. You can also find a detailed instruction on how to add
// platforms in the `pubspec.yaml` at
// https://flutter.dev/docs/development/packages-and-plugins/developing-packages#plugin-platforms.

import 'dart:async';
import 'dart:typed_data';

import 'package:rpc_port/disposable.dart';
import 'package:tizen_log/tizen_log.dart';

import 'parcel.dart';
import 'rpc_port_platform_interface.dart';
import 'disposable.dart';
import 'port.dart';

const String _logTag = "RPC_PORT";

abstract class StubBase extends Disposable {
  final String _portName;
  StreamSubscription<dynamic>? _streamSubscription;
  bool _isListening = false;

  StubBase(this._portName) {
    final manager = RpcPortPlatform.instance;
    manager.create(_portName);
  }

  String get portName => _portName;

  void setTrusted(bool trusted) {
    final manager = RpcPortPlatform.instance;
    manager.setTrusted(_portName, trusted);
  }

  void addPrivilege(String privilege) {
    final manager = RpcPortPlatform.instance;
    manager.addPrivilege(_portName, privilege);
  }

  void listen() {
    if (_isListening) return;

    final manager = RpcPortPlatform.instance;
    final Stream<dynamic> stream = manager.listen(_portName);
    _streamSubscription = stream.listen((dynamic event) {
      if (event is Map) {
        final Map<dynamic, dynamic> map = event;
        final String eventName = map['event'] as String;
        final String sender = map['sender'] as String;
        final String instance = map['instance'] as String;

        Log.info(
            _logTag, "event: $eventName, sender: $sender, instance: $instance");
        if (eventName == 'connected') {
          onConnectedEvent(sender, instance);
        } else if (eventName == 'disconnected') {
          onDisconnectedEvent(sender, instance);
        } else if (eventName == 'received') {
          final Uint8List rawData = map['rawData'] as Uint8List;
          final parcel = Parcel.fromRaw(rawData);
          onReceivedEvent(sender, instance, parcel);
        } else {
          Log.error(_logTag, "Unknown event; $eventName");
        }
      }
    });
    _isListening = true;
  }

  @override
  void dispose() {
    if (isDisposed) return;

    final manager = RpcPortPlatform.instance;
    manager.destroy(_portName);
    isDisposed = true;
  }

  Port getPort(String instance, PortType portType) {
    return Port.fromProxy(_portName, instance, portType);
  }

  void onConnectedEvent(String sender, String instance);
  void onDisconnectedEvent(String sender, String instance);
  void onReceivedEvent(String sender, String instance, Parcel parcel);
}
