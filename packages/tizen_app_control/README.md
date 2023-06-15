# tizen_app_control

[![pub package](https://img.shields.io/pub/v/tizen_app_control.svg)](https://pub.dev/packages/tizen_app_control)

Tizen application control APIs. Used to launch apps on a Tizen device.

## Usage

To use this package, add `tizen_app_control` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_app_control: ^0.2.3
```

### Sending a launch request

To send an explicit launch request, create an `AppControl` instance with an application ID as an argument.

```dart
import 'package:tizen_app_control/tizen_app_control.dart';

var request = AppControl(appId: 'com.example.app_id');
await request.sendLaunchRequest();
```

To send an implicit launch request, create an `AppControl` instance and specify necessary conditions, such as operation, URI, and MIME type. For example, if you want to share a text message via SMS on a watch device, set `operation` to `http://tizen.org/appcontrol/operation/share_text` as follows.

```dart
import 'package:tizen_app_control/tizen_app_control.dart';

await AppControl(
  operation: 'http://tizen.org/appcontrol/operation/share_text',
  uri: 'sms:',
  launchMode: LaunchMode.group,
  extraData: {
    'http://tizen.org/appcontrol/data/text': 'Some text',
  },
).sendLaunchRequest();
```

For detailed information on Tizen application controls, see [Tizen Docs: Application Controls](https://docs.tizen.org/application/native/guides/app-management/app-controls). For a list of common operation types and examples, see [Tizen Docs: Common Application Controls](https://docs.tizen.org/application/native/guides/app-management/common-appcontrols). Operation and data constants, such as `http://tizen.org/appcontrol/operation/view`, are defined in [the native API references](https://docs.tizen.org/application/native/api/wearable/latest/group__CAPI__APP__CONTROL__MODULE.html).

### Receiving a launch request

You can subscribe to incoming application controls using `AppControl.onAppControl`.

```dart
import 'package:tizen_app_control/tizen_app_control.dart';

var subscription = AppControl.onAppControl.listen((request) async {
  if (request.shouldReply) {
    var reply = AppControl();
    await request.reply(reply, AppControlReplyResult.succeeded);
  }
});
...
await subscription.cancel();
```

## Required privileges

Privileges may be required to perform operations requested by your app. Add required privileges in `tizen-manifest.xml` of your application.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
  <!-- The below are optional. -->
  <privilege>http://tizen.org/privilege/call</privilege>
  <privilege>http://tizen.org/privilege/download</privilege>
</privileges>
```
