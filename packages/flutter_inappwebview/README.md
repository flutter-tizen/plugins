# flutter_inappwebview_tizen

[![pub package](https://img.shields.io/pub/v/flutter_inappwebview_tizen.svg)](https://pub.dev/packages/flutter_inappwebview_tizen)

The Tizen implementation of [`flutter_inappwebview`](https://pub.dev/packages/flutter_inappwebview).

This package follows the same EWK-backed offscreen rendering approach as
[`webview_flutter_tizen`](https://pub.dev/packages/webview_flutter_tizen) and
maps it onto the `flutter_inappwebview` platform interface. Only the surface
that maps cleanly onto the Tizen WebView (chromium-efl) is implemented; every
other API raises `UnsupportedError` (or the `UnimplementedError` produced by
the `flutter_inappwebview_platform_interface` defaults).

## Required privileges

Add the internet privilege to the app manifest:

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

```yaml
dependencies:
  flutter_inappwebview: ^6.1.5
  flutter_inappwebview_tizen: ^0.1.1
```

```dart
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';

class WebViewExample extends StatelessWidget {
  const WebViewExample({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: InAppWebView(
        initialUrlRequest: URLRequest(url: WebUri('https://flutter.dev')),
        initialSettings: InAppWebViewSettings(
          javaScriptEnabled: true,
          useShouldOverrideUrlLoading: true,
        ),
      ),
    );
  }
}
```

## Note

- To play Youtube(video player), make app's background color to transparent.

```diff
--- a/packages/flutter_inappwebview/example/lib/in_app_webiew_example.screen.dart
+++ b/packages/flutter_inappwebview/example/lib/in_app_webiew_example.screen.dart
@@ -105,14 +105,13 @@ class _InAppWebViewExampleScreenState extends State<InAppWebViewExampleScreen> {
   Widget build(BuildContext context) {
     return Scaffold(
       appBar: AppBar(title: Text("InAppWebView")),
+      backgroundColor: Colors.transparent,
       drawer: myDrawer(context: context),
```

## Supported

- `InAppWebView` platform view
- URL/file/data loading, back/forward/reload/stop, post requests
- JavaScript evaluation
- Navigation interception via `shouldOverrideUrlLoading`
- `onLoadStart`, `onLoadStop`, `onReceivedError`, `onProgressChanged`,
  `onTitleChanged`, `onConsoleMessage`, `onScrollChanged`,
  `onZoomScaleChanged`, `onUpdateVisitedHistory`, and JavaScript dialog
  callbacks (`onJsAlert`, `onJsConfirm`, `onJsPrompt`)
- A subset of `InAppWebViewSettings`: `javaScriptEnabled`, `supportZoom`,
  `userAgent`, `transparentBackground`, and `useShouldOverrideUrlLoading`
- `clearCache`, `clearAllCache`, `getDefaultUserAgent`, `handlesURLScheme`
- `CookieManager.deleteAllCookies()`

`InAppWebViewController.getDefaultUserAgent()` returns the EWK user agent that
was captured the first time an `InAppWebView` was created in the process; the
value is cached so it remains available after every webview is disposed.
Calling it before any `InAppWebView` has been created returns an empty string.

## Not supported

The following capabilities are intentionally out of scope and raise
`UnsupportedError`/`UnimplementedError` when invoked:

- `InAppBrowser`, `HeadlessInAppWebView`, `ChromeSafariBrowser`
- `InAppWebViewKeepAlive`, popup `windowId`, context menus, pull-to-refresh
- `FindInteractionController`, `PrintJobController`, `WebMessage*`,
  `WebStorage*`, `WebViewEnvironment`, `HttpAuthCredentialDatabase`
- JavaScript handlers/channels and user script injection
- Per-cookie mutation/query APIs (`setCookie`, `getCookie(s)`,
  `deleteCookie(s)`)
- Resource interception, custom-scheme loading, downloads, permission and
  geolocation prompts, render-process events, fullscreen, capture state,
  content-size, and other callbacks not listed above
- Multi-step `goBackOrForward`/`canGoBackOrForward`/`goTo`, `isLoading`,
  `getOriginalUrl`, `getSettings`, `reloadFromOrigin`, `pause`/`resume`
- DevTools, simulated requests, screenshot/PDF/web-archive capture, JS/CSS
  injection, service worker / proxy / tracing / path-handler controllers
