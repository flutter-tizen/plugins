# url_launcher_tizen

[![pub package](https://img.shields.io/pub/v/url_launcher_tizen.svg)](https://pub.dev/packages/url_launcher_tizen)

The Tizen implementation of [`url_launcher`](https://github.com/flutter/plugins/tree/master/packages/url_launcher).

## Usage

This package is not an _endorsed_ implementation of `url_launcher`. Therefore, you have to include `url_launcher_tizen` alongside `url_launcher` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  url_launcher: ^6.0.9
  url_launcher_tizen: ^2.0.1
```

Then you can import `url_launcher` in your Dart code:

```dart
import 'package:url_launcher/url_launcher.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/url_launcher/url_launcher#usage.

## Required privileges

To use this plugin in a Tizen application, the application manager privilege is required. Add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```

For details, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

## Notes

- A `PlatformException` is raised if no application on the device can open the provided URL.
- The `launch` method's optional parameters (e.g. `useWebView`) are not currently supported on Tizen.
