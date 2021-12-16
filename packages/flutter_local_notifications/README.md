# flutter_local_notifications

[![pub package](https://img.shields.io/pub/v/flutter_local_notifications_tizen.svg)](https://pub.dev/packages/flutter_local_notifications_tizen)

The Tizen implementation of `flutter_local_notifications`.

## Usage

To use this plugin, add `flutter_local_notifications_tizen` as a dependency in your pubspec.yaml file:

```yaml
dependencies:
  flutter_local_notifications_tizen: ^1.0.0
```

Then you can import `flutter_local_notifications_tizen` in your Dart code:


```dart
import 'package:flutter_local_notifications_tizen/flutter_local_notifications_tizen.dart';

TizenFlutterLocalNotificationsPlugin plugin = TizenFlutterLocalNotificationsPlugin();
int notification_id = 1;
plugin.show(notification_id, 'Title', 'Body');
```

## Required privileges

To Use this plugin, you need to declare the following privileges in `tizen-manifest.xml`.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/notification</privilege>
  <!--To launch an application by app_control handler-->
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```
