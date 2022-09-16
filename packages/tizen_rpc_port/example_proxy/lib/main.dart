import 'dart:async';
import 'package:flutter/material.dart';

import 'package:flutter/services.dart';
import 'package:tizen_log/tizen_log.dart';

import 'message_proxy.dart';

const String _logTag = 'RpcPortProxyExample';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  State<MyApp> createState() => _MyAppState();
}

String _msg = '';

class MyNotify extends NotifyCB {
  @override
  void onReceived(String sender, String msg) {
    Log.info(_logTag, 'onReceived $sender: $msg');
    _msg = '$sender: $msg';
  }
}

class MyMessageProxy extends Message {
  MyMessageProxy(String appid) : super(appid);

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
  Future<void> onRejected() async {
    _msg = 'onRejected';
    Log.info(_logTag, 'onRejected');
    print('onRejected');
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
            child: Column(children: [
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
          persistentFooterButtons: [
            TextButton(
              onPressed: _sendMsg,
              child: const Text('Send message'),
            ),
          ] // This trailing comma makes auto-formatting nicer for build methods.
          ),
    );
  }
}
