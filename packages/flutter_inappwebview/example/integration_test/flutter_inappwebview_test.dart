// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

const String _fixtureHtml = '''
<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Fixture title</title>
    <style>
      html, body {
        margin: 0;
        padding: 0;
      }
      #page {
        min-width: 2200px;
        min-height: 2200px;
        padding: 16px;
        box-sizing: border-box;
      }
    </style>
  </head>
  <body>
    <div id="page">
      <h1>Fixture Page</h1>
    </div>
  </body>
</html>
''';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  late HttpServer server;
  late String firstUrl;
  late String secondUrl;
  late String blockedUrl;

  setUpAll(() async {
    server = await HttpServer.bind(InternetAddress.loopbackIPv4, 0);
    unawaited(
      server.forEach((HttpRequest request) {
        request.response.headers.contentType = ContentType.html;
        switch (request.uri.path) {
          case '/first':
            request.response.write(_htmlPage('First page'));
          case '/second':
            request.response.write(_htmlPage('Second page'));
          case '/blocked':
            request.response.write(_htmlPage('Blocked page'));
          case '/favicon.ico':
            request.response.statusCode = HttpStatus.notFound;
          default:
            fail('unexpected request: ${request.method} ${request.uri}');
        }
        request.response.close();
      }),
    );
    final String baseUrl = 'http://${server.address.address}:${server.port}';
    firstUrl = '$baseUrl/first';
    secondUrl = '$baseUrl/second';
    blockedUrl = '$baseUrl/blocked';
  });

  tearDownAll(() => server.close(force: true));

  testWidgets('load callbacks and navigation controls', (
    WidgetTester tester,
  ) async {
    final StreamController<String> loadStops =
        StreamController<String>.broadcast();
    final StreamController<int> progressValues =
        StreamController<int>.broadcast();
    final StreamController<String> consoleMessages =
        StreamController<String>.broadcast();
    final StreamController<String> navigationActions =
        StreamController<String>.broadcast();
    final StreamController<String> visitedUrls =
        StreamController<String>.broadcast();
    addTearDown(loadStops.close);
    addTearDown(progressValues.close);
    addTearDown(consoleMessages.close);
    addTearDown(navigationActions.close);
    addTearDown(visitedUrls.close);

    final Future<String> firstLoad = _waitForValue(loadStops.stream, firstUrl);
    final Future<int> initialProgress = progressValues.stream
        .firstWhere((int progress) => progress == 100)
        .timeout(const Duration(seconds: 10));
    final Future<String> consoleMessage = _waitForValue(
      consoleMessages.stream,
      'First page ready',
    );

    final InAppWebViewController controller = await _pumpWebView(
      tester,
      initialUrl: firstUrl,
      initialSettings: InAppWebViewSettings(useShouldOverrideUrlLoading: true),
      onLoadStop: (_, WebUri? url) {
        if (url != null) {
          loadStops.add(url.toString());
        }
      },
      onProgressChanged: (_, int progress) {
        progressValues.add(progress);
      },
      onConsoleMessage: (_, ConsoleMessage message) {
        consoleMessages.add(message.message);
      },
      onUpdateVisitedHistory: (_, WebUri? url, __) {
        if (url != null) {
          visitedUrls.add(url.toString());
        }
      },
      shouldOverrideUrlLoading: (_, NavigationAction action) async {
        final String? url = action.request.url?.toString();
        if (url != null) {
          navigationActions.add(url);
        }
        return url == blockedUrl
            ? NavigationActionPolicy.CANCEL
            : NavigationActionPolicy.ALLOW;
      },
    );

    expect(await firstLoad, firstUrl);
    expect(await initialProgress, 100);
    expect(await consoleMessage, 'First page ready');
    expect((await controller.getUrl()).toString(), firstUrl);
    expect(await controller.getTitle(), 'First page');

    final Future<String> secondNavigation = _waitForValue(
      navigationActions.stream,
      secondUrl,
    );
    final Future<String> secondLoad = _waitForValue(
      loadStops.stream,
      secondUrl,
    );
    final Future<String> secondVisited = _waitForValue(
      visitedUrls.stream,
      secondUrl,
    );

    await controller.evaluateJavascript(
      source: "document.getElementById('second-link').click();",
    );

    expect(await secondNavigation, secondUrl);
    expect(await secondLoad, secondUrl);
    expect(await secondVisited, secondUrl);
    expect(await controller.getTitle(), 'Second page');
    expect(await controller.canGoBack(), isTrue);

    final Future<String> firstReloaded = _waitForValue(
      loadStops.stream,
      firstUrl,
    );
    await controller.goBack();
    expect(await firstReloaded, firstUrl);
    expect((await controller.getUrl()).toString(), firstUrl);

    final Future<String> blockedNavigation = _waitForValue(
      navigationActions.stream,
      blockedUrl,
    );
    final Future<String> blockedLoad = _waitForValue(
      loadStops.stream,
      blockedUrl,
      timeout: const Duration(seconds: 1),
    );
    await controller.evaluateJavascript(
      source: "document.getElementById('blocked-link').click();",
    );
    expect(await blockedNavigation, blockedUrl);
    await expectLater(blockedLoad, throwsA(isA<TimeoutException>()));
    expect((await controller.getUrl()).toString(), firstUrl);
  });

  testWidgets('loadData and evaluate JavaScript', (WidgetTester tester) async {
    final InAppWebViewController controller = await _pumpWebView(tester);

    await _loadFixture(controller);

    expect(await InAppWebViewController.getDefaultUserAgent(), isNotEmpty);
    expect(await InAppWebViewController.handlesURLScheme('https'), isTrue);
    expect(
      await InAppWebViewController.handlesURLScheme('custom-scheme'),
      isFalse,
    );
    expect(await controller.getTitle(), 'Fixture title');
    expect(
      await controller.evaluateJavascript(
        source: "document.querySelector('h1').textContent",
      ),
      'Fixture Page',
    );
  });

  testWidgets('JavaScript dialog callbacks control responses', (
    WidgetTester tester,
  ) async {
    final Completer<String> alertMessage = Completer<String>();
    final InAppWebViewController controller = await _pumpWebView(
      tester,
      onJsAlert: (_, JsAlertRequest request) async {
        alertMessage.complete(request.message);
        return JsAlertResponse(handledByClient: true);
      },
      onJsConfirm: (_, __) async {
        return JsConfirmResponse(handledByClient: true);
      },
      onJsPrompt: (_, __) async {
        return JsPromptResponse(
          action: JsPromptResponseAction.CONFIRM,
          handledByClient: true,
          value: 'from callback',
        );
      },
    );

    await _loadFixture(controller);

    expect(
      await controller.evaluateJavascript(
        source: "alert('alert message'); 'alert complete';",
      ),
      'alert complete',
    );
    expect(
      await alertMessage.future.timeout(const Duration(seconds: 10)),
      'alert message',
    );
    expect(
      await controller.evaluateJavascript(source: "confirm('confirm?')"),
      isFalse,
    );
    expect(
      await controller.evaluateJavascript(source: "prompt('prompt?', 'base')"),
      'from callback',
    );
  });

  testWidgets('deleteAllCookies clears the cookie store', (
    WidgetTester tester,
  ) async {
    final StreamController<String> loadStops =
        StreamController<String>.broadcast();
    addTearDown(loadStops.close);

    final InAppWebViewController controller = await _pumpWebView(
      tester,
      initialUrl: firstUrl,
      onLoadStop: (_, WebUri? url) {
        if (url != null) {
          loadStops.add(url.toString());
        }
      },
    );
    await _waitForValue(loadStops.stream, firstUrl);

    final Object? cookieBefore = await controller.evaluateJavascript(
      source: '''
document.cookie = 'tizen_inappwebview=1; path=/';
document.cookie;
''',
    );
    expect(cookieBefore.toString(), contains('tizen_inappwebview=1'));

    expect(await CookieManager.instance().deleteAllCookies(), isTrue);

    final Future<String> reloaded = _waitForValue(loadStops.stream, firstUrl);
    await controller.reload();
    expect(await reloaded, firstUrl);
    final Object? cookieAfter = await controller.evaluateJavascript(
      source: 'document.cookie',
    );
    expect(cookieAfter.toString(), isNot(contains('tizen_inappwebview=1')));
  });
}

