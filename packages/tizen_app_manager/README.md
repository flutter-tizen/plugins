# tizen_app_manager

 [![pub package](https://img.shields.io/pub/v/tizen_app_manager.svg)](https://pub.dev/packages/tizen_app_manager)

tizen app manager API. Used for getting installed app info and getting running context info of specific app.

## Usage

To use this package, add `tizen_app_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_app_manager: ^0.1.0
```

### Getting current app info

To get the current app Info, get `currentAppId` and then get `AppInfo` with `getAppInfo` method.

```dart
    var appId = await TizenAppManager.currentAppId;
    var appInfo = await TizenAppManager.getAppInfo(appId);
```

### Getting installed apps info

To retrive all apps information that are installed in the Tizen device, use `getInstalledApps` method.

```dart
      var apps = await TizenAppManager.getInstalledApps();
      for (var app in apps) {
         // handle each App's info.
      }
```

### Getting app running context

To get specific app running context, create `AppRunningContext` instance.

```dart
    var appId = await TizenAppManager.currentAppId;
    var appContext= AppRunningContext(appId: appId)
```

### Listening for app state changes

You can listen for app state change by subscribing to the stream.

```dart
  final List<StreamSubscription<dynamic>> subscriptions = [];

  @override
  void initState() {
    super.initState();

    subscriptions.add(TizenAppManager.onAppLaunched
        .listen((AppRunningContext event) {
        //Got a launched app context info
    }));
    subscriptions.add(TizenAppManager.onAppTerminated
        .listen((AppRunningContext event) {
        //Got a terminated app context info
    }));
  }

  @override
  void dispose() {
    super.dispose();

    subscriptions.forEach((StreamSubscription s) => s.cancel());
    subscriptions.clear();
  }
```
