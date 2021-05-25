import 'package:flutter/material.dart';
import 'dart:async';

import 'package:messageport_tizen/messageport_tizen.dart';

const String kPortName = 'servicePort';
const String kRemoteAppId = 'com.example.messageport_tizen_example';

Future<String> _showErrorDialog(BuildContext context, dynamic error) {
  return showDialog<String>(
      context: context,
      builder: (BuildContext context) => AlertDialog(
              title: const Text('Error occurred'),
              content: Text(error.toString()),
              actions: <Widget>[
                TextButton(
                  onPressed: () => Navigator.pop(context, ''),
                  child: const Text('OK'),
                )
              ]));
}

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  void initState() {
    super.initState();
  }

  int _responseCount = 0;
  void onMessage(Object message, [TizenRemotePort remotePort]) {
    _log('message received: ' + message.toString());
    if (remotePort != null) {
      _log("remotePort received: ${remotePort.portName}, "
          "${remotePort.remoteAppId}, trusted: ${remotePort.trusted}");
      _responseCount++;
      remotePort.send("Response: $_responseCount");
    }
  }

  Widget _textButton(String text, Function func, bool enabled) {
    return Container(
      child: ElevatedButton(
          onPressed: enabled
              ? () {
                  _log('$text button clicked');
                  func();
                }
              : null,
          style: ElevatedButton.styleFrom(
            primary: Colors.blue,
            padding: EdgeInsets.all(20),
            elevation: 3,
          ),
          child: Text(text)),
      margin: EdgeInsets.all(5),
    );
  }

  List _sendOptions = [
    ['bool', true],
    ['int', 134],
    ['double', 157.986],
    ['String', "Test message"],
    [
      "List",
      [1, 2, 3]
    ],
    [
      "Map",
      {"a": 1, "b": 2, "c": 3}
    ]
  ];
  bool _attachLocalPort = false;

  Widget _sendButtons(BuildContext context, bool enabled) {
    return Column(
      children: [
        Row(
          mainAxisAlignment: MainAxisAlignment.start,
          children: [
            Checkbox(
                value: _attachLocalPort,
                onChanged: (value) {
                  setState(() {
                    _attachLocalPort = value;
                  });
                }),
            Text("Attach local port"),
          ],
        ),
        GridView.builder(
            scrollDirection: Axis.vertical,
            shrinkWrap: true,
            gridDelegate: SliverGridDelegateWithMaxCrossAxisExtent(
                maxCrossAxisExtent: 100,
                childAspectRatio: 2,
                crossAxisSpacing: 10,
                mainAxisSpacing: 10),
            itemCount: _sendOptions.length,
            itemBuilder: (BuildContext ctx, index) {
              return Builder(
                builder: (context) =>
                    _textButton(_sendOptions[index][0], () async {
                  try {
                    if (_attachLocalPort) {
                      await _remotePort.sendWithLocalPort(
                          _sendOptions[index][1], _localPort);
                    } else {
                      await _remotePort.send(_sendOptions[index][1]);
                    }
                  } catch (e) {
                    _showErrorDialog(context, e);
                  }
                }, enabled),
              );
            }),
      ],
    );
  }

  List<String> _logList = [];

  void _log(String log) {
    setState(() {
      String date = '${DateTime.now().hour.toString().padLeft(2, '0')}:'
          '${DateTime.now().minute.toString().padLeft(2, '0')}:'
          '${DateTime.now().second.toString().padLeft(2, '0')}.'
          '${DateTime.now().millisecond.toString().padLeft(3, '0')}';
      _logList.add('$date: $log');
    });
  }

  Widget _logger(BuildContext context) {
    return Expanded(
      child: Container(
        child: GestureDetector(
          onLongPress: () {
            setState(() {
              _logList.clear();
            });
          },
          child: ListView.builder(
            itemCount: _logList.length,
            itemBuilder: (context, index) {
              return Text(
                _logList[index],
                style: TextStyle(fontSize: 8),
              );
            },
          ),
        ),
        decoration: BoxDecoration(
            border: Border.all(width: 1),
            borderRadius: BorderRadius.all(Radius.circular(3))),
        margin: EdgeInsets.all(5),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        home: Scaffold(
            appBar: AppBar(
              title: const Text('MessagePort Tizen Plugin'),
            ),
            body: Column(children: [
              Builder(
                builder: (context) =>
                    _textButton('Create local port', () async {
                  _localPort =
                      await TizenMessageport.createLocalPort(kPortName);
                  setState(() {});
                }, _localPort == null),
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: [
                  Builder(
                    builder: (context) => _textButton('Register', () async {
                      try {
                        _localPort.register(onMessage);
                        setState(() {});
                      } catch (e) {
                        _showErrorDialog(context, e.toString());
                      }
                    }, (_localPort != null && !_localPort.registered)),
                  ),
                  Builder(
                      builder: (context) => _textButton('Unregister', () async {
                            try {
                              _localPort.unregister();
                              setState(() {});
                            } catch (e) {
                              _showErrorDialog(context, e.toString());
                            }
                          }, (_localPort != null && _localPort.registered))),
                ],
              ),
              Builder(
                  builder: (context) => _textButton('Connect to remote',
                          () async {
                        try {
                          _remotePort =
                              await TizenMessageport.connectToRemotePort(
                                  kRemoteAppId, kPortName);
                          setState(() {});
                        } catch (e) {
                          _showErrorDialog(context, e.toString());
                        }
                      },
                          (_localPort != null &&
                              _localPort.registered &&
                              _remotePort == null))),
              _sendButtons(
                  context,
                  _remotePort != null &&
                      (_localPort != null && _localPort.registered)),
              _logger(context),
            ])));
  }

  @override
  void dispose() {
    super.dispose();
  }

  TizenLocalPort _localPort;
  TizenRemotePort _remotePort;
}
