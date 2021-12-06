# image_picker_tizen

[![pub package](https://img.shields.io/pub/v/image_picker_tizen.svg)](https://pub.dev/packages/image_picker_tizen)

The Tizen implementation of [`image_picker`](https://github.com/flutter/plugins/tree/master/packages/image_picker).

## Usage

To use this plugin, add `image_picker` and `image_picker_tizen` as [dependencies in your pubspec.yaml file](https://flutter.io/platform-plugins/).

```yaml
dependencies:
  image_picker: ^0.8.4
  image_picker_tizen: ^2.1.0
```

Then you can import `image_picker` in your Dart code.

``` dart
import 'package:image_picker/image_picker.dart';

final ImagePicker picker = ImagePicker();
final XFile? image = await picker.pickImage(source: ImageSource.gallery);
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/image_picker/image_picker#example.

## Supported devices

- Galaxy Watch series (running Tizen 5.5 or later)

## Supported APIs

- [x] `ImagePicker.pickImage` (only `ImageSource.gallery` is available as `source`)
- [x] `ImagePicker.pickMultiImage`
- [ ] `ImagePicker.pickVideo` (no file manager app available)
- [ ] `ImagePicker.retrieveLostData` (Android-only)

## Required privileges

To use this plugin, you need to declare privileges in `tizen-manifest.xml` of your application.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```
