// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This test is run using `flutter drive` by the CI (see /script/tool/README.md
// in this repository for details on driving that tooling manually), but can
// also be run using `flutter test` directly during development.

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:webview_flutter/webview_flutter.dart';

Future<void> main() async {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  final HttpServer server = await HttpServer.bind(InternetAddress.anyIPv4, 0);
  unawaited(
    server.forEach((HttpRequest request) {
      if (request.uri.path == '/hello.txt') {
        request.response.writeln('Hello, world.');
      } else if (request.uri.path == '/secondary.txt') {
        request.response.writeln('How are you today?');
      } else if (request.uri.path == '/headers') {
        request.response.writeln('${request.headers}');
      } else if (request.uri.path == '/favicon.ico') {
        request.response.statusCode = HttpStatus.notFound;
      } else {
        fail('unexpected request: ${request.method} ${request.uri}');
      }
      request.response.close();
    }),
  );
  final String prefixUrl = 'http://${server.address.address}:${server.port}';
  final String primaryUrl = '$prefixUrl/hello.txt';
  final String secondaryUrl = '$prefixUrl/secondary.txt';

  testWidgets('loadRequest', (WidgetTester tester) async {
    final Completer<void> pageFinished = Completer<void>();

    final WebViewController controller = WebViewController();
    unawaited(
      controller.setNavigationDelegate(
        NavigationDelegate(onPageFinished: (_) => pageFinished.complete()),
      ),
    );
    unawaited(controller.loadRequest(Uri.parse(primaryUrl)));

    await tester.pumpWidget(WebViewWidget(controller: controller));

    await pageFinished.future;

    final String? currentUrl = await controller.currentUrl();
    expect(currentUrl, primaryUrl);
  });

  testWidgets('runJavaScriptReturningResult', (WidgetTester tester) async {
    final Completer<void> pageFinished = Completer<void>();

    final WebViewController controller = WebViewController();
    unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
    unawaited(
      controller.setNavigationDelegate(
        NavigationDelegate(onPageFinished: (_) => pageFinished.complete()),
      ),
    );
    unawaited(controller.loadRequest(Uri.parse(primaryUrl)));

    await tester.pumpWidget(WebViewWidget(controller: controller));

    await pageFinished.future;

    await expectLater(
      controller.runJavaScriptReturningResult('1 + 1'),
      completion(2),
    );
  });

  testWidgets('JavascriptChannel', (WidgetTester tester) async {
    final Completer<void> pageFinished = Completer<void>();
    final WebViewController controller = WebViewController();
    unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
    unawaited(
      controller.setNavigationDelegate(
        NavigationDelegate(onPageFinished: (_) => pageFinished.complete()),
      ),
    );

    final Completer<String> channelCompleter = Completer<String>();
    await controller.addJavaScriptChannel(
      'Echo',
      onMessageReceived: (JavaScriptMessage message) {
        channelCompleter.complete(message.message);
      },
    );

    await controller.loadHtmlString(
      'data:text/html;charset=utf-8;base64,PCFET0NUWVBFIGh0bWw+',
    );

    await tester.pumpWidget(WebViewWidget(controller: controller));

    await pageFinished.future;

    await controller.runJavaScript('Echo.postMessage("hello");');
    await expectLater(channelCompleter.future, completion('hello'));
  });

  testWidgets('resize webview', (WidgetTester tester) async {
    final Completer<void> initialResizeCompleter = Completer<void>();
    final Completer<void> buttonTapResizeCompleter = Completer<void>();
    final Completer<void> onPageFinished = Completer<void>();

    bool resizeButtonTapped = false;
    await tester.pumpWidget(
      ResizableWebView(
        onResize: () {
          if (resizeButtonTapped) {
            buttonTapResizeCompleter.complete();
          } else {
            initialResizeCompleter.complete();
          }
        },
        onPageFinished: () => onPageFinished.complete(),
      ),
    );

    await onPageFinished.future;
    // Wait for a potential call to resize after page is loaded.
    await initialResizeCompleter.future.timeout(
      const Duration(seconds: 3),
      onTimeout: () => null,
    );

    resizeButtonTapped = true;

    await tester.tap(find.byKey(const ValueKey<String>('resizeButton')));
    await tester.pumpAndSettle();

    await expectLater(buttonTapResizeCompleter.future, completes);
  });

  testWidgets('set custom userAgent', (WidgetTester tester) async {
    final Completer<void> pageFinished = Completer<void>();

    final WebViewController controller = WebViewController();
    unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
    unawaited(
      controller.setNavigationDelegate(
        NavigationDelegate(onPageFinished: (_) => pageFinished.complete()),
      ),
    );
    unawaited(controller.setUserAgent('Custom_User_Agent1'));
    unawaited(controller.loadRequest(Uri.parse('about:blank')));

    await tester.pumpWidget(WebViewWidget(controller: controller));

    await pageFinished.future;

    final String? customUserAgent = await controller.getUserAgent();
    expect(customUserAgent, 'Custom_User_Agent1');
  });

  testWidgets('getTitle', (WidgetTester tester) async {
    const String getTitleTest = '''
        <!DOCTYPE html><html>
        <head><title>Some title</title>
        </head>
        <body>
        </body>
        </html>
      ''';
    final String getTitleTestBase64 = base64Encode(
      const Utf8Encoder().convert(getTitleTest),
    );
    final Completer<void> pageLoaded = Completer<void>();

    final WebViewController controller = WebViewController();
    unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
    unawaited(
      controller.setNavigationDelegate(
        NavigationDelegate(onPageFinished: (_) => pageLoaded.complete()),
      ),
    );
    unawaited(
      controller.loadRequest(
        Uri.parse('data:text/html;charset=utf-8;base64,$getTitleTestBase64'),
      ),
    );

    await tester.pumpWidget(WebViewWidget(controller: controller));

    await pageLoaded.future;

    // On at least iOS, it does not appear to be guaranteed that the native
    // code has the title when the page load completes. Execute some JavaScript
    // before checking the title to ensure that the page has been fully parsed
    // and processed.
    await controller.runJavaScript('1;');

    final String? title = await controller.getTitle();
    expect(title, 'Some title');
  });

  group('Programmatic Scroll', () {
    testWidgets('setAndGetScrollPosition', (WidgetTester tester) async {
      const String scrollTestPage = '''
        <!DOCTYPE html>
        <html>
          <head>
            <style>
              body {
                height: 100%;
                width: 100%;
              }
              #container{
                width:5000px;
                height:5000px;
            }
            </style>
          </head>
          <body>
            <div id="container"/>
          </body>
        </html>
      ''';

      final String scrollTestPageBase64 = base64Encode(
        const Utf8Encoder().convert(scrollTestPage),
      );

      final Completer<void> pageLoaded = Completer<void>();
      final WebViewController controller = WebViewController();
      unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
      unawaited(
        controller.setNavigationDelegate(
          NavigationDelegate(onPageFinished: (_) => pageLoaded.complete()),
        ),
      );
      unawaited(
        controller.loadRequest(
          Uri.parse(
            'data:text/html;charset=utf-8;base64,$scrollTestPageBase64',
          ),
        ),
      );

      await tester.pumpWidget(WebViewWidget(controller: controller));

      await pageLoaded.future;

      await tester.pumpAndSettle(const Duration(seconds: 3));

      Offset scrollPos = await controller.getScrollPosition();

      // Check scrollTo()
      const int X_SCROLL = 123;
      const int Y_SCROLL = 321;
      // Get the initial position; this ensures that scrollTo is actually
      // changing something, but also gives the native view's scroll position
      // time to settle.
      expect(scrollPos.dx, isNot(X_SCROLL));
      expect(scrollPos.dy, isNot(Y_SCROLL));

      await controller.scrollTo(X_SCROLL, Y_SCROLL);
      scrollPos = await controller.getScrollPosition();
      expect(scrollPos.dx, X_SCROLL);
      expect(scrollPos.dy, Y_SCROLL);

      // Check scrollBy() (on top of scrollTo())
      await controller.scrollBy(X_SCROLL, Y_SCROLL);
      scrollPos = await controller.getScrollPosition();
      expect(scrollPos.dx, X_SCROLL * 2);
      expect(scrollPos.dy, Y_SCROLL * 2);
    });
  });

  group('NavigationDelegate', () {
    const String blankPage = '<!DOCTYPE html><head></head><body></body></html>';
    final String blankPageEncoded = 'data:text/html;charset=utf-8;base64,'
        '${base64Encode(const Utf8Encoder().convert(blankPage))}';

    testWidgets('can allow requests', (WidgetTester tester) async {
      Completer<void> pageLoaded = Completer<void>();

      final WebViewController controller = WebViewController();
      unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
      unawaited(
        controller.setNavigationDelegate(
          NavigationDelegate(
            onPageFinished: (_) => pageLoaded.complete(),
            onNavigationRequest: (NavigationRequest navigationRequest) {
              return (navigationRequest.url.contains('youtube.com'))
                  ? NavigationDecision.prevent
                  : NavigationDecision.navigate;
            },
          ),
        ),
      );

      await tester.pumpWidget(WebViewWidget(controller: controller));

      unawaited(controller.loadRequest(Uri.parse(blankPageEncoded)));

      await pageLoaded.future; // Wait for initial page load.

      pageLoaded = Completer<void>();
      await controller.runJavaScript('location.href = "$secondaryUrl"');
      await pageLoaded.future; // Wait for the next page load.

      final String? currentUrl = await controller.currentUrl();
      expect(currentUrl, secondaryUrl);
    });

    testWidgets('onWebResourceError', (WidgetTester tester) async {
      final Completer<WebResourceError> errorCompleter =
          Completer<WebResourceError>();

      final WebViewController controller = WebViewController();
      unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
      unawaited(
        controller.setNavigationDelegate(
          NavigationDelegate(
            onWebResourceError: (WebResourceError error) {
              errorCompleter.complete(error);
            },
          ),
        ),
      );
      unawaited(
        controller.loadRequest(Uri.parse('https://www.notawebsite..com')),
      );

      await tester.pumpWidget(WebViewWidget(controller: controller));

      final WebResourceError error = await errorCompleter.future;
      expect(error, isNotNull);
    });

    testWidgets('onWebResourceError is not called with valid url', (
      WidgetTester tester,
    ) async {
      final Completer<WebResourceError> errorCompleter =
          Completer<WebResourceError>();
      final Completer<void> pageFinishCompleter = Completer<void>();

      final WebViewController controller = WebViewController();
      unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
      unawaited(
        controller.setNavigationDelegate(
          NavigationDelegate(
            onPageFinished: (_) => pageFinishCompleter.complete(),
            onWebResourceError: (WebResourceError error) {
              errorCompleter.complete(error);
            },
          ),
        ),
      );
      unawaited(
        controller.loadRequest(
          Uri.parse('data:text/html;charset=utf-8;base64,PCFET0NUWVBFIGh0bWw+'),
        ),
      );

      await tester.pumpWidget(WebViewWidget(controller: controller));

      expect(errorCompleter.future, doesNotComplete);
      await pageFinishCompleter.future;
    });

    testWidgets('can block requests', (WidgetTester tester) async {
      Completer<void> pageLoaded = Completer<void>();

      final WebViewController controller = WebViewController();
      unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
      unawaited(
        controller.setNavigationDelegate(
          NavigationDelegate(
            onPageFinished: (_) => pageLoaded.complete(),
            onNavigationRequest: (NavigationRequest navigationRequest) {
              return (navigationRequest.url.contains('youtube.com'))
                  ? NavigationDecision.prevent
                  : NavigationDecision.navigate;
            },
          ),
        ),
      );

      await tester.pumpWidget(WebViewWidget(controller: controller));

      unawaited(controller.loadRequest(Uri.parse(blankPageEncoded)));

      await pageLoaded.future; // Wait for initial page load.

      pageLoaded = Completer<void>();
      await controller.runJavaScript(
        'location.href = "https://www.youtube.com/"',
      );

      // There should never be any second page load, since our new URL is
      // blocked. Still wait for a potential page change for some time in order
      // to give the test a chance to fail.
      await pageLoaded.future.timeout(
        const Duration(milliseconds: 500),
        onTimeout: () => '',
      );
      final String? currentUrl = await controller.currentUrl();
      expect(currentUrl, isNot(contains('youtube.com')));
    });

    testWidgets('supports asynchronous decisions', (WidgetTester tester) async {
      Completer<void> pageLoaded = Completer<void>();

      final WebViewController controller = WebViewController();
      unawaited(controller.setJavaScriptMode(JavaScriptMode.unrestricted));
      unawaited(
        controller.setNavigationDelegate(
          NavigationDelegate(
            onPageFinished: (_) => pageLoaded.complete(),
            onNavigationRequest: (NavigationRequest navigationRequest) async {
              NavigationDecision decision = NavigationDecision.prevent;
              decision = await Future<NavigationDecision>.delayed(
                const Duration(milliseconds: 10),
                () => NavigationDecision.navigate,
              );
              return decision;
            },
          ),
        ),
      );

      await tester.pumpWidget(WebViewWidget(controller: controller));

      unawaited(controller.loadRequest(Uri.parse(blankPageEncoded)));

      await pageLoaded.future; // Wait for initial page load.

      pageLoaded = Completer<void>();
      await controller.runJavaScript('location.href = "$secondaryUrl"');
      await pageLoaded.future; // Wait for second page to load.

      final String? currentUrl = await controller.currentUrl();
      expect(currentUrl, secondaryUrl);
    });
  });
}

