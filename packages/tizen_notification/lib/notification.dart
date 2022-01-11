import 'package:flutter/services.dart';
import 'package:tizen_app_control/app_control.dart';

import 'src/types.dart';

export 'package:tizen_app_control/app_control.dart';

export 'src/enums.dart';
export 'src/types.dart';

// ignore: public_member_api_docs
class TizenNotificationDetails {
  /// [TizenNotificationDetails] contains detail information about the notification
  /// being sent.
  ///
  /// [images] only supports type [List<NotificationImage>] and [NotificationImage].
  const TizenNotificationDetails({
    this.images,
    this.appControl,
    this.vibration,
    this.sound,
    this.properties = 0,
    this.displayApplist = DisplayApplist.all,
    this.ongoing = false,
  });

  /// [NotificationImage] refers to any image shown on the notification panel.
  final NotificationImage? images;

  /// A set of information used by app control to launch other applications.
  final AppControl? appControl;

  /// The vibration triggered when a notification is received.
  final NotificationVibration? vibration;

  /// The sound played when a notification is received.
  final NotificationSound? sound;

  /// Properties that configure how the system handles notification at
  /// certain scenarios.
  ///
  /// [properties] can have multiple [Property] values with the bitwise OR operator.
  /// For example, [Property.disableAppLaunch] | [Property.volatileDisplay].
  final int properties;

  /// The destination app that displays notification.
  ///
  /// The notification can be sent to multiple apps by listing values with the
  /// bitwise OR operator.
  /// For example, [DisplayApplist.tray] | [DisplayApplist.indicator].
  final int displayApplist;

  /// If [ongoing] is true, the user is not able to delete the notification.
  ///
  /// Currently, only the common profile supports this operation.
  final bool ongoing;

  /// Returns [TizenNotificationDetails] memeber fields in a map format.
  Map<String, dynamic> toMap() => <String, dynamic>{
        'displayApplist': displayApplist,
        'properties': properties,
        'vibration': vibration?.toMap(),
        'sound': sound?.toMap(),
        'images': images?.toMap(),
        'ongoing': ongoing,
        'appControl': <String, dynamic>{
          'appId': appControl?.appId,
          'operation': appControl?.operation,
          'uri': appControl?.uri,
          'mime': appControl?.mime,
          'extraData': appControl?.extraData,
        },
      };
}

const MethodChannel _channel = MethodChannel('tizen/notification');

/// A handle for sending notifications to Tizen.
class TizenNotificationPlugin {
  /// Removes a notification with [id].
  Future<void> cancel(int id) => _channel.invokeMethod('cancel', id.toString());

  /// Removes all notifications sent.
  Future<void> cancelAll() => _channel.invokeMethod('cancelAll');

  /// Sends the notification with [id], [title], and [body]. [id] is the identider
  /// of the notification which can later be passed to [cancel] to erase the
  /// notification.
  Future<void> show(
    int id, {
    String? title,
    String? body,
    TizenNotificationDetails? notificationDetails,
  }) {
    final Map<String, dynamic> details =
        notificationDetails?.toMap() ?? <String, dynamic>{};
    details['id'] = id.toString();
    details['title'] = title ?? '';
    details['body'] = body ?? '';
    return _channel.invokeMethod('show', details);
  }
}
