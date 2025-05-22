// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:wearable_rotary/wearable_rotary.dart';

void main() {
  runApp(
    MaterialApp(
      title: 'Rotary example app',
      home: const MyApp(),
      // Set the target platform to iOS so that navigation behaves as expected for a wearable app
      theme: ThemeData(platform: TargetPlatform.iOS),
    ),
  );
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('Rotary example app')),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            ElevatedButton(
              child: const Text('HorizontalScrollView'),
              onPressed: () {
                Navigator.push<dynamic>(
                  context,
                  MaterialPageRoute<dynamic>(
                    builder: (BuildContext context) => const RotaryScrollPage(
                      scrollDirection: Axis.horizontal,
                    ),
                  ),
                );
              },
            ),
            const SizedBox(height: 10),
            ElevatedButton(
              child: const Text('VerticalScrollView'),
              onPressed: () {
                Navigator.push<dynamic>(
                  context,
                  MaterialPageRoute<dynamic>(
                    builder: (BuildContext context) => const RotaryScrollPage(
                      scrollDirection: Axis.vertical,
                    ),
                  ),
                );
              },
            ),
          ],
        ),
      ),
    );
  }
}

class RotaryScrollPage extends StatelessWidget {
  const RotaryScrollPage({super.key, required this.scrollDirection});

  final Axis scrollDirection;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(
          scrollDirection == Axis.vertical
              ? 'VerticalScrollView'
              : 'HorizontalScrollView',
        ),
      ),
      body: Center(
        child: ListView.builder(
          padding: const EdgeInsets.all(16),
          controller: RotaryScrollController(),
          scrollDirection: scrollDirection,
          itemCount: 1000,
          itemBuilder: (BuildContext context, int index) => Text('Item $index'),
        ),
      ),
    );
  }
}