class ResizableWebView extends StatefulWidget {
  const ResizableWebView({
    super.key,
    required this.onResize,
    required this.onPageFinished,
  });

  final VoidCallback onResize;
  final VoidCallback onPageFinished;

  @override
  State<StatefulWidget> createState() => ResizableWebViewState();
}

class ResizableWebViewState extends State<ResizableWebView> {
  late final WebViewController controller = WebViewController()
    ..setJavaScriptMode(JavaScriptMode.unrestricted)
    ..setNavigationDelegate(
      NavigationDelegate(onPageFinished: (_) => widget.onPageFinished()),
    )
    ..addJavaScriptChannel(
      'Resize',
      onMessageReceived: (_) {
        widget.onResize();
      },
    )
    ..loadRequest(
      Uri.parse(
        'data:text/html;charset=utf-8;base64,${base64Encode(const Utf8Encoder().convert(resizePage))}',
      ),
    );

  double webViewWidth = 200;
  double webViewHeight = 200;

  static const String resizePage = '''
        <!DOCTYPE html><html>
        <head><title>Resize test</title>
          <script type="text/javascript">
            function onResize() {
              Resize.postMessage("resize");
            }
            function onLoad() {
              window.onresize = onResize;
            }
          </script>
        </head>
        <body onload="onLoad();" bgColor="blue">
        </body>
        </html>
      ''';

  @override
  Widget build(BuildContext context) {
    return Directionality(
      textDirection: TextDirection.ltr,
      child: Column(
        children: <Widget>[
          SizedBox(
            width: webViewWidth,
            height: webViewHeight,
            child: WebViewWidget(controller: controller),
          ),
          TextButton(
            key: const Key('resizeButton'),
            onPressed: () {
              setState(() {
                webViewWidth += 100.0;
                webViewHeight += 100.0;
              });
            },
            child: const Text('ResizeButton'),
          ),
        ],
      ),
    );
  }
}
