import 'package:flutter/services.dart';
import 'dart:typed_data';

import 'proxy_base.dart';
import 'stub_base.dart';
import 'rpc_port_platform_interface.dart';

/// An implementation of [RpcPortPlatform] that uses method channels.
class MethodChannelRpcPort extends RpcPortPlatform {
  /// The method channel used to interact with the native platform.
  final _channel = const MethodChannel('tizen/rpc_port');

  final _streams = <String, Stream<dynamic>>{};

  String createKey(String appid, String portName) => "$appid/$portName";

  @override
  Future<void> create(String portName) async {
    final Map<String, String> args = {'portName': portName};
    return _channel.invokeMethod('create', args);
  }

  @override
  Future<void> destroy(String portName) async {
    final Map<String, String> args = {'portName': portName};
    return _channel.invokeMethod('destroy', args);
  }

  @override
  Stream<dynamic> connect(ProxyBase proxy) {
    final key = createKey(proxy.appid, proxy.portName);
    if (_streams.containsKey(key)) {
      return _streams[key]!;
    }

    final eventChannel = EventChannel('tizen/rpc_port_proxy/$key');
    _streams[key] = eventChannel.receiveBroadcastStream();
    final args = <String, dynamic>{
      'appid': proxy.appid,
      'portName': proxy.portName,
    };

    _channel.invokeMethod('connect', args);
    return _streams[key]!;
  }

  @override
  Stream<dynamic> connectSync(ProxyBase proxy) {
    final key = createKey(proxy.appid, proxy.portName);
    if (_streams.containsKey(key)) {
      return _streams[key]!;
    }

    final eventChannel = EventChannel('tizen/rpc_port_proxy/$key');
    _streams[key] = eventChannel.receiveBroadcastStream();
    final args = <String, dynamic>{
      'appid': proxy.appid,
      'portName': proxy.portName,
    };

    _channel.invokeMethod('connectSync', args);
    return _streams[key]!;
  }

  @override
  Future<void> proxyDisconnect(ProxyPort port) async {
    final key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'portType': port.portType.index,
      };

      _channel.invokeMethod('proxyDisconnect', args);
    }
  }

  @override
  Future<dynamic> proxySend(ProxyPort port, Uint8List raw) async {
    final Map<String, dynamic> args = {
      'appid': port.appid,
      'portName': port.portName,
      'portType': port.portType.index,
      'rawData': raw
    };

    return _channel.invokeMethod('proxySend', args);
  }

  @override
  Future<Uint8List> proxyReceive(ProxyPort port) async {
    final Map<String, dynamic> args = {
      'appid': port.appid,
      'portName': port.portName,
      'portType': port.portType.index,
    };
    return (await _channel.invokeMethod('proxyReceive', args)) as Uint8List;
  }

  @override
  Future<void> proxySetPrivateSharingArray(
      ProxyPort port, List<String> paths) async {
    final key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'paths': paths,
      };

      return _channel.invokeMethod('proxySetPrivateSharingArray', args);
    }
  }

  @override
  Future<void> proxySetPrivateSharing(ProxyPort port, String path) async {
    final key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'path': path,
      };

      return _channel.invokeMethod('proxySetPrivateSharing', args);
    }
  }

  @override
  Future<void> proxyUnsetPrivateSharing(ProxyPort port) async {
    final key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
      };

      return _channel.invokeMethod('proxyUnsetPrivateSharing', args);
    }
  }

  @override
  Future<void> stubDisconnect(StubPort port) async {
    if (_streams.containsKey(port.portName)) {
      final args = <String, dynamic>{
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
      };

      _channel.invokeMethod('destroy', args);
    }
  }

  @override
  Future<dynamic> stubSend(StubPort port, Uint8List raw) async {
    final Map<String, dynamic> args = {
      'portName': port.portName,
      'instance': port.instance,
      'portType': port.portType.index,
      'rawData': raw
    };

    return _channel.invokeMethod('stubSend', args);
  }

  @override
  Future<Uint8List> stubReceive(StubPort port) async {
    final Map<String, dynamic> args = {
      'portName': port.portName,
      'instance': port.instance,
      'portType': port.portType.index,
    };
    return (await _channel.invokeMethod('stubReceive', args)) as Uint8List;
  }

  @override
  Future<void> stubSetPrivateSharingArray(
      StubPort port, List<String> paths) async {
    if (_streams.containsKey(port.portName)) {
      final args = <String, dynamic>{
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
        'paths': paths,
      };

      return _channel.invokeMethod('stubSetPrivateSharingArray', args);
    }
  }

  @override
  Future<void> stubSetPrivateSharing(StubPort port, String path) async {
    if (_streams.containsKey(port.portName)) {
      final args = <String, dynamic>{
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
        'path': path,
      };

      return _channel.invokeMethod('stubSetPrivateSharing', args);
    }
  }

  @override
  Future<void> stubUnsetPrivateSharing(StubPort port) async {
    if (_streams.containsKey(port.portName)) {
      final args = <String, dynamic>{
        'portName': port.portName,
        'instance': port.instance,
      };

      return _channel.invokeMethod('stubUnsetPrivateSharing', args);
    }
  }

  @override
  Future<void> setTrusted(String portName, bool trusted) async {
    final Map<String, dynamic> args = {
      'portName': portName,
      'trusted': trusted
    };
    return _channel.invokeMethod('setTrusted', args);
  }

  @override
  Future<void> addPrivilege(String portName, String privilege) async {
    final Map<String, String> args = {
      'portName': portName,
      'privilege': privilege
    };
    return _channel.invokeMethod('addPrivilege', args);
  }

  @override
  Stream<dynamic> listen(String portName) {
    if (_streams.containsKey(portName)) return _streams[portName]!;

    final EventChannel eventChannel =
        EventChannel('tizen/rpc_port_stub/$portName');
    _streams[portName] = eventChannel.receiveBroadcastStream();

    final Map<String, String> args = {'portName': portName};
    _channel.invokeMethod('listen', args);
    return _streams[portName]!;
  }
}
