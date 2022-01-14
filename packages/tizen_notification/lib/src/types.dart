// ignore_for_file: public_member_api_docs

import 'enums.dart';

/// Properties that configure how the system handles notification at
/// certain scenarios.
class Property {
  /// Display only SIM card inserted.
  static const int displayOnlySimMode = 1 << 0;

  /// Disable application launch when notification is selected.
  static const int disableAppLaunch = 1 << 1;

  /// Disable auto delete when notification is selected.
  static const int disableAutoDelete = 1 << 2;

  /// Delete notification when device is rebooted.
  static const int volatileDisplay = 1 << 8;
}

/// The destination app that displays notification.
class DisplayApplist {
  /// Notification Tray(Quickpanel).
  static const int tray = 1 << 0;

  /// Ticker notification.
  static const int ticker = 1 << 1;

  /// Lock screen.
  static const int lock = 1 << 2;

  /// Indicator.
  static const int indicator = 1 << 3;

  /// All display application.
  static const int all = (1 << 4) - 1;
}

class NotificationImage {
  /// [NotificationImage] specifies image options for notifications.
  NotificationImage({
    this.iconPath,
    this.indicatorPath,
    this.lockPath,
  });

  /// The path of icon.
  final String? iconPath;

  /// The path of indicator icon.
  final String? indicatorPath;

  /// The path of lock screen icon.
  final String? lockPath;

  /// Returns [NotificationImage] memeber fields in a map format.
  Map<String, dynamic> toMap() => <String, dynamic>{
        'icon': iconPath,
        'iconForIndicator': indicatorPath,
        'iconForLock': lockPath,
      };
}

class NotificationSound {
  /// [NotificationSound] specifies sound options for notifications.
  ///
  /// [path] referes to a file path to custom sound data played on notification,
  /// this value is ignored When [type] is not [SoundType.userData].
  NotificationSound({
    required this.type,
    this.path,
  });

  final SoundType type;
  String? path;

  /// Returns [NotificationSound] memeber fields in a map format.
  Map<String, dynamic> toMap() => <String, dynamic>{
        'type': type.name,
        'path': path,
      };
}

class NotificationVibration {
  /// [NotificationVibration] specifies vibration options for notifications.
  NotificationVibration({
    required this.type,
    this.path,
  });

  final VibrationType type;
  String? path;

  /// Returns [NotificationVibration] memeber fields in a map format.
  Map<String, dynamic> toMap() => <String, dynamic>{
        'type': type.name,
        'path': path,
      };
}
