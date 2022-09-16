import 'dart:typed_data';
import 'package:flutter/services.dart';
import 'package:tizen_log/tizen_log.dart';

import 'port.dart';
import 'proxy_base.dart';

const String _logTag = 'RpcPortMethodChannel';

/// An implementation of [RpcPortPlatform] that uses method channels.
class MethodChannelRpcPort {
  static final MethodChannelRpcPort instance = MethodChannelRpcPort();

  /// The method channel used to interact with the native platform.
  final MethodChannel _channel = const MethodChannel('tizen/rpc_port');

  final Map<String, Stream<dynamic>> _streams = <String, Stream<dynamic>>{};

  String _createKey(String appid, String portName) => '$appid/$portName';

  Future<void> stubCreate(String portName) async {
    final Map<String, String> args = <String, String>{'portName': portName};
    return await _channel.invokeMethod('stubCreate', args);
  }

  Future<void> stubDestroy(String portName) async {
    final Map<String, String> args = <String, String>{'portName': portName};
    return await _channel.invokeMethod('stubDestroy', args);
  }

  Stream<dynamic> proxyConnect(ProxyBase proxy) {
    final String key = _createKey(proxy.appid, proxy.portName);
    if (_streams.containsKey(key)) {
      return _streams[key]!;
    }

    final EventChannel eventChannel = EventChannel('tizen/rpc_port_proxy/$key');
    _streams[key] = eventChannel.receiveBroadcastStream();
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': proxy.appid,
      'portName': proxy.portName,
    };

    _channel.invokeMethod<dynamic>('proxyConnect', args);
    return _streams[key]!;
  }

  Stream<dynamic> proxyConnectSync(ProxyBase proxy) {
    final String key = _createKey(proxy.appid, proxy.portName);
    if (_streams.containsKey(key)) {
      return _streams[key]!;
    }

    final EventChannel eventChannel = EventChannel('tizen/rpc_port_proxy/$key');
    _streams[key] = eventChannel.receiveBroadcastStream();
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': proxy.appid,
      'portName': proxy.portName,
    };

    _channel.invokeMethod<dynamic>('proxyConnectSync', args);
    return _streams[key]!;
  }

  Future<void> proxyDestroy(ProxyBase proxy) async {
    final String key = _createKey(proxy.appid, proxy.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': proxy.appid,
        'portName': proxy.portName,
      };

      return await _channel.invokeMethod<void>('proxyDestroy', args);
    }
  }

  Future<void> portDisconnect(Port port) async {
    final String key = _createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
      };

      return await _channel.invokeMethod<void>('portDisconnect', args);
    }
  }

  Future<dynamic> portSend(Port port, Uint8List raw) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': port.appid,
      'portName': port.portName,
      'instance': port.instance,
      'portType': port.portType.index,
      'rawData': raw
    };

    return await _channel.invokeMethod<dynamic>('portSend', args);
  }

  Future<Uint8List> portReceive(Port port) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'appid': port.appid,
      'portName': port.portName,
      'instance': port.instance,
      'portType': port.portType.index,
    };
    final Uint8List? ret;

    try {
      ret = await _channel.invokeMethod<Uint8List>('portReceive', args);
      if (ret == null) {
        throw Exception('Receive is failed');
      }
    } catch (e) {
      Log.error(_logTag, e.toString());
      return Uint8List(0);
    }

    return ret;
  }

  Future<void> portSetPrivateSharingArray(Port port, List<String> paths) async {
    final String key = _createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
        'paths': paths,
      };

      return await _channel.invokeMethod('portSetPrivateSharingArray', args);
    }
  }

  Future<void> portSetPrivateSharing(Port port, String path) async {
    final String key = _createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
        'path': path,
      };

      return await _channel.invokeMethod('portSetPrivateSharing', args);
    }
  }

  Future<void> portUnsetPrivateSharing(Port port) async {
    final String key = _createKey(port.appid, port.portName);
    if (_streams.containsKey(key)) {
      final Map<String, dynamic> args = <String, dynamic>{
        'appid': port.appid,
        'portName': port.portName,
        'instance': port.instance,
        'portType': port.portType.index,
      };

      return await _channel.invokeMethod('portUnsetPrivateSharing', args);
    }
  }

  Future<void> stubSetTrusted(String portName, bool trusted) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'portName': portName,
      'trusted': trusted
    };

    return await _channel.invokeMethod('stubSetTrusted', args);
  }

  @override
  Future<void> stubAddPrivilege(String portName, String privilege) async {
    final Map<String, String> args = <String, String>{
      'portName': portName,
      'privilege': privilege
    };
    return await _channel.invokeMethod('stubAddPrivilege', args);
  }

  Stream<dynamic> stubListen(String portName) {
    if (_streams.containsKey(portName)) {
      return _streams[portName]!;
    }

    final EventChannel eventChannel =
        EventChannel('tizen/rpc_port_stub/$portName');
    _streams[portName] = eventChannel.receiveBroadcastStream();

    final Map<String, String> args = <String, String>{'portName': portName};
    _channel.invokeMethod<dynamic>('stubListen', args);
    return _streams[portName]!;
  }
}
