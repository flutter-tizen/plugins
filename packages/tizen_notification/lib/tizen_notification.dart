// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';

import 'src/types.dart';

export 'src/types.dart';

/// Provides functionality for displaying notifications.
class TizenNotificationPlugin {
  final MethodChannel _channel = const MethodChannel('tizen/notification');

  /// Displays a notification with the given properties.
  ///
  /// [id] is the identifier of the notification which can later be used to
  /// remove the notification.
  Future<void> show(
    int id, {
    String title = '',
    String body = '',
    TizenNotificationDetails? notificationDetails,
  }) {
    final Map<String, Object?> details =
        notificationDetails?.toMap() ?? <String, Object?>{};
    details['id'] = id.toString();
    details['title'] = title;
    details['body'] = body;

    // Set disableAppLaunch automatically if appControl is unset.
    if (notificationDetails?.appControl == null) {
      final int properties = details['properties']! as int;
      details['properties'] =
          properties | NotificationProperty.disableAppLaunch;
    }

    return _channel.invokeMethod('show', details);
  }

  /// Removes a notification with the specified [id].
  Future<void> cancel(int id) => _channel.invokeMethod('cancel', id.toString());

  /// Removes all notifications.
  Future<void> cancelAll() => _channel.invokeMethod('cancelAll');
}
