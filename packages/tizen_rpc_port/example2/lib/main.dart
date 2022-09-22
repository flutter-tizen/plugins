import 'dart:async';

import 'package:flutter/material.dart';
import 'package:tizen_log/tizen_log.dart';

import 'message_stub.dart';

String _logTag = 'RpcPortStubExample';

void main() {
  runApp(const MyApp());
}

/// Service fo Message class.
class Service extends ServiceBase {
  /// Constructor of Service.
  Service(super.sender, super.instance);

  String _name = '';
  NotifyCB? _cb;

  @override
  Future<void> onCreate() async {
    Log.info(_logTag, 'onCreate. instance: $instance');
  }

  @override
  Future<void> onTerminate() async {
    Log.info(_logTag, 'onTerminate. instance: $instance');
  }

  @override
  Future<int> onRegister(String name, NotifyCB cb) async {
    Log.info(_logTag, 'register. instance: $instance, name: $name');
    _name = name;
    _cb = cb;
    return 0;
  }

  @override
  Future<int> onSend(String msg) async {
    Log.info(_logTag, 'send. instance: $instance, msg: $msg');
    _cb?.invoke(_name, msg);
    return 0;
  }

  @override
  Future<void> onUnregister() async {
    Log.info(_logTag, 'unregister. instance: $instance');
    _cb = null;
  }
}

/// Message Stub class.
class MyMessageStub extends Message {
  /// Constructor of MyMessageStub.
  MyMessageStub() : super();

  @override
  ServiceBase createInstance(String sender, String instance) =>
      Service(sender, instance);
}

/// exmaple2 app class.
class MyApp extends StatefulWidget {
  /// Constructor of MyApp.
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  MyMessageStub? _stub;

  static void serialize(dynamic param) {
    switch (param.runtimeType) {
      case List<String>:
        {
          Log.info(_logTag, 'List<String>');
          break;
        }
    }
  }

  @override
  void initState() {
    final List<String> stringList = <String>[];
    serialize(stringList);

    super.initState();
    Log.info(_logTag, 'initState');
    _stub = MyMessageStub();
    _stub?.listen();
    initPlatformState();
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    const String platformVersion = '7.0';

    if (!mounted) {
      return;
    }

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Text('Running on: $_platformVersion\n'),
        ),
      ),
    );
  }
}
