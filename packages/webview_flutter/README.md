# webview_flutter_tizen

[![pub package](https://img.shields.io/pub/v/webview_flutter_tizen.svg)](https://pub.dev/packages/webview_flutter_tizen)

The Tizen implementation of [`webview_flutter`](https://github.com/flutter/plugins/tree/master/packages/webview_flutter).

## Supported devices

This plugin is only supported on TV devices running Tizen 5.5 or later.

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `webview_flutter`. Therefore, you have to include `webview_flutter_tizen` alongside `webview_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  webview_flutter_tizen: ^0.3.10
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

- This is an initial webview plugin for Tizen and is implemented based on Tizen Lightweight Web Engine (LWE). If you would like to know detailed specifications that the LWE supports, please refer to the following link :
https://review.tizen.org/gerrit/gitweb?p=platform/upstream/lightweight-web-engine.git;a=blob;f=docs/Spec.md;h=ecb8f437c5a1facc77d3435e1a8aad6a267f12f3;hb=refs/heads/tizen
