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
  Future<void> onReceived(String sender, String message);
}

class MyNotify extends NotifyCallback {
  MyNotify(this._receiver) : super();

  final MessageReceiver _receiver;

  @override
  Future<void> onReceived(String sender, String message) async {
    _receiver.onReceived(sender, message);
  }
}

class _MyAppState extends State<MyApp> implements MessageReceiver {
  late final Message _messageClient;
  String _input = '';
  String _message = '';

  @override
  void initState() {
    super.initState();
    initPlatformState();
  }

  Future<void> initPlatformState() async {
    try {
      _messageClient = Message('com.example.tizen_rpc_port_server_example');
      await _messageClient.connect(onDisconnected: () async {
        setState(() => _message = 'Disconnected');
      });
      setState(() => _message = 'Connected');
      _messageClient.register('ClientApp', MyNotify(this));
    } on PlatformException {
      _message = 'Connection has failed.';
    }
  }

  @override
  Future<void> onReceived(String sender, String message) async {
    setState(() {
      _message = '$sender: $message';
    });
  }

  Future<void> _sendMsg() async {
    await _messageClient.send(_input);
  }

  Future<void> _registerCallback() async {
    await _messageClient.register('ClientApp', MyNotify(this));
    setState(() {
      _message = 'Register callback has done.';
    });
  }

  Future<void> _unregisterCallback() async {
    await _messageClient.unregister();
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
