# webview_flutter_lwe

[![pub package](https://img.shields.io/pub/v/webview_flutter_lwe.svg)](https://pub.dev/packages/webview_flutter_lwe)

The Tizen implementation of [`webview_flutter`](https://pub.dev/packages/webview_flutter) backed by the Lightweight Web Engine (LWE).

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `webview_flutter`. Therefore, you have to include `webview_flutter_lwe` alongside `webview_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  webview_flutter: ^4.13.1
  webview_flutter_lwe: ^0.4.0
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

## Limitations

### Scrollbar visibility

The `setVerticalScrollBarEnabled` and `setHorizontalScrollBarEnabled` methods are not independently controllable. Calling either method will affect both vertical and horizontal scrollbars simultaneously.

If you need to hide scrollbars, call only one of these methods with `false`. Setting different values for each scrollbar (e.g., vertical enabled but horizontal disabled) is not supported.

```dart
// This will hide both vertical and horizontal scrollbars
await controller.setVerticalScrollBarEnabled(false);

// This will show both vertical and horizontal scrollbars
await controller.setHorizontalScrollBarEnabled(true);
```

## Supported devices

This plugin is supported on devices running Tizen 5.5 or later.

For a detailed list of features supported by the Lightweight Web Engine, refer to [this page](https://git.tizen.org/cgit/platform/upstream/lightweight-web-engine/tree/docs/Spec.md?h=tizen).
