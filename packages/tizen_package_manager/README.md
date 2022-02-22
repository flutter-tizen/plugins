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

To retrieve information of a specific package, use the `getPackageInfo` method which returns an instance of  `PackageInfo`.

```dart
var packageId = 'org.tizen.settings';
var packageInfo = await PackageManager.getPackageInfo(packageId);
```

### Retrieving all packages' info

To retrieve information of all packages installed on a Tizen device, use `getPackagesInfo` method.

```dart
var packageList = await PackageManager.getPackagesInfo();
for (var package  in packageList) {
  // Handle each package's info.
}
```

### Monitoring package events

You can listen for package events using `PackageManager.onInstallProgressChanged`, `PackageManager.onUninstallProgressChanged`, and `PackageManager.onUpdateProgressChanged`.

```dart
_subscription = PackageManager.onInstallProgressChanged
    .listen((PackageEvent event) {
      // A package is being installed.
});

_subscription.cancel();
```

## Required privileges

Privileges are required to use the package manager functionality. Add required privileges in tizen-manifest.xml of your application.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/packagemanager.info</privilege>
  <!-- The below is optional for install/uninstall and platform privilge -->
  <privilege>http://tizen.org/privilege/packagemanager.admin</privilege>
</privileges>
```
