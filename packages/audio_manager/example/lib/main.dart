// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:audio_manager_tizen_example/volume_control.dart';
import 'package:flutter/material.dart';

void main() {
  runApp(MyApp());
}

/// The main UI app widget.
class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Audio Manager Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const MyHomePage(title: 'Audio Manager Demo'),
    );
  }
}

/// Home page.
class MyHomePage extends StatefulWidget {
  /// Home page.
  const MyHomePage({Key? key, required this.title}) : super(key: key);

  /// Home page title.
  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  @override
  Widget build(BuildContext context) {
    return VolumeControlScreen();
  }
}
