# webview_flutter_tizen

[![pub package](https://img.shields.io/pub/v/webview_flutter_tizen.svg)](https://pub.dev/packages/webview_flutter_tizen)

The Tizen implementation of [`webview_flutter`](https://github.com/flutter/plugins/tree/main/packages/webview_flutter).

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `webview_flutter`. Therefore, you have to include `webview_flutter_tizen` alongside `webview_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  webview_flutter: ^3.0.4
  webview_flutter_tizen: ^0.5.6
```

## Example

```dart
import 'package:webview_flutter/webview_flutter.dart';

class WebViewExample extends StatefulWidget {
  const WebViewExample({Key? key}) : super(key: key);

  @override
  WebViewExampleState createState() => WebViewExampleState();
}

class WebViewExampleState extends State<WebViewExample> {
  @override
  Widget build(BuildContext context) {
    return WebView(initialUrl: 'https://flutter.dev');
  }
}
```

## Supported devices

This plugin is supported on devices running Tizen 5.5 or later.

The WebView widget is backed by the Lightweight Web Engine (LWE) on Tizen. For a detailed list of features supported by the Lightweight Web Engine, refer to [this page](https://git.tizen.org/cgit/platform/upstream/lightweight-web-engine/tree/docs/Spec.md?h=tizen).
