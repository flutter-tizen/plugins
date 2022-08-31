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

class ProxyPort extends Port {
  final String _appid;
  final String _portName;

  ProxyPort(this._appid, this._portName, PortType portType) : super(portType);

  String get appid => _appid;

  String get portName => _portName;

  @override
  Future<void> disconnect() async {
    final manager = RpcPortPlatform.instance;
    await manager.proxyDisconnect(this);
  }

  @override
  Future<void> send(Parcel parcel) async {
    final manager = RpcPortPlatform.instance;
    return manager.proxySend(this, parcel.raw as Uint8List);
  }

  @override
  Future<Parcel> receive() async {
    final manager = RpcPortPlatform.instance;
    return Parcel.fromRaw(await manager.proxyReceive(this));
  }

  @override
  Future<void> setPrivateSharingList(List<String> paths) async {
    final manager = RpcPortPlatform.instance;
    await manager.proxySetPrivateSharingArray(this, paths);
  }

  @override
  Future<void> setPrivateSharing(String path) async {
    final manager = RpcPortPlatform.instance;
    await manager.proxySetPrivateSharing(this, path);
  }

  @override
  Future<void> unsetPrivateSharing() async {
    final manager = RpcPortPlatform.instance;
    await manager.proxyUnsetPrivateSharing(this);
  }
}

abstract class ProxyBase extends Disposable {
  late final String _appid;
  late final String _portName;
  bool _connected = false;
  StreamSubscription<dynamic>? _streamSubscription;

  ProxyBase(this._appid, this._portName);

  String get appid => _appid;
  String get portName => _portName;

  void connect() {
    if (_connected) {
      throw Exception('Proxy $_appid/$_portName already requested to stub');
    }

    final manager = RpcPortPlatform.instance;
    final Stream<dynamic> stream = manager.connect(this);
    _streamSubscription = stream.listen((event) async {
      if (event is Map) {
        final map = event;
        if (map.containsKey('event')) {
          final event = map['event'] as String;
          final appid = map['receiver'] as String;
          final portName = map['portName'] as String;
          Log.info(_logTag, "event: $event, appid:$appid, portName:$portName");
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
            final rawData = map['rawData'] as Uint8List;
            final parcel = Parcel.fromRaw(rawData);
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

    final manager = RpcPortPlatform.instance;
    final Stream<dynamic> stream = manager.connectSync(this);
    _streamSubscription = stream.listen((event) async {
      if (event is Map) {
        final map = event;
        if (map.containsKey('event')) {
          final event = map['event'] as String;
          final appid = map['receiver'] as String;
          final portName = map['portName'] as String;
          Log.info(_logTag, "event: $event, appid:$appid, portName:$portName");
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
            final rawData = map['rawData'] as Uint8List;
            final parcel = Parcel.fromRaw(rawData);
            await onReceivedEvent(appid, portName, parcel);
          } else {
            throw Exception('Not supported event');
          }
        }
      }
    });
  }

  ProxyPort getPort(PortType portType) {
    return ProxyPort(_appid, _portName, portType);
  }

  /// virtual functions
  Future<void> onConnectedEvent(String appid, String portName);
  Future<void> onDisconnectedEvent(String appid, String portName);
  Future<void> onRejectedEvent(String appid, String portName);
  Future<void> onReceivedEvent(String appid, String portName, Parcel parcel);
}
