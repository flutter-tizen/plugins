# tizen_app_manager

 [![pub package](https://img.shields.io/pub/v/tizen_app_manager.svg)](https://pub.dev/packages/tizen_app_manager)

Tizen application manager API. Used for getting installed app info and getting running context info of specific app.

## Usage

To use this package, add `tizen_app_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_app_manager: ^0.1.1
```

### Retrieving current app info

To retrieve information of the current app, get app ID with `currentAppId` and then get `AppInfo` with `getAppInfo` method.

```dart
var appId = await AppManager.currentAppId;
var appInfo = await AppManager.getAppInfo(appId);
```

### Retrieving all apps info

To retrieve information of all apps installed on a Tizen device, use `getInstalledApps` method.

```dart
var apps = await AppManager.getInstalledApps();
for (var app in apps) {
  // Handle each app's info.
}
```

### Getting app running context

To get specific app running context, create `AppRunningContext` instance.

```dart
var appId = await AppManager.currentAppId;
var appContext = AppRunningContext(appId: appId);
```

### Monitoring app events

You can listen for app state change by subscribing to the stream.

```dart
final List<StreamSubscription<AppRunningContext>> _subscriptions =
      <StreamSubscription<AppRunningContext>>[];

@override
void initState() {
  super.initState();

  _subscriptions.add(AppManager.onAppLaunched
      .listen((AppRunningContext event) {
      // Handle the launched event.
      ...
      event.dispose();
  }));
  _subscriptions.add(AppManager.onAppTerminated
      .listen((AppRunningContext event) {
      // Handle the terminated event.
      ...
      event.dispose();
  }));
}

@override
void dispose() {
  super.dispose();

  _subscriptions.forEach((StreamSubscription subscription) => subscription.cancel());
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
