#ifndef PERMISSION_HANDLER_TYPE_H_
#define PERMISSION_HANDLER_TYPE_H_

// Keep in sync with the values defined in:
// https://github.com/Baseflow/flutter-permission-handler/blob/master/permission_handler_platform_interface/lib/src/permissions.dart
enum class PermissionGroup {
  kCalendar = 0,
  kCamera = 1,
  kContacts = 2,
  kLocation = 3,
  kLocationAlways = 4,
  kLocationWhenInUse = 5,
  kMediaLibrary = 6,
  kMicrophone = 7,
  kPhone = 8,
  kPhotos = 9,
  kPhotosAddOnly = 10,
  kReminders = 11,
  kSensors = 12,
  kSMS = 13,
  kSpeech = 14,
  kStorage = 15,
  kIgnoreBatteryOptimizations = 16,
  kNotification = 17,
  kAccessMediaLocation = 18,
  kActivityRecognition = 19,
  kUnknown = 20,
  kBluetooth = 21,
  kManageExternalStorage = 22,
  kSystemAlertWindow = 23,
  kRequestInstallPackages = 24,
  kAppTrackingTransparency = 25,
  kCriticalAlerts = 26,
  kAccessNotificationPolicy = 27,
  kBluetoothScan = 28,
  kBluetoothAdvertise = 29,
  kBluetoothConnect = 30
};

// permission status
enum class PermissionStatus {
  kDenied = 0,
  kGranted,
  kRestricted,
  kLimited,
  kPermanentlyDenied
};

// service status
enum class ServiceStatus { kDisabled = 0, kEnabled, kNotApplicable };

#endif  // PERMISSION_HANDLER_TYPE_H_
