import 'dart:async';
import 'package:flutter/material.dart';

import 'package:flutter/services.dart';
import 'package:tizen_log/tizen_log.dart';

import 'message_proxy.dart';

const String _logTag = 'RpcPortProxyExample';

void main() {
  runApp(const MyApp());
}

/// Example app class
class MyApp extends StatefulWidget {
  /// Constructor of MyApp.
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

String _msg = '';

/// MyNotify delegate class.
class MyNotify extends NotifyCB {
  @override
  Future<void> onReceived(String sender, String msg) async {
    Log.info(_logTag, 'onReceived $sender: $msg');
    _msg = '$sender: $msg';
  }
}

/// Message proxy class.
class MyMessageProxy extends Message {
  /// Constructor of MeMessageProxy.
  MyMessageProxy(super.appid);

  /// display message.
  String get msg => _msg;
  set msg(String msg) => _msg = msg;

  @override
  Future<void> onConnected() async {
    _msg = 'onConnected';
    Log.info(_logTag, 'onConnected');
    print('onConnected');

    await register('Native_GOGO1', MyNotify());
  }

  @override
  Future<void> onDisconnected() async {
    _msg = 'onDisconnected';
    Log.info(_logTag, 'onDisconnected');
    print('onDisconnected');
  }

  @override
  Future<void> onRejected(int error) async {
    _msg = 'onRejected';
    Log.info(_logTag, 'onRejected. error($error)');
    print('onRejected. error($error)');
  }
}

class _MyAppState extends State<MyApp> {
  late final MyMessageProxy _myProxy;
  String _input = '';
  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> initPlatformState() async {
    try {
      _myProxy = MyMessageProxy('com.example.rpc_port_stub_example');
      await _myProxy.connect();
    } on PlatformException {
      _msg = 'Failed to get platform version.';
    }

    if (!mounted) {
      return;
    }

    setState(() {
      _msg = _myProxy.msg;
    });
  }

  Future<void> _sendMsg() async {
    await _myProxy.send(_input);
    Future<void>.delayed(
        const Duration(seconds: 1),
        () => setState(() {
              Log.info(_logTag, 'received message: $_msg');
            }));
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
          appBar: AppBar(
            title: const Text('RpcPortProxy example app'),
          ),
          body: SingleChildScrollView(
            child: Column(children: <Widget>[
              Padding(
                padding: const EdgeInsets.fromLTRB(0.0, 50.0, 0.0, 50.0),
                child: Text('Message: $_msg\n'),
              ),
              TextField(
                  onChanged: (String text) {
                    setState(() => _input = text);
                  },
                  decoration: const InputDecoration(
                    border: OutlineInputBorder(),
                    labelText: 'Input',
                  )),
            ]),
          ),
          persistentFooterButtons: <Widget>[
            TextButton(
              onPressed: _sendMsg,
              child: const Text('Send message'),
            ),
          ] // This trailing comma makes auto-formatting nicer for build methods.
          ),
    );
  }
}
