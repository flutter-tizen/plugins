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

The bundle content is in the form of key-value pairs. The key is always a `String`. The value is either a `String`, a `List<String>`, or a `Uint8List`.

Bundles can be treated like a `Map`. You can use the `[]` operator or `Bundle.addAll()` to add data to a bundle.

```dart
import 'dart:typed_data';
import 'package:tizen_bundle/tizen_bundle.dart';

var bundle = Bundle();
bundle['string'] = 'value';
bundle['strings'] = <String>['value1', 'value2', 'value3'];
bundle['bytes'] = Uint8List.fromList(<int>[0x01, 0x02, 0x03]);
```

### Accessing the bundle content

To get data from a bundle (or update their values), use the `[]` operator.

```dart
var stringValue = bundle['string'];
if (stringValue is String) {
  print('string: $stringValue');
}

var stringsValue = bundle['strings'];
if (stringsValue is List<String>) {
  print('strings: $stringsValue');
}

var bytesValue = bundle['bytes'];
if (bytesValue is Uint8List) {
  print('bytes: $bytesValue');
}
```

To remove a key-value pair from a bundle, use `Bundle.remove()`.

```dart
if (bundle.containsKey('string')) {
  bundle.remove('string');
}
```

### Encoding and decoding a bundle

To store bundle data to a file or send over a connection, you can encode it to a string using `Bundle.encode()`. To decode the string back to a bundle, use `Bundle.decode()`.

```dart
var bundle = Bundle();
bundle['key'] = 'value';

var encoded = bundle.encode();
var newBundle = Bundle.decode(encoded);

print(newBundle['key']);
```
