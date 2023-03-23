# flutter_app_badger_tizen

[![pub package](https://img.shields.io/pub/v/flutter_app_badger_tizen.svg)](https://pub.dev/packages/flutter_app_badger_tizen)

The Tizen implementation of [`flutter_app_badger`](https://pub.dev/packages/flutter_app_badger).

## Usage

 This package is not an _endorsed_ implementation of `flutter_app_badger`. Therefore, you have to include `flutter_app_badger_tizen` alongside `flutter_app_badger` as dependencies in your `pubspec.yaml` file.

 ```yaml
dependencies:
  flutter_app_badger: ^1.5.0
  flutter_app_badger_tizen: ^0.1.0
```

Then you can import `flutter_app_badger` in your Dart code:

```dart
import 'package:flutter_app_badger/flutter_app_badger.dart';
```

For detailed usage, see https://pub.dev/packages/flutter_app_badger#getting-started.

## Required privileges

To use this plugin, you need to declare privileges in `tizen-manifest.xml` of your application.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/notification</privilege>
</privileges>
```

## Supported devices

- Galaxy Watch series (running Tizen 4.0 or later)

## Notes

You need to declare the following feature in your `tizen-manifest.xml` if you plan to release your app on the app store (to enable [feature-based filtering](https://docs.tizen.org/application/native/tutorials/details/app-filtering)).

```xml
<feature name="http://tizen.org/feature/badge"/>
```
