
# webview_flutter_tizen

The Tizen implementation of [`webview_flutter`](https://github.com/flutter/plugins/tree/master/packages/webview_flutter).

## Supported devices

This plugin is available on these types of devices:

- Galaxy Watch (running Tizen 5.5 or later)

## Usage

```yaml
dependencies:
  webview_flutter: ^1.0.6
  webview_flutter_tizen: ^0.0.1
```

To enable tizen implementation, set `WebView.platform = TizenWebView();` in `initState()`.
For example:

```dart
import 'dart:io';

import 'package:webview_flutter/webview_flutter.dart';
import 'package:webview_flutter_tizen/webview_flutter_tizen.dart';

class WebViewExample extends StatefulWidget {
  @override
  WebViewExampleState createState() => WebViewExampleState();
}

class WebViewExampleState extends State<WebViewExample> {
  @override
  void initState() {
    super.initState();
    WebView.platform = TizenWebView();
  }

  @override
  Widget build(BuildContext context) {
    return WebView(
      initialUrl: 'https://flutter.dev',
    );
  }
}
```