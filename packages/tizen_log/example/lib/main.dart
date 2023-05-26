// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:tizen_log/tizen_log.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  static const String _logTag = 'tizen_log_example';

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Log example',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: Scaffold(
        appBar: AppBar(title: const Text('Log example')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              const Text(
                'Log tag: $_logTag',
                style: TextStyle(fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 20),
              TextButton(
                onPressed: () => Log.verbose(_logTag, 'verbose message'),
                child: const Text('Log verbose message'),
              ),
              TextButton(
                onPressed: () => Log.debug(_logTag, 'debug message'),
                child: const Text('Log debug message'),
              ),
              TextButton(
                onPressed: () => Log.info(_logTag, 'info message'),
                child: const Text('Log info message'),
              ),
              TextButton(
                onPressed: () => Log.warn(_logTag, 'warn message'),
                child: const Text('Log warn message'),
              ),
              TextButton(
                onPressed: () => Log.error(_logTag, 'error message'),
                child: const Text('Log error message'),
              ),
              TextButton(
                onPressed: () => Log.fatal(_logTag, 'fatal message'),
                child: const Text('Log fatal message'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
