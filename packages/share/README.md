# share_tizen

[![pub package](https://img.shields.io/pub/v/share_tizen.svg)](https://pub.dev/packages/share_tizen)

The Tizen implementation of [`share`](https://github.com/flutter/plugins/tree/master/packages/share).

## Usage

To use this plugin, add `share` and `share_tizen` as [dependencies in your pubspec.yaml file](https://flutter.io/platform-plugins/).

```yaml
dependencies:
  share: ^2.0.1
  share_tizen: ^2.0.0
```

## Example

Import the library.

``` dart
import 'package:share/share.dart';
```

Then invoke the static `share` method anywhere in your Dart code.

``` dart
Share.share('check out my website https://example.com');
```

## Limitations

- This plugin is only supported on **Galaxy Watch** devices running Tizen 4.0 or later.
- Passing in an optional argument `subject` to `Share.share()` or invoking `Share.shareFiles()` leads to a **PlatformException** because no e-mail app is available on watche devices.
- You cannot choose which application to use for sharing. Only the **Message** app can handle sharing requests.

## Required privileges

To use this plugin, you need to declare privileges in `tizen-manifest.xml` of your application.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
</privileges>
```
