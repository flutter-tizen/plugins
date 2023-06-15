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
  MyApp({super.key});

  final TizenNotificationPlugin _tizenNotificationPlugin =
      TizenNotificationPlugin();
  final int _notificationId = 1;

  Future<void> _showNotification() async {
    final TizenNotificationDetails details = TizenNotificationDetails(
      icons: NotificationIcons(icon: 'test.png'),
      sound: NotificationSound(type: SoundType.builtIn),
      vibration: NotificationVibration(type: VibrationType.builtIn),
      properties: NotificationProperty.disableAutoDelete,
      appControl: AppControl(appId: 'org.tizen.tizen_notification_example'),
    );
    await _tizenNotificationPlugin.show(
      _notificationId,
      title: 'Notification title',
      body: 'Notification body',
      notificationDetails: details,
    );
  }

  Future<void> _cancelNotification() async {
    await _tizenNotificationPlugin.cancel(_notificationId);
  }

  Future<void> _cancelAllNotifications() async {
    await _tizenNotificationPlugin.cancelAll();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Tizen Notification Example')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              ElevatedButton(
                onPressed: _showNotification,
                child: const Text('Show notification'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _cancelNotification,
                child: const Text('Cancel notification'),
              ),
              const SizedBox(height: 10),
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