Future<InAppWebViewController> _pumpWebView(
  WidgetTester tester, {
  String initialUrl = 'about:blank',
  InAppWebViewSettings? initialSettings,
  void Function(InAppWebViewController, WebUri?)? onLoadStop,
  void Function(InAppWebViewController, int)? onProgressChanged,
  void Function(InAppWebViewController, ConsoleMessage)? onConsoleMessage,
  void Function(InAppWebViewController, WebUri?, bool?)? onUpdateVisitedHistory,
  Future<JsAlertResponse?> Function(InAppWebViewController, JsAlertRequest)?
  onJsAlert,
  Future<JsConfirmResponse?> Function(InAppWebViewController, JsConfirmRequest)?
  onJsConfirm,
  Future<JsPromptResponse?> Function(InAppWebViewController, JsPromptRequest)?
  onJsPrompt,
  Future<NavigationActionPolicy?> Function(
    InAppWebViewController,
    NavigationAction,
  )?
  shouldOverrideUrlLoading,
}) async {
  final Completer<InAppWebViewController> controllerCompleter =
      Completer<InAppWebViewController>();

  await tester.pumpWidget(
    MaterialApp(
      home: Scaffold(
        body: SizedBox.expand(
          child: InAppWebView(
            initialSettings: initialSettings,
            initialUrlRequest: URLRequest(url: WebUri(initialUrl)),
            onWebViewCreated: controllerCompleter.complete,
            onLoadStop: onLoadStop,
            onProgressChanged: onProgressChanged,
            onConsoleMessage: onConsoleMessage,
            onUpdateVisitedHistory: onUpdateVisitedHistory,
            onJsAlert: onJsAlert,
            onJsConfirm: onJsConfirm,
            onJsPrompt: onJsPrompt,
            shouldOverrideUrlLoading: shouldOverrideUrlLoading,
          ),
        ),
      ),
    ),
  );

  addTearDown(() async {
    await tester.pumpWidget(const SizedBox.shrink());
  });

  return controllerCompleter.future.timeout(const Duration(seconds: 10));
}

