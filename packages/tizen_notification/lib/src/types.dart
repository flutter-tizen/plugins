// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:tizen_app_control/tizen_app_control.dart';

export 'package:tizen_app_control/tizen_app_control.dart';

/// How the system handles the notification in certain scenarios.
class NotificationProperty {
  // The constants below are originally defined in notification_type.h as
  // the enum type _notification_property.
  NotificationProperty._();

  /// Display only if a SIM card is inserted.
  static const int onlySimMode = 0x00000001;

  /// Do not perform any operation when the notification is clicked.
  static const int disableAppLaunch = 0x00000002;

  /// Do not dismiss the notification when clicked.
  static const int disableAutoDelete = 0x00000004;

  /// Dismiss the notification when the device is rebooted.
  static const int volatile = 0x00000100;
}

/// Where and how the notification should be presented.
class NotificationStyle {
  // The constants below are originally defined in notification_type.h as
  // the enum type _notification_display_applist.
  NotificationStyle._();

  /// Display in the notification area of the quick panel.
  static const int tray = 0x00000001;

  /// Display in the lock screen.
  static const int lock = 0x00000004;

  /// Display in the indicator area (the top of the screen).
  static const int indicator = 0x00000002 | 0x00000008;

  /// All of the above.
  static const int all = tray | lock | indicator;
}

/// A set of icons to be shown in the notification layouts.
class NotificationIcons {
  /// Creates a [NotificationIcons] with the given icon paths.
  NotificationIcons({
    this.icon,
    this.indicatorIcon,
    this.lockIcon,
  });

  /// The path to the icon file.
  final String? icon;

  /// The path to the indicator icon file.
  final String? indicatorIcon;

  /// The path to the lock screen icon file.
  final String? lockIcon;

  /// Converts to a map.
  Map<String, Object?> toMap() => <String, Object?>{
        'icon': icon,
        'iconForIndicator': indicatorIcon,
        'iconForLock': lockIcon,
      };
}

/// The type of sound.
enum SoundType {
  /// No sound.
  none,

  /// Default sound.
  builtIn,

  /// User sound data.
  userData,
}

/// The sound to play when the notification is presented.
class NotificationSound {
  /// Creates a [NotificationSound] with the given [type] and [path].
  NotificationSound({required this.type, this.path});

  /// The type of sound.
  final SoundType type;

  /// The path to the user sound data file.
  ///
  /// Only applicable if the [type] is [SoundType.userData].
  String? path;

  /// Converts to a map.
  Map<String, Object?> toMap() => <String, Object?>{
        'type': type.name,
        'path': path,
      };
}

/// The type of vibration.
enum VibrationType {
  /// No vibration.
  none,

  /// Default vibration pattern.
  builtIn,

  /// User vibration data.
  userData,
}

/// The notification vibration options.
class NotificationVibration {
  /// Creates a [NotificationVibration] with the given [type] and [path].
  NotificationVibration({required this.type, this.path});

  /// The type of vibration.
  final VibrationType type;

  /// The path to the user vibration data file.
  ///
  /// Only applicable if the [type] is [VibrationType.userData].
  String? path;

  /// Converts to a map.
  Map<String, Object?> toMap() => <String, Object?>{
        'type': type.name,
        'path': path,
      };
}

/// The notification details.
class TizenNotificationDetails {
  /// Constructs a [TizenNotificationDetails] from the given properties.
  const TizenNotificationDetails({
    this.icons,
    this.sound,
    this.vibration,
    this.properties = 0,
    this.style = NotificationStyle.all,
    this.ongoing = false,
    this.appControl,
  });

  /// A set of icons to be shown in the notification layouts.
  final NotificationIcons? icons;

  /// The sound to play when the notification is presented.
  final NotificationSound? sound;

  /// The notification vibration options.
  final NotificationVibration? vibration;

  /// Specifies how the system handles the notification in certain scenarios.
  ///
  /// Multiple [NotificationProperty] values can be set using the bitwise OR
  /// operator (|).
  final int properties;

  /// Specifies where and how the notification should be presented.
  ///
  /// Multiple [NotificationStyle] values can be set using the bitwise OR
  /// operator (|).
  final int style;

  /// Whether the notification can be dismissed by user.
  ///
  /// Only supported on Raspberry Pi (common profile) devices.
  final bool ongoing;

  /// A control message to be sent when the notification is clicked.
  final AppControl? appControl;

  /// Converts to a map.
  Map<String, Object?> toMap() => <String, Object?>{
        'image': icons?.toMap(),
        'sound': sound?.toMap(),
        'vibration': vibration?.toMap(),
        'properties': appControl != null
            ? properties
            : properties | NotificationProperty.disableAppLaunch,
        'displayApplist': style,
        'ongoing': ongoing,
        if (appControl != null)
          'appControl': <String, Object?>{
            'appId': appControl!.appId,
            'operation': appControl!.operation,
            'uri': appControl!.uri,
            'mime': appControl!.mime,
            'extraData': appControl!.extraData,
          },
      };
}
