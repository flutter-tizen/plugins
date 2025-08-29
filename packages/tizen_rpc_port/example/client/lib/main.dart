// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';

import 'message_client.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _message = 'Not connected';
  String _message2 = 'Not connected2';
  bool _isConnected = false;
  bool _isConnected2 = false;
  String _input = '';
  Message? _client;
  Message? _client2;

  Future<void> _connect() async {
    setState(() {
      _message = 'Connecting...';
    });

    try {
      _client ??= Message('org.tizen.tizen_rpc_port_server_example');
      await _client!.connect(
        onError: (Object error) {
          setState(() {
            _message = 'Error: $error';
          });
        },
        onDisconnected: () async {
          setState(() {
            _isConnected = false;
            _message = 'Disconnected';
          });
        },
      );
      _client!.register('ClientApp', onMessage);

      setState(() {
        _isConnected = true;
        _message = 'Connected';
      });
    } catch (error) {
      setState(() {
        _message = 'Error: $error';
      });
    }
  }

  Future<void> _connect2() async {
    setState(() {
      _message2 = 'Connecting...';
    });

    try {
      _client2 ??= Message('org.tizen.tizen_rpc_port_server_example');
      await _client2!.connect(
        onError: (Object error) {
          setState(() {
            _message2 = 'Error: $error';
          });
        },
        onDisconnected: () async {
          setState(() {
            _isConnected2 = false;
            _message2 = 'Disconnected';
          });
        },
      );
      _client2!.register('ClientApp2', onMessage2);

      setState(() {
        _isConnected2 = true;
        _message2 = 'Connected';
      });
    } catch (error) {
      setState(() {
        _message2 = 'Error: $error';
      });
    }
  }

  void onMessage(String sender, String message) {
    setState(() {
      _message = 'Received: $message';
    });
  }

  void onMessage2(String sender, String message) {
    setState(() {
      _message2 = 'Received: $message';
    });
  }

  Future<void> _send() async {
    if (_isConnected) {
      _client!.send(_input);
    }
  }

  Future<void> _send2() async {
    if (_isConnected2) {
      _client2!.send(_input);
    }
  }

  @override
  void dispose() {
    if (_isConnected) {
      if (_client != null) {
        _client!.unregister();
        _client!.disconnect();
      }
    }
    if (_isConnected2) {
      if (_client2 != null) {
        _client2!.unregister();
        _client2!.disconnect();
      }
    }
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('TizenRpcPort Client Demo')),
        body: Padding(
          padding: const EdgeInsets.all(10),
          child: Column(
            children: <Widget>[
              Padding(
                padding: const EdgeInsets.symmetric(vertical: 20),
                child: Text(_message),
              ),
              Padding(
                padding: const EdgeInsets.symmetric(vertical: 20),
                child: Text(_message2),
              ),
              TextField(
                onChanged: (String text) {
                  setState(() => _input = text);
                },
                decoration: const InputDecoration(
                  border: OutlineInputBorder(),
                  labelText: 'Input',
                ),
              ),
            ],
          ),
        ),
        persistentFooterButtons: <Widget>[
          Column(
            children: <Widget>[
              TextButton(
                style: ButtonStyle(
                  minimumSize:
                      MaterialStateProperty.all(const Size.fromHeight(100)),
                ),
                onPressed: _connect,
                child: const Text('Connect'),
              ),
              TextButton(
                style: ButtonStyle(
                  minimumSize:
                      MaterialStateProperty.all(const Size.fromHeight(100)),
                ),
                onPressed: _connect2,
                child: const Text('Connect2'),
              ),
              TextButton(
                style: ButtonStyle(
                  minimumSize:
                      MaterialStateProperty.all(const Size.fromHeight(100)),
                ),
                onPressed: _isConnected ? _send : null,
                child: const Text('Send'),
              ),
              TextButton(
                style: ButtonStyle(
                  minimumSize:
                      MaterialStateProperty.all(const Size.fromHeight(100)),
                ),
                onPressed: _isConnected2 ? _send2 : null,
                child: const Text('Send2'),
              ),
            ],
          ),
        ],
      ),
    );
  }
}
