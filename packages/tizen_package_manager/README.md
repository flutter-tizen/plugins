# tizen_package_manager

 [![pub package](https://img.shields.io/pub/v/tizen_package_manager.svg)](https://pub.dev/packages/tizen_package_manager)

Tizen package manager API. Used for getting installed package info.

## Usage

To use this package, add `tizen_package_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_package_manager: ^0.1.0
```

### Retrieving specific package info

To retrieve information for the specific package,  get `PackageInfo` with `getPackageInfo` method.

```dart
var packageId = 'org.tizen.settings';
var packageInfo = await PackageManager.getPackageInfo(packageId);
```

### Retriving all package info

To retrive all package info for installed packages on a Tizen device, use `getInstalledApps` method.

```dart
var packageList = await PackageManager.getPackagesInfo();
for (var package  in packageList) {
  // Handle each package's info.
}
```

### Monitoring package events

You can listen for package events by subscribing to the stream.

```dart
  final List<StreamSubscription<PackageEvent>> _subscriptions =
      <StreamSubscription<PackageEvent>>[];

@override
void initState() {
  super.initState();

  _subscriptions.add(PackageManager.onInstallProgressChanged
      .listen((PackageEvent event) {
      // Got a installed package event.
  }));
  _subscriptions.add(PackageManager.onUninstallProgressChanged
      .listen((PackageEvent event) {
      // Got a uninstalled package event.
  }));
  _subscriptions.add(PackageManager.onUpdateProgressChanged
      .listen((PackageEvent event) {
      // Got a updated package event.
  }));
}

@override
void dispose() {
  for (final StreamSubscription<PackageEvent> subscription
      in _subscriptions) {
    subscription.cancel();
  }
  _subscriptions.clear();
  super.dispose();
}
```

## Required privileges

To enable your application to use the package manager functionality. Add required privileges in tizen-manifest.xml of your application.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/packagemanager.info</privilege>
</privileges>
```
