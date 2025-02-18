// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';

import 'package:messageport_tizen/messageport_tizen.dart';

const String kPortName = 'servicePort';
const String kRemoteAppId = 'org.tizen.messageport_tizen_example';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  LocalPort? _localPort;
  RemotePort? _remotePort;
  int _responseCount = 0;

  void onMessage(dynamic message, [RemotePort? remotePort]) {
    _log('Message received: $message');

    if (remotePort != null) {
      _responseCount++;
      remotePort.send('Response: $_responseCount');
    }
  }

  Widget _textButton(String text, void Function() onPressed, bool enabled) {
    return Container(
      margin: const EdgeInsets.all(5),
      child: ElevatedButton(
        onPressed: enabled ? onPressed : null,
        child: Text(text),
      ),
    );
  }

  Widget _localPortRegisterButtons() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: <Widget>[
        _textButton('Register', () async {
          try {
            _localPort?.register(onMessage);
            _log('Local port registration done');
            setState(() {});
          } catch (error) {
            _log(error.toString());
          }
        }, _localPort != null && !_localPort!.registered),
        _textButton('Unregister', () async {
          try {
            await _localPort?.unregister();
            _log('Local port unregistration done');
            setState(() {});
          } catch (error) {
            _log(error.toString());
          }
        }, _localPort?.registered ?? false),
      ],
    );
  }

  Widget _remotePortButtons() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: <Widget>[
        _textButton(
          'Connect to remote',
          () async {
            try {
              _remotePort = await RemotePort.connect(kRemoteAppId, kPortName);
              _log('Connected to remote port');
              setState(() {});
            } catch (error) {
              _log(error.toString());
            }
          },
          (_localPort?.registered ?? false) && _remotePort == null,
        ),
        _textButton('Check remote', () async {
          try {
            final bool status = await _remotePort!.check();
            _log('Remote port status: ${status ? 'open' : 'closed'}');
            setState(() {});
          } catch (error) {
            _log(error.toString());
          }
        }, _remotePort != null),
      ],
    );
  }

  final Map<String, Object> _sendOptions = <String, Object>{
    'bool': true,
    'int': 134,
    'double': 157.986,
    'String': 'Test message',
    'List': <int>[1, 2, 3],
    'Map': <String, int>{'a': 1, 'b': 2, 'c': 3},
  };
  bool _attachLocalPort = false;

  Widget _sendButtons() {
    return Column(
      children: <Widget>[
        Row(
          children: <Widget>[
            Checkbox(
              value: _attachLocalPort,
              onChanged: (bool? value) {
                setState(() {
                  _attachLocalPort = value ?? false;
                });
              },
            ),
            const Text('Attach local port'),
          ],
        ),
        GridView.builder(
          shrinkWrap: true,
          gridDelegate: const SliverGridDelegateWithFixedCrossAxisCount(
            crossAxisCount: 3,
            mainAxisExtent: 40,
          ),
          itemCount: _sendOptions.length,
          itemBuilder: (BuildContext context, int index) {
            final String key = _sendOptions.keys.elementAt(index);
            return _textButton(key, () async {
              try {
                final Object value = _sendOptions[key]!;
                if (_attachLocalPort) {
                  await _remotePort?.sendWithLocalPort(value, _localPort!);
                } else {
                  await _remotePort?.send(value);
                }
              } catch (error) {
                _log(error.toString());
              }
            }, _remotePort != null && (_localPort?.registered ?? false));
          },
        ),
      ],
    );
  }

  final List<String> _logs = <String>[];

  void _log(String log) {
    final String date =
        '${DateTime.now().hour.toString().padLeft(2, '0')}:'
        '${DateTime.now().minute.toString().padLeft(2, '0')}:'
        '${DateTime.now().second.toString().padLeft(2, '0')}.'
        '${DateTime.now().millisecond.toString().padLeft(3, '0')}';
    setState(() {
      _logs.add('$date: $log');
    });
    debugPrint('$date: $log');
  }

  Widget _logger(BuildContext context) {
    return Expanded(
      child: Container(
        padding: const EdgeInsets.all(5),
        child: GestureDetector(
          onTap: () {
            setState(() {
              _logs.clear();
            });
          },
          child: ListView.builder(
            itemCount: _logs.length,
            itemBuilder: (BuildContext context, int index) {
              return Text(_logs[index], style: const TextStyle(fontSize: 10));
            },
          ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen MessagePort Example')),
        body: Column(
          children: <Widget>[
            _textButton('Create local port', () async {
              _localPort = await LocalPort.create(kPortName);
              setState(() {});
            }, _localPort == null),
            _localPortRegisterButtons(),
            _remotePortButtons(),
            _sendButtons(),
            _logger(context),
          ],
        ),
      ),
    );
  }

  @override
  void dispose() {
    super.dispose();
    _localPort?.unregister();
  }
}
