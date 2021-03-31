# permisson_handler_tizen

The Tizen implementation of [`permisson_handler`](https://github.com/Baseflow/flutter-permission-handler).

## Supported devices

This plugin is unavailable on Tizen TV.

## Required privileges

The privileges are required for the permission when the plugin used. 
If you want to use the permission, add the specified privileges in your `tizen-manifest.xml` file.

calendar permission:
```xml
<privilege>http://tizen.org/privilege/calendar.read</privilege>
<privilege>http://tizen.org/privilege/calendar.write</privilege>
```

camera permission:
```xml
<privilege>http://tizen.org/privilege/camera</privilege>
```

contact permission:
```xml
<privilege>http://tizen.org/privilege/contact.read</privilege>
<privilege>http://tizen.org/privilege/contact.write</privilege>
```

location/locationAlways/locationWhenInUse permission:
```xml
<privilege>http://tizen.org/privilege/location</privilege>
<privilege>http://tizen.org/privilege/location.coarse</privilege>
```

microphone/speech permission:
```xml
<privilege>http://tizen.org/privilege/recorder</privilege>
```

phone permission:
```xml
<privilege>http://tizen.org/privilege/call</privilege>
```

sensors permission:
```xml
<privilege>http://tizen.org/privilege/healthinfo</privilege>
```

sms permission:
```xml
<privilege>http://tizen.org/privilege/message.read</privilege>
<privilege>http://tizen.org/privilege/message.write</privilege>
```

storage permission:
```xml
<privilege>http://tizen.org/privilege/externalstorage</privilege>
```

accessMediaLocation permission:
```xml
<privilege>http://tizen.org/privilege/mediastorage</privilege>
```

If you want to use openAppSettings(), add 'appmanager' privilege in your `tizen-manifest.xml` file.
```xml
<privilege>http://tizen.org/privilege/appmanager.launch</privilege>
```

For details, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

## Usage

To use this plugin in a Tizen application, you have to include `permission_handler_tizen` alongside `permission_handler` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  permission_handler: ^5.0.1+1
  permission_handler_tizen: ^1.0.0
```

Then you can import `permission_handler` in your Dart code:

```dart
import 'package:permission_handler/permission_handler.dart';
```

In Tizen platform, the check permission result is 'allow', 'deny' or 'undetermined', and request permission result is 'allow forever', 'deny forever' or 'deny once'. If user deny the permission and don't select 'Don't ask again?', user can request permission again and the result of check permission is 'undetermined'. It's different from Android.

For how to use the plugin, see https://github.com/Baseflow/flutter-permission-handler#how-to-use.
