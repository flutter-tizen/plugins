# tizen_app_manager

 [![pub package](https://img.shields.io/pub/v/tizen_app_manager.svg)](https://pub.dev/packages/tizen_app_manager)

Tizen application manager API. Used for getting installed app info and getting running context info of specific app.

## Usage

To use this package, add `tizen_app_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_app_manager: ^0.1.0
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
final List<StreamSubscription<dynamic>> subscriptions = [];

@override
void initState() {
  super.initState();

  subscriptions.add(AppManager.onAppLaunched
      .listen((AppRunningContext event) {
      // Handle the launched app context object and dispose it.
      ...
      context.dispose();
  }));
  subscriptions.add(AppManager.onAppTerminated
      .listen((AppRunningContext event) {
      // Handle the terminated app context object and dispose it.
      ...
      context.dispose();
  }));
}

@override
void dispose() {
  super.dispose();

  subscriptions.forEach((StreamSubscription s) => s.cancel());
  subscriptions.clear();
}
```

## Required privileges

Privileges be required to perform `resume()` in `AppRunningContext` class. Add required privileges in `tizen-manifest.xml` of your application.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```
