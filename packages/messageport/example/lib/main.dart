import 'dart:async';
import 'package:flutter/material.dart';

import 'package:messageport_tizen/messageport_tizen.dart';

const String kPortName = 'servicePort';
const String kRemoteAppId = 'org.tizen.messageport_tizen_example';

Future<String?> _showErrorDialog(BuildContext context, dynamic error) {
  return showDialog<String>(
    context: context,
    builder: (BuildContext context) => AlertDialog(
      title: const Text('Error occurred'),
      content: Text(error.toString()),
      actions: <Widget>[
        TextButton(
          onPressed: () => Navigator.pop(context, ''),
          child: const Text('OK'),
        ),
      ],
    ),
  );
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
  void onMessage(dynamic message, [RemotePort? remotePort]) {
    _log('message received: ' + message.toString());
    if (remotePort != null) {
      _log('remotePort received: ${remotePort.portName}, '
          '${remotePort.remoteAppId}, trusted: ${remotePort.trusted}');
      _responseCount++;
      remotePort.send('Response: $_responseCount');
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
          padding: const EdgeInsets.all(20),
          elevation: 3,
        ),
        child: Text(text),
      ),
      margin: const EdgeInsets.all(5),
    );
  }

  Widget _localPortRegisteringButtons() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      children: <Widget>[
        Builder(
          builder: (BuildContext context) => _textButton(
            'Register',
            () async {
              try {
                _localPort?.register(onMessage);
                setState(() {});
              } catch (e) {
                _showErrorDialog(context, e.toString());
              }
            },
            _localPort != null && !_localPort!.registered,
          ),
        ),
        Builder(
          builder: (BuildContext context) => _textButton(
            'Unregister',
            () async {
              try {
                _localPort?.unregister();
                setState(() {});
              } catch (e) {
                _showErrorDialog(context, e.toString());
              }
            },
            _localPort?.registered ?? false,
          ),
        ),
      ],
    );
  }

  Widget _remotePortButtons() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: <Widget>[
        Builder(
          builder: (BuildContext context) => _textButton(
            'Connect to remote',
            () async {
              try {
                _remotePort = await TizenMessagePort.connectToRemotePort(
                    kRemoteAppId, kPortName);
                setState(() {});
              } catch (e) {
                _showErrorDialog(context, e.toString());
              }
            },
            (_localPort?.registered ?? false) && _remotePort == null,
          ),
        ),
        Builder(
          builder: (BuildContext context) => _textButton(
            'Check remote',
            () async {
              try {
                final bool status = await _remotePort!.check();
                _log(
                    'Status of remote port: ' + (status ? 'opened' : 'closed'));
                setState(() {});
              } catch (e) {
                _showErrorDialog(context, e.toString());
              }
            },
            _remotePort != null,
          ),
        ),
      ],
    );
  }

  final Map<String, dynamic> _sendOptions = <String, dynamic>{
    'bool': true,
    'int': 134,
    'double': 157.986,
    'String': 'Test message',
    'List': <int>[1, 2, 3],
    'Map': <String, int>{'a': 1, 'b': 2, 'c': 3}
  };
  bool _attachLocalPort = false;

  Widget _sendButtons(BuildContext context, bool enabled) {
    return Column(
      children: <Widget>[
        Row(
          mainAxisAlignment: MainAxisAlignment.start,
          children: <Widget>[
            Checkbox(
                value: _attachLocalPort,
                onChanged: (bool? value) {
                  setState(() {
                    _attachLocalPort = value ?? false;
                  });
                }),
            const Text('Attach local port'),
          ],
        ),
        GridView.builder(
          scrollDirection: Axis.vertical,
          shrinkWrap: true,
          gridDelegate: const SliverGridDelegateWithMaxCrossAxisExtent(
            maxCrossAxisExtent: 100,
            childAspectRatio: 2,
            crossAxisSpacing: 10,
            mainAxisSpacing: 10,
          ),
          itemCount: _sendOptions.length,
          itemBuilder: (BuildContext ctx, int index) {
            final String key = _sendOptions.keys.elementAt(index);
            return Builder(
              builder: (BuildContext context) => _textButton(
                key,
                () async {
                  try {
                    if (_attachLocalPort) {
                      await _remotePort?.sendWithLocalPort(
                        _sendOptions[key],
                        _localPort!,
                      );
                    } else {
                      await _remotePort?.send(_sendOptions[key]);
                    }
                  } catch (e) {
                    _showErrorDialog(context, e);
                  }
                },
                enabled,
              ),
            );
          },
        ),
      ],
    );
  }

  final List<String> _logList = <String>[];

  void _log(String log) {
    setState(() {
      final String date = '${DateTime.now().hour.toString().padLeft(2, '0')}:'
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
            itemBuilder: (BuildContext context, int index) {
              return Text(
                _logList[index],
                style: const TextStyle(fontSize: 8),
              );
            },
          ),
        ),
        decoration: BoxDecoration(
          border: Border.all(width: 1),
          borderRadius: const BorderRadius.all(Radius.circular(3)),
        ),
        margin: const EdgeInsets.all(5),
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
        body: Column(children: <Widget>[
          Builder(
            builder: (BuildContext context) => _textButton(
              'Create local port',
              () async {
                _localPort = await TizenMessagePort.createLocalPort(kPortName);
                setState(() {});
              },
              _localPort == null,
            ),
          ),
          _localPortRegisteringButtons(),
          _remotePortButtons(),
          _sendButtons(
            context,
            _remotePort != null && (_localPort?.registered ?? false),
          ),
          _logger(context),
        ]),
      ),
    );
  }

  @override
  void dispose() {
    super.dispose();
  }

  LocalPort? _localPort;
  RemotePort? _remotePort;
}
