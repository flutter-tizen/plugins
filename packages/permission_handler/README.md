# permisson_handler_tizen

The Tizen implementation of [`permisson_handler`](https://github.com/Baseflow/flutter-permission-handler).

## Usage

1. Add required privileges in your `tizen-manifest.xml` file. For example,

   ```xml
   <manifest>
     ...
     <privileges>
       <privilege>http://tizen.org/privilege/mediastorage</privilege>
     </privileges>
   </manifest>
   ```

   | Permissions | Tizen privileges |
   |-|-|
   | accessMediaLocation | `http://tizen.org/privilege/mediastorage` |
   | calendar | `http://tizen.org/privilege/calendar.read`<br>`http://tizen.org/privilege/calendar.write` |
   | camera | `http://tizen.org/privilege/camera` |
   | contact | `http://tizen.org/privilege/contact.read`<br>`http://tizen.org/privilege/contact.write` |
   | location<br>locationAlways<br>locationWhenInUse | `http://tizen.org/privilege/location`<br>`http://tizen.org/privilege/location.coarse` |
   | microphone<br>speech | `http://tizen.org/privilege/recorder` |
   | phone | `http://tizen.org/privilege/call` |
   | sensors | `http://tizen.org/privilege/healthinfo` |
   | sms | `http://tizen.org/privilege/message.read`<br>`http://tizen.org/privilege/message.write` |
   | storage | `http://tizen.org/privilege/externalstorage` |

   For more information on Tizen privileges, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

2. Add `permission_handler` and `permission_handler_tizen` as dependencies in your `pubspec.yaml` file.

   ```yaml
   dependencies:
     permission_handler: ^6.1.1
     permission_handler_tizen: ^1.0.0
   ```

   Then you can import `permission_handler` in your Dart code:

   ```dart
   import 'package:permission_handler/permission_handler.dart';
   ```

   For detailed usage of the plugin, see https://github.com/Baseflow/flutter-permission-handler#how-to-use.

## Limitations

- This plugin is unavailable on Tizen TV where permissions are already granted to apps by default.
- `openAppSettings()` will open the system settings instead of the app settings on Tizen. To use it, add the app manager privilege (`http://tizen.org/privilege/appmanager.launch`) in your `tizen-manifest.xml` file.
