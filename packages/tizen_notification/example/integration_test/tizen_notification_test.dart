// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_notification/tizen_notification.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  late TizenNotificationPlugin plugin;

  setUp(() {
    plugin = TizenNotificationPlugin();
  });

  group('TizenNotificationPlugin', () {
    testWidgets('show notification does not throw',
        (WidgetTester tester) async {
      await plugin.show(1, title: 'Test Title', body: 'Test Body');
    });

    testWidgets('show notification with default title and body does not throw',
        (WidgetTester tester) async {
      await plugin.show(2);
    });

    testWidgets('cancel notification does not throw',
        (WidgetTester tester) async {
      await plugin.show(3, title: 'To Cancel');
      await plugin.cancel(3);
    });

    testWidgets('cancelAll does not throw', (WidgetTester tester) async {
      await plugin.show(4, title: 'Notification 1');
      await plugin.show(5, title: 'Notification 2');
      await plugin.cancelAll();
    });

    testWidgets(
        'show notification with TizenNotificationDetails does not throw',
        (WidgetTester tester) async {
      final TizenNotificationDetails details = TizenNotificationDetails(
        properties: NotificationProperty.disableAutoDelete,
        style: NotificationStyle.tray,
      );
      await plugin.show(
        6,
        title: 'Detailed',
        body: 'With details',
        notificationDetails: details,
      );
      await plugin.cancel(6);
    });
  });
}
