// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:tizen_log/tizen_log.dart';

import 'message_stub.dart';

String _logTag = 'RpcPortStubExample';

void main() {
  runApp(const MyApp());
}

class Service extends ServiceBase {
  Service(super.sender, super.instance);

  String _name = '';
  NotifyCallback? _callback;

  @override
  Future<void> onCreate() async {
    Log.info(_logTag, 'onCreate. instance: $instance');
  }

  @override
  Future<void> onTerminate() async {
    Log.info(_logTag, 'onTerminate. instance: $instance');
  }

  @override
  Future<int> onRegister(String name, NotifyCallback callback) async {
    Log.info(_logTag, 'register. instance: $instance, name: $name');
    _name = name;
    _callback = callback;
    return 0;
  }

  @override
  Future<int> onSend(String message) async {
    Log.info(_logTag, 'send. instance: $instance, msg: $message');
    _callback?.invoke(_name, message);
    return 0;
  }

  @override
  Future<void> onUnregister() async {
    Log.info(_logTag, 'unregister. instance: $instance');
    _callback = null;
  }
}

class MyMessageStub extends Message {
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
  static const String _platformVersion = '7.0';
  late final MyMessageStub _stub;

  @override
  void initState() {
    super.initState();
    Log.info(_logTag, 'initState');
    _stub = MyMessageStub();
    _stub.listen();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('RpcPortStub example app'),
        ),
        body: const Center(
          child: Text('Running on: $_platformVersion'),
        ),
      ),
    );
  }
}
