# webview_flutter_tizen

[![pub package](https://img.shields.io/pub/v/webview_flutter_tizen.svg)](https://pub.dev/packages/webview_flutter_tizen)

The Tizen implementation of [`webview_flutter`](https://pub.dev/packages/webview_flutter) for Tizen TV devices.

The WebView widget is backed by the EFL WebKit (EWK) on Tizen.

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
  webview_flutter: ^4.0.2
  webview_flutter_tizen: ^0.7.1
```

## Example

```dart
import 'package:webview_flutter/webview_flutter.dart';

class WebViewExample extends StatefulWidget {
  const WebViewExample({super.key});

  @override
  State<WebViewExample> createState() => _WebViewExampleState();
}

class _WebViewExampleState extends State<WebViewExample> {
  final WebViewController _controller = WebViewController();

  @override
  void initState() {
    super.initState();

    _controller.loadRequest(Uri.parse('https://flutter.dev'));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: WebViewWidget(controller: _controller),
    );
  }
}
```

## Supported devices

This plugin is only supported on Tizen TV devices running Tizen 5.5 or later.  
