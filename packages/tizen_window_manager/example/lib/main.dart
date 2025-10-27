// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:tizen_window_manager/tizen_window_manager.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  Map<String, dynamic>? _geometry;
  bool _isLoadingGeometry = false;

  Timer? _windowTimer;

  void _activateWindow() {
    final AppLifecycleState currentLifecycleState =
        WidgetsBinding.instance.lifecycleState!;
    if (currentLifecycleState != AppLifecycleState.resumed) {
      _windowTimer?.cancel();
      _windowTimer = Timer.periodic(const Duration(milliseconds: 500), (
        Timer timer,
      ) {
        final AppLifecycleState? currentState =
            WidgetsBinding.instance.lifecycleState;
        if (currentState == AppLifecycleState.resumed) {
          timer.cancel();
          _windowTimer = null;
        } else {
          debugPrint(
            'AppLifecycleState is still not resumed (current: $currentState), calling activate window again',
          );
          WindowManager.activate();
        }
      });
    } else {
      debugPrint(
          'AppLifecycleState is resumed, no need to start periodic calls');
    }
  }

  void _lowerWindow() {
    WindowManager.lower();
    debugPrint('Window lowered successfully');
  }

  void _lowerWindowAndActivateWindow() {
    WindowManager.lower();
    debugPrint('Window lowered successfully');
    Future<void>.delayed(const Duration(seconds: 3), () {
      debugPrint('3 seconds passed, showing window again');
      _activateWindow();
    });
  }

  Future<void> _getGeometry() async {
    setState(() {
      _isLoadingGeometry = true;
    });

    try {
      final Map<String, int> geometry = await WindowManager.getGeometry();
      debugPrint('Window geometry: $geometry');
      debugPrint('Position: (${geometry['x']}, ${geometry['y']})');
      debugPrint('Size: ${geometry['width']} x ${geometry['height']}');

      setState(() {
        _geometry = geometry;
        _isLoadingGeometry = false;
      });
    } catch (e) {
      debugPrint('Error getting geometry: $e');
      setState(() {
        _isLoadingGeometry = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Tizen Window Manager Example'),
          backgroundColor: Colors.blue,
        ),
        body: SingleChildScrollView(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: <Widget>[
              const SizedBox(height: 20),
              const Text(
                'Tizen Window Manager API Test',
                style: TextStyle(
                  fontSize: 24,
                  fontWeight: FontWeight.bold,
                ),
                textAlign: TextAlign.center,
              ),
              const SizedBox(height: 20),
              const Text(
                'Window Control',
                style: TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.w600,
                ),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _lowerWindow,
                child: const Text('Call lower window.'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _lowerWindowAndActivateWindow,
                child: const Text(
                    'Call lower window and call activate window 3 seconds later.'),
              ),
              const SizedBox(height: 10),
              const Text(
                'Window Information',
                style: TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.w600,
                ),
              ),
              const SizedBox(height: 10),
              if (_geometry != null) ...<Widget>[
                Container(
                  padding: const EdgeInsets.all(16.0),
                  decoration: BoxDecoration(
                    color: Colors.grey[100],
                    borderRadius: BorderRadius.circular(8.0),
                    border: Border.all(color: Colors.grey[300]!),
                  ),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: <Widget>[
                      const Text(
                        'Window Geometry:',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.w600,
                        ),
                      ),
                      const SizedBox(height: 8),
                      Text(
                        'Position: (${_geometry!['x']}, ${_geometry!['y']})',
                        style: const TextStyle(fontSize: 14),
                      ),
                      const SizedBox(height: 4),
                      Text(
                        'Size: ${_geometry!['width']} x ${_geometry!['height']}',
                        style: const TextStyle(fontSize: 14),
                      ),
                    ],
                  ),
                ),
              ],
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _getGeometry,
                child: _isLoadingGeometry
                    ? const SizedBox(
                        width: 16,
                        height: 16,
                        child: CircularProgressIndicator(
                          strokeWidth: 2,
                          valueColor:
                              AlwaysStoppedAnimation<Color>(Colors.white),
                        ),
                      )
                    : const Text('Get Window Geometry'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
