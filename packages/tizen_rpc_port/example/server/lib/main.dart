// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';

import 'message_server.dart';

void main() {
  runApp(const MyApp());
}

class Service extends MessageServiceBase {
  Service(super.sender, super.instance);

  String _name = '';
  NotifyCallback? _callback;

  @override
  Future<void> onCreate() async {
    print('onCreate. instance: $instance');
  }

  @override
  Future<void> onTerminate() async {
    print('onTerminate. instance: $instance');
  }

  @override
  Future<int> onRegister(String name, NotifyCallback callback) async {
    print('register. instance: $instance, name: $name');
    _name = name;
    _callback = callback;
    return 0;
  }

  @override
  Future<int> onSend(String message) async {
    print('send. instance: $instance, msg: $message');
    _callback?.invoke(_name, message);
    return 0;
  }

  @override
  Future<void> onUnregister() async {
    print('unregister. instance: $instance');
    _callback = null;
  }
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  static const String _platformVersion = '7.0';
  late final Message _server;

  @override
  void initState() {
    super.initState();
    _server = Message(
        serviceBuilder: (String sender, String instance) =>
            Service(sender, instance));
    _server.listen();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('RpcPortServer example app'),
        ),
        body: const Center(
          child: Text('Running on: $_platformVersion'),
        ),
      ),
    );
  }
}
