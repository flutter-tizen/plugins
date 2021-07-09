# webview_flutter_tizen_ewk

The Tizen implementation of [`webview_flutter`](https://github.com/flutter/plugins/tree/master/packages/webview_flutter).

## Supported devices

This plugin is available on these types of devices:

- Galaxy Watch or TV (running Tizen 5.5 or later)

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `webview_flutter`. Therefore, you have to include `webview_flutter_tizen_ewk` alongside `webview_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  webview_flutter: ^1.0.6
  webview_flutter_tizen_ewk: ^0.1.0
```

## Example

```dart
import 'dart:io';
import 'package:webview_flutter/webview_flutter.dart';

class WebViewExample extends StatefulWidget {
  @override
  WebViewExampleState createState() => WebViewExampleState();
}

class WebViewExampleState extends State<WebViewExample> {
  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return WebView(
      initialUrl: 'https://flutter.dev',
    );
  }
}
```

## Limitations
- This plugin is only supported on **Galaxy Watch and TV** devices running Tizen 5.5 or later.
- This is an initial webview plugin for Tizen and is implemented based on Tizen Chromium-EWK engine.