# tizen_bundle

[![pub package](https://img.shields.io/pub/v/tizen_bundle.svg)](https://pub.dev/packages/tizen_bundle)

Tizen [Data Bundle](https://docs.tizen.org/application/native/guides/app-management/data-bundles) APIs.

## Usage

To use this package, add `tizen_bundle` as a dependency in your `pubspec.yaml` file.

```yaml
depenedencies:
  tizen_bundle: ^0.1.1
```

### Adding content to a bundle

The bundle content is in the form of key-value pairs. The key is always a `String`. The value must be either a `String`, a `List<String>`, or a `Uint8List`.

Bundles can be treated like a `Map`. You can use the `[]` operator or `Bundle.addAll()` to add data to a bundle.

```dart
import 'package:tizen_bundle/tizen_bundle.dart';

var bundle = Bundle();
bundle['string'] = 'value';
bundle['strings'] = <String>['value1', 'value2', 'value3'];
bundle['bytes'] = Uint8List.fromList(<int>[0x01, 0x02, 0x03]);
```

### Accessing the bundle content

To get data from a bundle or update their values, use the `[]` operator.

```dart
var stringValue = bundle['string'];
if (stringValue is String) {
  // The value is a string.
}

var stringsValue = bundle['strings'];
if (stringsValue is List<String>) {
  // The value is a string list.
}

var bytesValue = bundle['bytes'];
if (bytesValue is Uint8List) {
  // The value is a byte list.
}
```

You can also use other [methods and properties](https://api.flutter.dev/flutter/dart-collection/MapMixin-class.html) supported by a `Map`, such as `containsKey` and `remove`.

```dart
if (bundle.containsKey('string')) {
  bundle.remove('string');
}
```

### Encoding and decoding a bundle

To save bundle data to a file or send over network, you can encode them to a raw string using `Bundle.encode()`. To restore the bundle from the encoded string, use `Bundle.decode()`.

```dart
var bundle = Bundle();
bundle['key'] = 'value';

var encoded = bundle.encode();
var newBundle = Bundle.decode(encoded);
```
