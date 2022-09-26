# tizen_bundle

[![pub package](https://img.shields.io/pub/v/tizen_bundle.svg)](https://pub.dev/packages/tizen_bundle)

Tizen bundle APIs.

## Usage

To use this package, add `tizen_bundle` as a dependency in your `pubspec.yaml` file.

```yaml
depenedencies:
  tizen_bundle: ^0.1.0
```

### Adding Content to a Bundle

The bundle content is in the from of key-value pairs. The key is always a string. The value can be of the following types:

**Table: Bundle value types**
| Type enum                | Dart type         |
|--------------------------|-------------------|
| `BundleType.string`      | String            |
| `BundleType.strings`     | List<String>      |
| `BundleType.bytes`       | Uint8List         |

To add content to a bundle, use following methods or `operator [](String key, Object value)` you want to add:

- `Bundle.addAll()`
- `Bundle.addEntries()`

```dart
import 'package:tizen_bundle/tizen_bundle.dart';

var bundle = Bundle();
bundle['string'] = 'stringValue';

List<String> values = ['stringValue1', 'stringValue2', 'stringValue3'];
bundle['strings'] = values;

Uint8List bytes = Uint8List(3);
bytes[0] = 0x01;
bytes[1] = 0x02;
bytes[2] = 0x03;
bundle['bytes'] =  bytes;
```

## Managing the Bundle Content

To manage the bundle content:

1. Gets values from the bundle object using `operator [](String key)` you want to get :
   You can also get the number of the bundle items with the `Bundle.length` property, and check whether the value is the type or not with `is` keyword as below:

```dart
import 'package:tizen_log/tizen_log.dart';

String logTag = 'BundleTest';
Log.info(logTag, 'length: ${bundle.length}');
var stringValue = bundle['string'];
if (stringValue is String) {
  Log.info(logTag, 'string: $stringValue');
}

var stringsValue = bundle['strings'];
if (stringsValue is List<String>) {
  Log.info(logTag, 'strings: $stringsValue');
}

var bytesValue = bundle['bytes'];
if (bytesValue is Uint8List) {
  Log.info(logTag, 'bytes: $bytesValue');
}
```

2. Deletes a key-value pair from the bundle content using the `Bundle.remove()` method:

```dart
if (bundle.containsKey('string'))
  bundle.remove('string');

if (bundle.containsKey('strings'))
  bundle.remove('strings');

if (bundle.containsKey('bytes'))
  bundle.remove('bytes');
```

## Iterating the Bundle Content

To iterate through the bundle records, use the 'Bundle.keys' property:

```dart
final keys = bundle.keys;
keys.asMap().forEach((index, key) {
  Log.info(logTag, 'key: ${keyInfo.name}');
}

```

## Encoding and Decoding the Bundle

To store or send a bundle over a connection, encode it to `String` with the `Bundle.toRaw()` method.

To open the encoded bundle, use the `Bundle.fromRaw()` method.

```dart
var bundle = Bundle();
bundle.addString('key1', 'value1');

var raw = bundle.toRaw();

var newBundle = Bundle.fromRaw(raw);
Log.info(logTag, 'value: ${newBundle.getString('key')}');
```
