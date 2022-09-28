// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:tizen_bundle/tizen_bundle.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  final Bundle _bundle = Bundle();
  int _count = 0;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Tizen Bundle example',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen Bundle example')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              Text('Current size: ${_bundle.length}'),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: () {
                  _bundle['stringKey_${_count++}'] = 'stringValue_$_count';
                  setState(() {});
                },
                child: const Text('Add String'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: () {
                  if (_bundle.isNotEmpty) {
                    _bundle.remove(_bundle.keys.last);
                    setState(() {});
                  }
                },
                child: const Text('Remove String'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: () {
                  _bundle.clear();
                  setState(() {});
                },
                child: const Text('Clear'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
