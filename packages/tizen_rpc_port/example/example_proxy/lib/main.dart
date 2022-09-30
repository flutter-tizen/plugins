// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:flutter/material.dart';

import 'package:flutter/services.dart';
import 'package:tizen_log/tizen_log.dart';

import 'message_proxy.dart';

// ignore_for_file: public_member_api_docs

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
    Log.info(_logTag, 'onReceived $sender: $message');
    _message = '$sender: $message';
  }
}

class MyMessageProxy extends Message {
  MyMessageProxy(super.appid);

  String get message => _message;
  set message(String message) => _message = message;

  @override
  Future<void> onConnected() async {
    _message = 'onConnected';
    Log.info(_logTag, 'onConnected');
    print('onConnected');

    await register('Native_GOGO1', MyNotify());
  }

  @override
  Future<void> onDisconnected() async {
    _message = 'onDisconnected';
    Log.info(_logTag, 'onDisconnected');
    print('onDisconnected');
    await connect();
  }

  @override
  Future<void> onRejected(String errorMessage) async {
    _message = 'onRejected';
    Log.info(_logTag, 'onRejected. error($errorMessage)');
    print('onRejected. error($errorMessage)');
    await connect();
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
      _message = 'Failed to get platform version.';
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
    Future<void>.delayed(
        const Duration(seconds: 1),
        () => setState(() {
              Log.info(_logTag, 'received message: $_message');
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
              child: const Text('Send message'),
            ),
          ]),
    );
  }
}
