# tizen_app_manager

 [![pub package](https://img.shields.io/pub/v/tizen_app_manager.svg)](https://pub.dev/packages/tizen_app_manager)

Tizen application manager APIs. Used for getting app info and getting app running context on a Tizen device.

## Usage

To use this package, add `tizen_app_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_app_manager: ^0.2.0
```

### Retrieving current app info

To retrieve information of the current app, get app ID with `AppManager.currentAppId` and then get `AppInfo` with `AppManager.getAppInfo`.

```dart
import 'package:tizen_app_manager/tizen_app_manager.dart';

String appId = await AppManager.currentAppId;
AppInfo appInfo = await AppManager.getAppInfo(appId);
```

### Retrieving all apps info

To retrieve information of all installed apps, use `AppManager.getInstalledApps`.

```dart
List<AppInfo> apps = await AppManager.getInstalledApps();
for (AppInfo app in apps) {
  // Handle each app's info.
}
```

### Getting app running context

To get a specific app's running context, create an `AppRunningContext` instance with the target app ID.

```dart
String appId = await AppManager.currentAppId;
AppRunningContext appContext = AppRunningContext(appId: appId);
```

### Monitoring app events

You can listen for app state changes by subscribing to `AppManager.onAppLaunched` and `AppManager.onAppTerminated`.

```dart
final List<StreamSubscription<AppRunningContext>> _subscriptions =
      <StreamSubscription<AppRunningContext>>[];

@override
void initState() {
  super.initState();

  _subscriptions
      .add(AppManager.onAppLaunched.listen((AppRunningContext context) {
    ...
  }));
  _subscriptions
      .add(AppManager.onAppTerminated.listen((AppRunningContext context) {
    ...
  }));
}

@override
void dispose() {
  super.dispose();

  _subscriptions
      .forEach((StreamSubscription subscription) => subscription.cancel());
  _subscriptions.clear();
}
```

## Required privileges

The following privileges may be required to use this plugin. Add required privileges to `tizen-manifest.xml` of your application.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
  <privilege>http://tizen.org/privilege/appmanager.kill.bgapp</privilege>
  <privilege>http://tizen.org/privilege/appmanager.kill</privilege>
</privileges>
```

- The `http://tizen.org/privilege/appmanager.launch` privilege is required by `AppRunningContext.resume()`.
- The `http://tizen.org/privilege/appmanager.kill.bgapp` privilege is required by `AppRunningContext.terminate(background: true)`.
- The `http://tizen.org/privilege/appmanager.kill` privilege is required by `AppRunningContext.terminate(background: false)`. Note that this is a platform level privilege and cannot be granted to third-party applications.
