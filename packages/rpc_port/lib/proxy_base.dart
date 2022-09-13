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
import 'port.dart';
import 'rpc_port_platform_interface.dart';
import 'disposable.dart';

String _logTag = "RPC_PORT";

abstract class ProxyBase extends Disposable {
  ProxyBase(this._appid, this._portName);

  late final String _appid;
  late final String _portName;
  bool _connected = false;
  StreamSubscription<dynamic>? _streamSubscription;

  String get appid => _appid;
  String get portName => _portName;

  void connect() {
    if (_connected) {
      throw Exception('Proxy $_appid/$_portName already requested to stub');
    }

    final RpcPortPlatform manager = RpcPortPlatform.instance;
    final Stream<dynamic> stream = manager.connect(this);
    _streamSubscription = stream.listen((dynamic event) async {
      if (event is Map) {
        final map = event;
        if (map.containsKey('event')) {
          final String event = map['event'] as String;
          final String appid = map['receiver'] as String;
          final String portName = map['portName'] as String;
          Log.info(_logTag, 'event: $event, appid:$appid, portName:$portName');
          if (event == 'connected') {
            await onConnectedEvent(appid, portName);
            _connected = true;
          } else if (event == 'disconnected') {
            await onDisconnectedEvent(appid, portName);
            _streamSubscription?.cancel();
            _streamSubscription = null;
            _connected = false;
          } else if (event == 'rejected') {
            await onRejectedEvent(appid, portName);
            _streamSubscription?.cancel();
            _streamSubscription = null;
            _connected = false;
          } else if (event == 'received') {
            final Uint8List rawData = map['rawData'] as Uint8List;
            final Parcel parcel = Parcel.fromRaw(rawData);
            await onReceivedEvent(appid, portName, parcel);
          } else {
            throw Exception('Not supported event');
          }
        }
      }
    });
  }

  void connectSync() {
    if (_connected) {
      throw Exception('Proxy $_appid/$_portName already requested to stub');
    }

    final RpcPortPlatform manager = RpcPortPlatform.instance;
    final Stream<dynamic> stream = manager.connectSync(this);
    _streamSubscription = stream.listen((dynamic event) async {
      if (event is Map) {
        final Map<String, dynamic> map = event as Map<String, dynamic>;
        if (map.containsKey('event')) {
          final String event = map['event'] as String;
          final String appid = map['receiver'] as String;
          final String portName = map['portName'] as String;
          Log.info(_logTag, 'event: $event, appid:$appid, portName:$portName');
          if (event == 'connected') {
            await onConnectedEvent(appid, portName);
            _connected = true;
          } else if (event == 'disconnected') {
            await onDisconnectedEvent(appid, portName);
            _streamSubscription?.cancel();
            _streamSubscription = null;
            _connected = false;
          } else if (event == 'rejected') {
            await onRejectedEvent(appid, portName);
            _streamSubscription?.cancel();
            _streamSubscription = null;
            _connected = false;
          } else if (event == 'received') {
            final Uint8List rawData = map['rawData'] as Uint8List;
            final Parcel parcel = Parcel.fromRaw(rawData);
            await onReceivedEvent(appid, portName, parcel);
          } else {
            throw Exception('Not supported event');
          }
        }
      }
    });
  }

  Port getPort(PortType portType) {
    return Port.fromStub(_appid, _portName, portType);
  }

  /// virtual functions
  Future<void> onConnectedEvent(String appid, String portName);
  Future<void> onDisconnectedEvent(String appid, String portName);
  Future<void> onRejectedEvent(String appid, String portName);
  Future<void> onReceivedEvent(String appid, String portName, Parcel parcel);
}
