import 'dart:typed_data';
import 'package:flutter/services.dart';

import 'port.dart';
import 'proxy_base.dart';
import 'rpc_port_platform_interface.dart';

/// An implementation of [RpcPortPlatform] that uses method channels.
class MethodChannelRpcPort extends RpcPortPlatform {
  /// The method channel used to interact with the native platform.
  final _channel = const MethodChannel('tizen/rpc_port');

  final Map<String, Stream<dynamic>> _streams = <String, Stream<dynamic>>{};

  String createKey(String appid, String portName) => '$appid/$portName';

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
    final String key = createKey(proxy.appid, proxy.portName);
    if (_streams.containsKey(key)) {
      return _streams[key]!;
    }

    final EventChannel eventChannel = EventChannel('tizen/rpc_port_proxy/$key');
    _streams[key] = eventChannel.receiveBroadcastStream();
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': proxy.appid,
      'portName': proxy.portName,
    };

    _channel.invokeMethod<dynamic>('connect', args);
    return _streams[key]!;
  }

  @override
  Stream<dynamic> connectSync(ProxyBase proxy) {
    final String key = createKey(proxy.appid, proxy.portName);
    if (_streams.containsKey(key)) {
      return _streams[key]!;
    }

    final EventChannel eventChannel = EventChannel('tizen/rpc_port_proxy/$key');
    _streams[key] = eventChannel.receiveBroadcastStream();
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': proxy.appid,
      'portName': proxy.portName,
    };

    _channel.invokeMethod<dynamic>('connectSync', args);
    return _streams[key]!;
  }

  @override
  Future<void> disconnect(Port port) async {
    final String key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
      };

      _channel.invokeMethod<void>('disconnect', args);
    }
  }

  @override
  Future<dynamic> send(Port port, Uint8List raw) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': port.appid,
      'portName': port.portName,
      'instance': port.instance,
      'portType': port.portType.index,
      'rawData': raw
    };

    return _channel.invokeMethod<dynamic>('send', args);
  }

  @override
  Future<Uint8List> receive(Port port) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': port.appid,
      'portName': port.portName,
      'instance': port.instance,
      'portType': port.portType.index,
    };
    final Uint8List? ret =
        await _channel.invokeMethod<Uint8List>('receive', args);
    if (ret == null) {
      throw Exception('Receive is failed');
    }

    return ret;
  }

  @override
  Future<void> setPrivateSharingArray(Port port, List<String> paths) async {
    final String key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
        'paths': paths,
      };

      return _channel.invokeMethod('setPrivateSharingArray', args);
    }
  }

  @override
  Future<void> setPrivateSharing(Port port, String path) async {
    final String key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
        'path': path,
      };

      return _channel.invokeMethod('setPrivateSharing', args);
    }
  }

  @override
  Future<void> unsetPrivateSharing(Port port) async {
    final String key = createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
      };

      return _channel.invokeMethod('unsetPrivateSharing', args);
    }
  }

  @override
  Future<void> setTrusted(String portName, bool trusted) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'portName': portName,
      'trusted': trusted
    };

    return _channel.invokeMethod('setTrusted', args);
  }

  @override
  Future<void> addPrivilege(String portName, String privilege) async {
    final Map<String, String> args = <String, String>{
      'portName': portName,
      'privilege': privilege
    };
    return _channel.invokeMethod('addPrivilege', args);
  }

  @override
  Stream<dynamic> listen(String portName) {
    if (_streams.containsKey(portName)) {
      return _streams[portName]!;
    }

    final EventChannel eventChannel =
        EventChannel('tizen/rpc_port_stub/$portName');
    _streams[portName] = eventChannel.receiveBroadcastStream();

    final Map<String, String> args = <String, String>{'portName': portName};
    _channel.invokeMethod<dynamic>('listen', args);
    return _streams[portName]!;
  }
}
