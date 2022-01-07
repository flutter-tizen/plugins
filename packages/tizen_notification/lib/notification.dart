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
    this.properties,
    this.appControl,
    this.vibration,
    this.sound,
    this.displayApplist = DisplayApplist.all,
    this.onGoing = false,
  });

  /// A single or a list of [NotificationImage].
  ///
  /// [NotificationImage] refers to any image shown on the notification panel.
  final dynamic images;

  /// Properties that configure how the system handles notification at
  /// certain scenarios.
  ///
  /// [properties] can have multiple [Property] values with the bitwise OR operator.
  /// For example, [Property.disableAppLaunch] | [Property.volatileDisplay].
  final int? properties;

  /// A set of information used by app control to launch other applications.
  final AppControl? appControl;

  /// The vibration triggered when a notification is shown on the panel.
  final NotificationVibration? vibration;

  /// The sound played when a notification is shown on the panel.
  final NotificationSound? sound;

  /// The destination app that displays notification.
  ///
  /// The notification can be sent to multiple apps by listing values with the
  /// bitwise OR operator.
  /// For example, [DisplayApplist.tray] | [DisplayApplist.indicator].
  final int? displayApplist;

  /// If [onGoing] is true, the user is not able to delete the notification.
  ///
  /// Currently, only the common profile supports this operation.
  final bool onGoing;

  /// Returns [TizenNotificationDetails] memeber fields in a map format.
  Map<String, dynamic> toMap() => <String, dynamic>{
        'displayApplist': displayApplist,
        'properties': properties,
        'vibration': vibration?.toMap(),
        'sound': sound?.toMap(),
        'images': _getImages(images)
            ?.map((NotificationImage image) => image.toMap())
            .toList(),
        'onGoing': onGoing,
        'appControl': <String, dynamic>{
          'appId': appControl?.appId,
          'operation': appControl?.operation,
          'uri': appControl?.uri,
          'mime': appControl?.mime,
          'extraData': appControl?.extraData,
        },
      };

  List<NotificationImage>? _getImages(dynamic image) {
    assert(image == null ||
        image is List<NotificationImage> ||
        image is NotificationImage);
    List<NotificationImage> images = <NotificationImage>[];
    if (image is List<NotificationImage>) {
      images = image;
    } else if (image is NotificationImage) {
      images.add(image);
    } else {
      // extras == null
      return null;
    }
    return images;
  }
}

const MethodChannel _channel = MethodChannel('tizen/internal/notification');

/// A handle for sending notifications to Tizen.
class TizenNotificationPlugin {
  ///Removes a notification with [id].
  Future<void> cancel(int id, {String? tag}) =>
      _channel.invokeMethod('cancel', id.toString());

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
