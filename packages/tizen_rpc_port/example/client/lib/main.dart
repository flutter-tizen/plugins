// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import 'message_client.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

abstract class MessageReceiver {
  void updateMessage(String message);
}

class MyNotify extends NotifyCallback {
  MyNotify(this._receiver) : super();

  final MessageReceiver _receiver;

  @override
  Future<void> onReceived(String sender, String message) async {
    _receiver.updateMessage('$sender: $message');
  }
}

class MyMessageClient extends Message {
  MyMessageClient(super.appid, this._receiver);

  final MessageReceiver _receiver;

  @override
  Future<void> onConnected() async {
    _receiver.updateMessage('Connected');
    await register('ClientApp', MyNotify(_receiver));
  }

  @override
  Future<void> onDisconnected() async {
    _receiver.updateMessage('Discnnected');
    await connect();
  }

  @override
  Future<void> onRejected(String errorMessage) async {
    _receiver.updateMessage('Rejected error: $errorMessage');
  }
}

class _MyAppState extends State<MyApp> implements MessageReceiver {
  late final MyMessageClient _myProxy;
  String _input = '';
  String _message = '';

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> initPlatformState() async {
    try {
      _myProxy =
          MyMessageClient('com.example.tizen_rpc_port_server_example', this);
      await _myProxy.connect();
    } on PlatformException {
      _message = 'Connection has failed.';
    }
  }

  @override
  void updateMessage(String message) {
    setState(() {
      _message = message;
    });
  }

  Future<void> _sendMsg() async {
    await _myProxy.send(_input);
  }

  Future<void> _registerCallback() async {
    await _myProxy.register('ClientApp', MyNotify(this));
    setState(() {
      _message = 'Register callback has done.';
    });
  }

  Future<void> _unregisterCallback() async {
    await _myProxy.unregister();
    setState(() {
      _message = 'Unregister callback has done.';
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('RpcPortClient example app'),
        ),
        body: SingleChildScrollView(
          child: Column(
            children: <Widget>[
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
            ],
          ),
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
        ],
      ),
    );
  }
}
