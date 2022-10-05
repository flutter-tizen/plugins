// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';
import 'package:flutter/material.dart';

import 'package:flutter/services.dart';
import 'package:tizen_log/tizen_log.dart';

import 'message_proxy.dart';

const String _logTag = 'TizenRpcPortProxyExample';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

String _message = '';

class MyNotify extends NotifyCallback {
  @override
  Future<void> onReceived(String sender, String message) async {
    Log.info(_logTag, 'Received $sender: $message');
    _message = '$sender: $message';
  }
}

class MyMessageProxy extends Message {
  MyMessageProxy(super.appid);

  String get message => _message;
  set message(String message) => _message = message;

  @override
  Future<void> onConnected() async {
    _message = 'Connected';
    await register('ClientApp', MyNotify());
  }

  @override
  Future<void> onDisconnected() async {
    _message = 'Disconnected';
    await connect();
  }

  @override
  Future<void> onRejected(String errorMessage) async {
    _message = 'Rejected error: $errorMessage';
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
      _myProxy = MyMessageProxy('com.example.tizen_rpc_port_stub_example');
      await _myProxy.connect();
    } on PlatformException {
      _message = 'connect() is failed.';
    }

    if (!mounted) {
      return;
    }

    setState(() {
      _message = _myProxy.message;
    });
  }

  Future<void> _sendMsg() async {
    await _myProxy.send(_input);
  }

  Future<void> _registerCallback() async {
    await _myProxy.register('ClientApp', MyNotify());
    setState(() {
      _message = 'register callback done.';
      Log.info(_logTag, 'register callback done.');
    });
  }

  Future<void> _unregisterCallback() async {
    await _myProxy.unregister();
    setState(() {
      _message = 'Unregister callback done.';
      Log.info(_logTag, 'Unregister callback done.');
    });
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
                child: Text('Message: $_message\n'),
              ),
              TextField(
                  onChanged: (String text) => setState(() => _input = text),
                  decoration: const InputDecoration(
                    border: OutlineInputBorder(),
                    labelText: 'Input',
                  )),
            ]),
          ),
          persistentFooterButtons: <Widget>[
            TextButton(
              onPressed: _sendMsg,
              child: const Text('Send'),
            ),
            TextButton(
              onPressed: _registerCallback,
              child: const Text('Register'),
            ),
            TextButton(
              onPressed: _unregisterCallback,
              child: const Text('Unregister'),
            ),
          ]),
    );
  }
}