Future<void> _loadFixture(InAppWebViewController controller) async {
  await controller.loadData(data: _fixtureHtml);
  await _waitForFixtureDocument(
    controller,
  ).timeout(const Duration(seconds: 10));
}

Future<void> _waitForFixtureDocument(InAppWebViewController controller) async {
  Object? lastResult;
  final DateTime end = DateTime.now().add(const Duration(seconds: 10));

  while (DateTime.now().isBefore(end)) {
    try {
      final Object? title = await controller.evaluateJavascript(
        source: 'document.title',
      );
      final Object? heading = await controller.evaluateJavascript(
        source: "document.querySelector('h1')?.textContent",
      );
      if (title?.toString() == 'Fixture title' &&
          heading?.toString() == 'Fixture Page') {
        return;
      }
      lastResult = 'title=$title heading=$heading';
    } catch (error) {
      lastResult = error;
    }
    await Future<void>.delayed(const Duration(milliseconds: 200));
  }

  throw TimeoutException(
    'Fixture page did not become ready. Last result: $lastResult',
  );
}

Future<T> _waitForValue<T>(
  Stream<T> stream,
  T value, {
  Duration timeout = const Duration(seconds: 10),
}) {
  return stream.firstWhere((T event) => event == value).timeout(timeout);
}

String _htmlPage(String title) {
  return '''
<!doctype html>
<html>
  <head>
    <meta charset="utf-8">
    <title>$title</title>
  </head>
  <body>
    <h1>$title</h1>
    <a id="second-link" href="/second">Second</a>
    <a id="blocked-link" href="/blocked">Blocked</a>
    <script>console.log('$title ready');</script>
  </body>
</html>
''';
}
