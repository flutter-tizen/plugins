# tizen_application_manager

 [![pub package](https://img.shields.io/pub/v/tizen_application_manager.svg)](https://pub.dev/packages/tizen_application_manager)

tizen application manager API. Used for getting installed application info and getting running context info of specific application.

## Usage

To use this package, add `tizen_application_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_application_manager: ^0.1.0
```

### Getting current application's info

To get the current application Info, get `currentAppId` and then get `ApplicationInfo` with `getApplicationInfo` method.

```dart
    var appId = await ApplicationManager.currentAppId;
    var appInfo = await ApplicationManager.getApplicationInfo(appId);
```

### Getting installed applications info

To retrive all applications information that are installed in the Tizen device, use `getInstalledApplications` method.

```dart
      var apps = await ApplicationManager.getInstalledApplications();
      for (var app in apps) {
         // handle each application's info.
      }
```

### Getting application running context

To get specific application running context, create `ApplicationRunningContext` instance.

```dart
    var appId = await ApplicationManager.currentAppId;
    var appContext= ApplicationRunningContext(applicationId: appId)
```

### Listening for application state changes

You can listen for application state change by subscribing to the stream.

```dart
  final List<StreamSubscription<dynamic>> subscriptions = [];

  @override
  void initState() {
    super.initState();

    subscriptions.add(TizenApplicationManager.onApplicationLaunched
        .listen((ApplicationRunningContext event) {
        //Got a launched application context info
    }));
    subscriptions.add(TizenApplicationManager.onApplicationTerminated
        .listen((ApplicationRunningContext event) {
        //Got a terminated application context info
    }));
  }

  @override
  void dispose() {
    super.dispose();

    subscriptions.forEach((StreamSubscription s) => s.cancel());
    subscriptions.clear();
  }
```
