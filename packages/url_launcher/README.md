# url_launcher_tizen

[![pub package](https://img.shields.io/pub/v/url_launcher_tizen.svg)](https://pub.dev/packages/url_launcher_tizen)

The Tizen implementation of [`url_launcher`](https://pub.dev/packages/url_launcher).

## Usage

This package is not an _endorsed_ implementation of `url_launcher`. Therefore, you have to include `url_launcher_tizen` alongside `url_launcher` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  url_launcher: ^6.3.1
  url_launcher_tizen: ^2.1.4
```

Then you can import `url_launcher` in your Dart code:

```dart
import 'package:url_launcher/url_launcher.dart';
```

For detailed usage, see https://pub.dev/packages/url_launcher#usage.

## Required privileges

To use this plugin in a Tizen application, the application manager privilege is required. Add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```

For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).

## Notes

- A `PlatformException` is raised if no application on the device can open the provided URL.
- Only the default launch mode (`LaunchMode.platformDefault`) is supported on Tizen.
