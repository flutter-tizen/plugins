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

/// The key-value pair to provide additional information for the launch request.
class ExtraData {
  /// [values] only supports type [List<String>] and [String].
  ExtraData({
    required this.key,
    required this.values,
  });

  /// The key of the [ExtraData].
  final String key;

  /// The values of the [ExtraData].
  ///
  /// [values] only supports type [List<String>] and [String].
  final dynamic values;

  /// Returns [ExtraData] memeber fields in a map format.
  Map<String, dynamic> toMap() => <String, dynamic>{
        'key': key,
        'values': _getValues(values),
      };

  List<String> _getValues(dynamic value) {
    assert(value is String || value is List<String>);
    List<String> values = <String>[];
    if (value is String) {
      values.add(value);
    } else {
      values = value as List<String>;
    }
    return values;
  }
}

class NotificationImage {
  /// [NotificationImage] specifies image options for notifications.
  NotificationImage({
    required this.type,
    this.path = '',
  });

  final ImageType type;
  final String path;

  /// Returns [NotificationImage] memeber fields in a map format.
  Map<String, String> toMap() => <String, String>{
        'type': type.toString().split('.').last,
        'path': path,
      };
}

class AppControlData {
  /// A set of information used by app control to launch applications that
  /// match specific operation, URI, MIME type, and extra data.
  ///
  /// [extras] only supports type [List<ExtraData>] and [ExtraData].
  AppControlData({
    required this.appId,
    this.extras,
    this.operation,
    this.uri,
    this.mime,
  });

  /// The ID of the application to be launched.
  final String appId;

  /// [operation] is used in implcit launches to determine which set of
  /// applications should be launched. See https://docs.tizen.org/application/native/guides/app-management/app-controls/#determining-the-application-for-an-implicit-launch-request
  /// for details.
  String? operation;

  /// [uri] is used in implcit launches to determine which set of
  /// applications should be launched. See https://docs.tizen.org/application/native/guides/app-management/app-controls/#determining-the-application-for-an-implicit-launch-request
  /// for details.
  String? uri;

  /// [mime] is used in implcit launches to determine which set of
  /// applications should be launched. See https://docs.tizen.org/application/native/guides/app-management/app-controls/#determining-the-application-for-an-implicit-launch-request
  /// for details.
  String? mime;

  /// A list of key-value pairs to provide additional information for the launch request.
  final dynamic extras;

  /// Returns [AppControlData] memeber fields in a map format.
  Map<String, dynamic> toMap() => <String, dynamic>{
        'appId': appId,
        'operation': operation,
        'uri': uri,
        'mime': mime,
        'extraData': _getExtras(extras)
            ?.map((ExtraData extra) => extra.toMap())
            .toList(),
      };

  List<ExtraData>? _getExtras(dynamic extras) {
    assert(extras == null || extras is List<ExtraData> || extras is ExtraData);
    List<ExtraData> extraDatas = <ExtraData>[];
    if (extras is List<ExtraData>) {
      extraDatas = extras;
    } else if (extras is ExtraData) {
      extraDatas.add(extras);
    } else {
      // extras == null
      return null;
    }
    return extraDatas;
  }
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
        'type': type.toString().split('.').last,
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
        'type': type.toString().split('.').last,
        'path': path,
      };
}
