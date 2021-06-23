# path_provider_tizen

The Tizen implementation of [`path_provider`](https://github.com/flutter/plugins/tree/master/packages/path_provider).

## Usage

This package is not an _endorsed_ implementation of `path_provider`. Therefore, you have to include `path_provider_tizen` alongside `path_provider` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  path_provider: ^2.0.1
  path_provider_tizen: ^2.0.0
```

Then you can import `path_provider` in your Dart code:

```dart
import 'package:path_provider/path_provider.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/path_provider/path_provider#usage.

## Notes

- On Tizen, `getExternalStorageDirectories` will return internal storage paths (such as `/home/owner/media/Music`) unlike on Android where the function returns external storage (separate partition or SD card) paths.
- To access paths returned by `getExternalStorageDirectory` and `getExternalCacheDirectories`, you will need an SD card inserted to your Tizen device.

## Required privileges

- To access paths returned by

  - `getExternalStorageDirectories`
  - `getDownloadsDirectory`

  add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

  ```xml
  <privileges>
    <privilege>http://tizen.org/privilege/mediastorage</privilege>
  </privileges>
  ```

  and also acquire the `Permission.mediaLibrary` permission using the [`permission_handler`](https://pub.dev/packages/permission_handler_tizen) plugin. The permission is already granted on TV devices by default.

- To access paths returned by

  - `getExternalStorageDirectory`
  - `getExternalCacheDirectories`

  add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

  ```xml
  <privileges>
    <privilege>http://tizen.org/privilege/externalstorage.appdata</privilege>
  </privileges>
  ```
