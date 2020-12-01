# path_provider_tizen

The Tizen implementation of [`path_provider`](https://github.com/flutter/plugins/tree/master/packages/path_provider).

## Usage

This package is not an _endorsed_ implementation of `path_provider`. Therefore, you have to include `path_provider_tizen` alongside `path_provider` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  path_provider: ^1.6.10
  path_provider_tizen: ^1.0.0
```

Then you can import `path_provider` in your Dart code:

```dart
import 'package:path_provider/path_provider.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/path_provider/path_provider#usage.

## Required privileges

To access paths returned by

- `getExternalDataPath`
- `getExternalCachePath`

add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privilege>http://tizen.org/privilege/externalstorage.appdata</privilege>
```

To access paths returned by

- `getExternalStoragePaths`
- `getDownloadsPath`

add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privilege>http://tizen.org/privilege/mediastorage</privilege>
```

and also acquire the **Storage** [permission](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges) using the [`permission_handler`](https://pub.dev/packages/permission_handler_tizen) plugin (to be available soon).
