// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:tizen_notification/tizen_notification.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  MyApp({Key? key}) : super(key: key);

  final TizenNotificationPlugin _tizenNotificationPlugin =
      TizenNotificationPlugin();
  final int notificationId = 1;

  Future<void> _showNotification() async {
    final TizenNotificationDetails details = TizenNotificationDetails(
      image: NotificationImage(iconPath: 'test.png'),
      properties: Property.disableAutoDelete,
      vibration: NotificationVibration(type: VibrationType.builtIn),
      sound: NotificationSound(type: SoundType.builtIn),
    );
    await _tizenNotificationPlugin.show(
      notificationId,
      title: 'show Notification Title',
      body: 'show Notification Body',
      notificationDetails: details,
    );
  }

  Future<void> _cancelNotification() async {
    await _tizenNotificationPlugin.cancel(notificationId);
  }

  Future<void> _cancelAllNotifications() async {
    await _tizenNotificationPlugin.cancelAll();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              ElevatedButton(
                onPressed: _showNotification,
                child: const Text('Show notification'),
              ),
              ElevatedButton(
                onPressed: _cancelNotification,
                child: const Text('Cancel notification'),
              ),
              ElevatedButton(
                onPressed: _cancelAllNotifications,
                child: const Text('Cancel all notifications'),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
