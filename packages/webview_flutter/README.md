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
  webview_flutter: ^4.4.2
  webview_flutter_tizen: ^0.9.1
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

## Note

- To play Youtube, make app's background color to transparent.

```diff
--- a/packages/webview_flutter/example/lib/main.dart
+++ b/packages/webview_flutter/example/lib/main.dart
   @override
   Widget build(BuildContext context) {
     return Scaffold(
-       backgroundColor: Colors.green,
+       backgroundColor: Colors.transparent,
       appBar: AppBar(
         title: const Text('Flutter WebView example'),
```

- In Tizen 6.0, there were some devices that failed to create the web view. In this case, the creation failure is resolved by using the Upgrade Web Engine (UWE) internally. If you set the `WebViewController.tizenEnginePolicy` extension API to `true` before creating the `WebviewWidget`, the webview will internally search for another version of the engine. However, this API can be changed(or removed) at any time and is not officially guaranteed to work.

```dart
import 'package:webview_flutter_tizen/webview_flutter_tizen.dart';

WebViewController _controller;
_controller.tizenEnginePolicy = true;
```
