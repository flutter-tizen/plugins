# tizen_bundle

Tizen bundle APIs.

## Usage

To use this package, add `tizen_bundle` as a dependency in your `pubspec.yaml` file.

```yaml
depenedencies:
  tizen_bundle:

```

## Adding Content to a Bundle

The bundle content is in the from of key-value pairs, The key is always a string. The value can be of the following types:

**Table: Bundle value types**
| Value constant           | Value type        |
|--------------------------|-------------------|
| `BundleType.string`      | String (default)  |
| `BundleType.strings`     | String list       |
| `BundleType.bytes`       | Bytes             |

To add content to a bundle, use a method associated with the type of the value you want to add:

- `Bundle.addString()`
- `Bundle.addStrings()`
- `Bundle.addBytes()`

```dart
import 'package:tizen_bundle/tizen_bundle.dart';

var bundle = Bundle();
bundle.addString('string', 'stringValue');

List<String> values = ['stringValue1', 'stringValue2', 'stringValue3'];
bundle.addStrings('strings', values);

List<int> bytes[] = [0x01, 0x02, 0x03];
bundle.addBytes('bytes', bytes);
```

When you no longer needed the bundle object, release it using the `Bundle.dispose()` method as below:

``` dart
bundle.dispose();
```

## Managing the Bundle Content

To manage the bundle content:

1. Gets values from the bundle object using the method associated with the type of the value you want to get :

- `Bundle.getString()`
- `Bundle.getStrings()`
- `Bundle.getBytes()`

  You can also get the number of the bundle items with the `Bundle.length` property, and the type of a value with a specific key with the `Bundle.getType()` method.

```dart
import 'package:tizen_log/tizen_log.dart';

String logTag = 'BundleTest';
Log.info(logTag, 'length: ${bundle.length}');
if (bundle.getType('string') == BundleType.string) {
  var stringValue = bundle.getString("string");
  Log.info(logTag, 'string: $stringValue');
}

if (bundle.getType('strings') == BundleType.strings) {
  var stringsValue = bundle.getStrings("strings");
  Log.info(logTag, 'strings: $stringsValue');
}

if (bundle.getType('bytes') == BundleType.bytes) {
  var bytesValue = bundle.getBytes('bytes');
  Log.info(logTag, 'bytes: $bytesValue');
}
```

2. Deletes a key-value pair from the bundle content using the `Bundle.delete()` method:

```dart
if (bundle.contains('string'))
  bundle.delete('string');

if (bundle.contains('strings'))
  bundle.delete('strings');

if (bundle.contains('bytes'))
  bundle.delete('bytes');
```

## Iterating the Bundle Content

To iterate through the bundle records, use the 'Bundle.GetKeys()' method:

```dart
final keyInfos = bundle.getKeys();
keyInfos.asMap().forEach((index, keyInfo) {
for (var index = 0; index < keys.length; ++index) {
  Log.info(logTag, 'key: ${keyInfo.name}');
  if (keyInfo.type == BundlType.string) {
    Log.info(logTag, 'value: ${bundle.getString(keyInfo.name)}');
  } else if (keyInfo.type == BundleType.strings) {
    Log.info(logTag, 'value: ${bundle.getStrings(keyInfo.name)}');
  } else if (keyInfo.type == BundleType.bytes) {
    Log.info(logTag, 'value: ${bundle.getBytes(keyInfo.name)}');
  }
}

```

## Encoding and Decoding the Bundle

To store or send a bundle over a connection, encode it to `String` with the `Bundle.toRaw()` method.

To open the encoded bundle, use the `Bundle.fromRaw()` method.

```dart
var bundle = Bundle();
bundle.addString('key1', 'value1');

var raw = bundle.toRaw();
bundle.dispose();

var newBundle = Bundle.fromRaw(raw);
Log.info(logTag, 'value: ${newBundle.getString('key')}');
newBundle.dispose();
```