# permisson_handler_tizen

[![pub package](https://img.shields.io/pub/v/permission_handler_tizen.svg)](https://pub.dev/packages/permission_handler_tizen)

The Tizen implementation of [`permisson_handler`](https://pub.dev/packages/permission_handler).

You can use this plugin to ask the user for runtime permissions if your app performs security-sensitive operations or access restricted data.

## Usage

1. Declare necessary privileges in your `tizen-manifest.xml` file by referring to the below [permission list](#list-of-permissions). For example, if you want to access the device's media library in your Flutter app, add:

   ```xml
   <privileges>
     <privilege>http://tizen.org/privilege/mediastorage</privilege>
   </privileges>
   ```

2. Add `permission_handler` and `permission_handler_tizen` as dependencies in your `pubspec.yaml` file.

   ```yaml
   dependencies:
     permission_handler: ^10.4.3
     permission_handler_tizen: ^1.3.0
   ```

   Then you can import `permission_handler` in your Dart code:

   ```dart
   import 'package:permission_handler/permission_handler.dart';
   ```

   For detailed usage, see https://pub.dev/packages/permission_handler#how-to-use.

## List of permissions

| Permission | Tizen permission | Tizen privileges |
|-|-|-|
| `Permission.calendar` | Calendar | `http://tizen.org/privilege/calendar.read`<br>`http://tizen.org/privilege/calendar.write` |
| `Permission.camera` | Camera | `http://tizen.org/privilege/camera` |
| `Permission.contact` | Contacts | `http://tizen.org/privilege/contact.read`<br>`http://tizen.org/privilege/contact.write` |
| `Permission.location`<br>`Permission.locationAlways`<br>`Permission.locationWhenInUse` | Location | `http://tizen.org/privilege/location`<br>`http://tizen.org/privilege/location.coarse` |
| `Permission.mediaLibrary` | Storage | `http://tizen.org/privilege/mediastorage` |
| `Permission.microphone` | Microphone | `http://tizen.org/privilege/recorder` |
| `Permission.phone` | Call | `http://tizen.org/privilege/call` |
| `Permission.sensors` | Sensor | `http://tizen.org/privilege/healthinfo` |
| `Permission.sms` | Message | `http://tizen.org/privilege/message.read`<br>`http://tizen.org/privilege/message.write` |
| `Permission.storage` | Storage | `http://tizen.org/privilege/externalstorage` |

The following permissions are not applicable for Tizen:

- Android-only: `accessMediaLocation`, `accessNotificationPolicy`, `activityRecognition`, `bluetoothAdvertise`, `bluetoothConnect`, `bluetoothScan`, `manageExternalStorage`, `requestInstallPackages`, `systemAlertWindow`, `nearbyWifiDevices`, `videos`, `audio`, `kScheduleExactAlarm`, `sensorsAlways`
- iOS-only: `appTrackingTransparency`, `bluetooth`, `criticalAlerts`, `photos`, `photosAddOnly`, `reminders`, `speech`

On Tizen, your app can use some security-sensitive features (such as bluetooth) without explicitly acquiring permissions. However, you might need to declare relevant privileges in its `tizen-manifest.xml` file. For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).

## Supported devices

- Galaxy Watch series (running Tizen 5.5)

On TV devices, you don't need to explicitly request permissions since they are already granted to apps by default.

## Supported APIs

- [x] `Permission.status` (including shortcuts such as `Permission.isGranted` and `Permission.isPermanentlyDenied`)
- [x] `Permission.serviceStatus`
- [ ] `Permission.shouldShowRequestRationale` (Android-only)
- [x] `Permission.request`
- [x] `List<Permission>.request`
- [x] `openAppSettings` (not supported on emulators)
