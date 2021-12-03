# permisson_handler_tizen

[![pub package](https://img.shields.io/pub/v/permission_handler_tizen.svg)](https://pub.dev/packages/permission_handler_tizen)

The Tizen implementation of [`permisson_handler`](https://github.com/Baseflow/flutter-permission-handler).

You can use this plugin to ask the user for runtime permissions if your app performs security-sensitive operations or access restricted data.

## Usage

1. Declare privileges in your `tizen-manifest.xml` file. For example, if you want to access the device's media library in your Flutter app:

   ```xml
   <manifest>
     ...
     <privileges>
       <privilege>http://tizen.org/privilege/mediastorage</privilege>
     </privileges>
   </manifest>
   ```

   | Permission | Tizen permission | Privileges |
   |-|-|-|
   | Permission.accessMediaLocation | _Android-only_ |
   | Permission.activityRecognition | _Android-only_ |
   | Permission.bluetooth | _iOS-only_ |
   | Permission.calendar | Calendar | `http://tizen.org/privilege/calendar.read`<br>`http://tizen.org/privilege/calendar.write` |
   | Permission.camera | Camera | `http://tizen.org/privilege/camera` |
   | Permission.contact | Contacts | `http://tizen.org/privilege/contact.read`<br>`http://tizen.org/privilege/contact.write` |
   | Permission.location<br>Permission.locationAlways<br>Permission.locationWhenInUse | Location | `http://tizen.org/privilege/location`<br>`http://tizen.org/privilege/location.coarse` |
   | Permission.mediaLibrary | Storage | `http://tizen.org/privilege/mediastorage` |
   | Permission.microphone | Microphone | `http://tizen.org/privilege/recorder` |
   | Permission.phone | Call | `http://tizen.org/privilege/call` |
   | Permission.photos<br>Permission.photosAddOnly | _iOS-only_ |
   | Permission.reminders | _iOS-only_ |
   | Permission.sensors | Sensor | `http://tizen.org/privilege/healthinfo` |
   | Permission.sms | Message | `http://tizen.org/privilege/message.read`<br>`http://tizen.org/privilege/message.write` |
   | Permission.speech | _iOS-only_ |
   | Permission.storage | Storage | `http://tizen.org/privilege/externalstorage` |

   For more information on Tizen privileges, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

2. Add `permission_handler` and `permission_handler_tizen` as dependencies in your `pubspec.yaml` file.

   ```yaml
   dependencies:
     permission_handler: ^6.1.1
     permission_handler_tizen: ^1.1.1
   ```

   Then you can import `permission_handler` in your Dart code:

   ```dart
   import 'package:permission_handler/permission_handler.dart';
   ```

   For detailed usage of the plugin, see https://github.com/Baseflow/flutter-permission-handler#how-to-use.

## Notes

This plugin is intended for **Galaxy Watch** devices only. On **TV**s, you don't need to request permissions since they are already granted to apps by default.