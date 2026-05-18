// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:keyboard_detection_tizen/keyboard_detection_tizen.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'keyboard_detection_tizen example',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const HomePage(),
    );
  }
}

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  late final KeyboardDetectionController _controller;

  @override
  void initState() {
    super.initState();
    _controller = KeyboardDetectionController(
      onChanged: (KeyboardState state) {
        debugPrint('keyboard_detection_tizen: $state');
      },
    );
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Keyboard Detection')),
      body: SafeArea(
        child: Padding(
          padding: const EdgeInsets.all(24),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              StreamBuilder<KeyboardState>(
                stream: _controller.stream,
                initialData: _controller.state,
                builder: (BuildContext _, AsyncSnapshot<KeyboardState> snap) {
                  final KeyboardState s = snap.data ?? KeyboardState.unknown;
                  return Column(
                    key: const Key('status'),
                    children: <Widget>[
                      Text(
                        'state: ${s.name}',
                        key: const Key('state-text'),
                        style: Theme.of(context).textTheme.titleMedium,
                      ),
                      const SizedBox(height: 8),
                      Text(
                        'visible: ${_controller.stateAsBool() ?? "unknown"}',
                        key: const Key('visible-text'),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        'width: ${_controller.width.toStringAsFixed(1)} '
                        '/ size(height): ${_controller.size.toStringAsFixed(1)} ',
                        key: const Key('size-text'),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        'position: '
                        '(${_controller.position.dx.toStringAsFixed(1)}, '
                        '${_controller.position.dy.toStringAsFixed(1)})',
                        key: const Key('position-text'),
                      ),
                    ],
                  );
                },
              ),
              const SizedBox(height: 32),
              const TextField(
                decoration: InputDecoration(
                  labelText: 'Tap here to open keyboard',
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
