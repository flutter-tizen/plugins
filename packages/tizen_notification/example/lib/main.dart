// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:tizen_notification/notification.dart';

void main() {
  runApp(NotificationTest());
}

class NotificationTest extends StatelessWidget {
  final TizenNotificationPlugin _tizenNotificationPlugin =
      TizenNotificationPlugin();
  final int notificationId = 1;

  Future<void> _showNotification() async {
    final TizenNotificationDetails details = TizenNotificationDetails(
      images: NotificationImage(type: ImageType.icon, path: 'test.png'),
      properties: Property.disableAutoDelete,
      vibration: NotificationVibration(type: VibrationType.builtIn),
      sound: NotificationSound(type: SoundType.builtIn),
    );
    await _tizenNotificationPlugin.show(
      notificationId,
      'show Notification Title',
      'show Notification Body',
      details,
    );
  }

  Future<void> _cancelNotification() async {
    await _tizenNotificationPlugin.cancel(notificationId);
  }

  Future<void> _cancelAllNotification() async {
    await _tizenNotificationPlugin.cancelAll();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            ElevatedButton(
              child: const Text('show Notification'),
              onPressed: _showNotification,
            ),
            ElevatedButton(
                child: const Text('cancel Notification'),
                onPressed: _cancelNotification),
            ElevatedButton(
                child: const Text('cancelAll Notification'),
                onPressed: _cancelAllNotification),
          ],
        ),
      ),
    );
  }
}
