# path_provider_tizen

[![pub package](https://img.shields.io/pub/v/path_provider_tizen.svg)](https://pub.dev/packages/path_provider_tizen)

The Tizen implementation of [`path_provider`](https://pub.dev/packages/path_provider).

## Usage

This package is not an _endorsed_ implementation of `path_provider`. Therefore, you have to include `path_provider_tizen` alongside `path_provider` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  path_provider: ^2.0.7
  path_provider_tizen: ^2.1.1
```

Then you can import `path_provider` in your Dart code:

```dart
import 'package:path_provider/path_provider.dart';
```

For detailed usage, see https://pub.dev/packages/path_provider#usage.

## Supported APIs

- [x] `getTemporaryDirectory` (returns the app's cache directory path)
- [x] `getApplicationSupportDirectory` (returns the app's data directory path)
- [ ] `getLibraryDirectory` (iOS-only)
- [x] `getApplicationDocumentsDirectory` (returns the app's data directory path)
- [x] `getExternalStorageDirectory` (requires an SD card)
- [x] `getExternalCacheDirectories` (requires an SD card)
- [x] `getExternalStorageDirectories` (returns shared media library paths such as `/home/owner/media/Music`)
- [ ] `getDownloadsDirectory` (desktop-only)

## Required privileges

- To access paths returned by `getExternalStorageDirectories`, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

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
