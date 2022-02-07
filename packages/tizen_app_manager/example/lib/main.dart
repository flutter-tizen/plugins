// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';

import './apps_event.dart';
import './apps_list.dart';
import './current_app_info.dart';

/// The main entry point for the UI app.
void main() {
  runApp(const MyApp());
}

/// The main UI app widget.
class MyApp extends StatelessWidget {
  /// The constructor of the main UI app widget.
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Application manager demo',
      theme: ThemeData(primarySwatch: Colors.blue),
      home: const _MyHomePage(title: 'Application manager demo'),
    );
  }
}

class _MyHomePage extends StatefulWidget {
  const _MyHomePage({Key? key, required this.title}) : super(key: key);

  /// The title label
  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<_MyHomePage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Application manager demo')),
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: <Widget>[
            TextButton(
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute<Object>(
                        builder: (BuildContext context) =>
                            const CurrentAppScreen()),
                  );
                },
                child: const Text('Current application info')),
            TextButton(
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute<Object>(
                        builder: (BuildContext context) =>
                            const AppsListScreen()),
                  );
                },
                child: const Text('Installed applications list')),
            TextButton(
                onPressed: () {
                  Navigator.push(
                    context,
                    MaterialPageRoute<Object>(
                        builder: (BuildContext context) =>
                            const AppsEventScreen()),
                  );
                },
                child: const Text('Application launch/terminate and listener')),
          ],
        ),
      ),
    );
  }
}
