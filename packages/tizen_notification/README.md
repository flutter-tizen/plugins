# tizen_notification

[![pub package](https://img.shields.io/pub/v/tizen_notification.svg)](https://pub.dev/packages/tizen_notification)

Tizen notification APIs. Used to show and delete notifications on a Tizen device.

## Usage

To use this plugin, add `tizen_notification` as a dependency in your pubspec.yaml file:

```yaml
dependencies:
  tizen_notification: ^1.0.0
```

Then you can import `tizen_notification` in your Dart code:


```dart
import 'package:tizen_notification/notification.dart';

TizenNotificationPlugin plugin = TizenNotificationPlugin();
await plugin.show(1, 'Title', 'Body');
```

## Required privileges

To use this plugin, you need to declare the following privileges in `tizen-manifest.xml`.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/notification</privilege>
  <!--To launch an application by app_control handler-->
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```
