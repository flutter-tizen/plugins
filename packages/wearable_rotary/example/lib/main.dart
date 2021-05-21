import 'dart:async';

import 'package:flutter/material.dart';
import 'package:wearable_rotary/wearable_rotary.dart';

void main() => runApp(MyApp());

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _latestRotaryEvent = 'Initial value';
  StreamSubscription<RotaryEvent>? _rotarySubscription;

  @override
  void initState() {
    super.initState();
    _rotarySubscription = rotaryEvent.listen((RotaryEvent event) {
      setState(() {
        _latestRotaryEvent =
            event == RotaryEvent.CLOCKWISE ? 'CLOCKWISE' : 'COUNTER_CLOCKWISE';
      });
    });
  }

  @override
  void dispose() {
    super.dispose();
    _rotarySubscription?.cancel();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Rotary example app'),
        ),
        body: Center(
          child: Text(_latestRotaryEvent),
        ),
      ),
    );
  }
}
