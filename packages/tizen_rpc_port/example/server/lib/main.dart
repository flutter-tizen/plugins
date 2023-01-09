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

class EchoService extends ServiceBase {
  EchoService(super.sender, super.instance);

  String? _name;
  NotifyCallback? _callback;

  @override
  Future<void> onCreate() async {
    print('Service started');
  }

  @override
  Future<void> onTerminate() async {
    print('Service terminated');
  }

  @override
  Future<int> onRegister(String name, NotifyCallback callback) async {
    print('Registered: $name');
    _name = name;
    _callback = callback;
    return 0;
  }

  @override
  Future<int> onSend(String message) async {
    print('Received: $message');
    if (_callback != null) {
      _callback!.invoke(_name!, message);
    }
    return 0;
  }

  @override
  Future<void> onUnregister() async {
    print('Unregistered: $_name');
    _name = null;
    _callback = null;
  }
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  late final Message _server;

  @override
  void initState() {
    super.initState();

    _server = Message(
      serviceBuilder: (String sender, String instance) =>
          EchoService(sender, instance),
    );
    _server.listen();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('TizenRpcPort Server Demo'),
        ),
        body: const Center(child: Text('Service running')),
      ),
    );
  }
}
