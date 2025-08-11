# tizen_package_manager

 [![pub package](https://img.shields.io/pub/v/tizen_package_manager.svg)](https://pub.dev/packages/tizen_package_manager)

Tizen package manager APIs. Used to get information about packages installed on a Tizen device.

## Usage

To use this package, add `tizen_package_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_package_manager: ^0.3.0
```

### Retrieving specific package info

To retrieve information of a specific package, use `PackageManager.getPackageInfo` which returns an instance of `PackageInfo`.

```dart
String packageId = 'org.tizen.settings';
PackageInfo package = await PackageManager.getPackageInfo(packageId);
```

### Retrieving all packages' info

To retrieve information of all packages installed on a Tizen device, use `PackageManager.getPackagesInfo`.

```dart
List<PackageInfo> packages = await PackageManager.getPackagesInfo();
```

### Monitoring package events

You can listen for package events using `onInstallProgressChanged`, `onUninstallProgressChanged`, and `onUpdateProgressChanged`.

```dart
_subscription = PackageManager.onInstallProgressChanged.listen((event) {
  // A package is being installed.
});
...
_subscription.cancel();
```

## Required privileges

Privileges are required to use the package manager functionality. Add required privileges in `tizen-manifest.xml` of your application.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/packagemanager.info</privilege>
  <!-- The below is optional and only required for installation and uninstallation of packages. -->
  <privilege>http://tizen.org/privilege/packagemanager.admin</privilege>
</privileges>
```

Note that `http://tizen.org/privilege/packagemanager.admin` is a platform-level privilege and can only be used by preloaded applications.
