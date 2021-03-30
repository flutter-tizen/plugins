# image_picker_tizen

The Tizen implementation of [`image_picker`](https://github.com/flutter/plugins/tree/master/packages/image_picker).

## Usage

To use this plugin, add `image_picker` and `image_picker_tizen` as [dependencies in your pubspec.yaml file](https://flutter.io/platform-plugins/).

```yaml
dependencies:
  image_picker: ^0.7.3
  image_picker_tizen: ^2.0.0
```

## Example

Import the library.

``` dart
import 'package:image_picker/image_picker.dart';
```

Then invoke the static `image_picker` method anywhere in your Dart code.

``` dart
final picker = ImagePicker();
final pickedFile = await picker.getImage(source: ImageSource.gallery);
```

## Limitations

- This plugin is only supported on **Galaxy Watch** devices running Tizen 5.5 or later.
- The only API you can use is `getImage(source: ImageSource.gallery)`. You can't pick a video file (`getVideo()`) or pick a file from `ImageSource.camera`.

## Required privileges

To use this plugin, you need to declare privileges in `tizen-manifest.xml` of your application.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```
