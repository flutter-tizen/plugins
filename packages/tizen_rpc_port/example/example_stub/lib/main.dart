// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:tizen_log/tizen_log.dart';

import 'message_stub.dart';

// ignore_for_file: public_member_api_docs

String _logTag = 'RpcPortStubExample';

void main() {
  runApp(const MyApp());
}

class Service extends ServiceBase {
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

class MyMessageStub extends Message {
  MyMessageStub() : super();

  @override
  ServiceBase createInstance(String sender, String instance) =>
      Service(sender, instance);
}

class MyApp extends StatefulWidget {
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
