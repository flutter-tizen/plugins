// Copyright 2023 Lorenzo Pichilli. All rights reserved.
// Licensed under the Apache License, Version 2.0.
// Imported from https://pub.dev/packages/flutter_inappwebview.

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';

import 'main.dart';

class HeadlessInAppWebViewExampleScreen extends StatefulWidget {
  const HeadlessInAppWebViewExampleScreen({super.key});

  @override
  State<HeadlessInAppWebViewExampleScreen> createState() =>
      _HeadlessInAppWebViewExampleScreenState();
}

class _HeadlessInAppWebViewExampleScreenState
    extends State<HeadlessInAppWebViewExampleScreen> {
  HeadlessInAppWebView? headlessWebView;
  String url = "";

  @override
  void initState() {
    super.initState();

    var url = !kIsWeb
        ? WebUri("https://flutter.dev")
        : WebUri("http://localhost:${Uri.base.port}/page.html");

    headlessWebView = HeadlessInAppWebView(
      webViewEnvironment: webViewEnvironment,
      initialUrlRequest: URLRequest(url: url),
      initialSettings: InAppWebViewSettings(isInspectable: kDebugMode),
      onWebViewCreated: (controller) {
        debugPrint('HeadlessInAppWebView created!');
      },
      onConsoleMessage: (controller, consoleMessage) {
        debugPrint("CONSOLE MESSAGE: ${consoleMessage.message}");
      },
      onLoadStart: (controller, url) async {
        setState(() {
          this.url = url.toString();
        });
      },
      onLoadStop: (controller, url) async {
        setState(() {
          this.url = url.toString();
        });
      },
      onUpdateVisitedHistory: (controller, url, isReload) {
        setState(() {
          this.url = url.toString();
        });
      },
    );
  }

  @override
  void dispose() {
    super.dispose();
    headlessWebView?.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("HeadlessInAppWebView")),
      drawer: myDrawer(context: context),
      body: SafeArea(
        child: Column(
          children: <Widget>[
            Container(
              padding: const EdgeInsets.all(20.0),
              child: Text(
                "CURRENT URL\n${(url.length > 50) ? '${url.substring(0, 50)}...' : url}",
              ),
            ),
            Center(
              child: ElevatedButton(
                onPressed: () async {
                  await headlessWebView?.dispose();
                  await headlessWebView?.run();
                },
                child: const Text("Run HeadlessInAppWebView"),
              ),
            ),
            Container(height: 10),
            Center(
              child: ElevatedButton(
                onPressed: () async {
                  if (headlessWebView?.isRunning() ?? false) {
                    await headlessWebView?.webViewController
                        ?.evaluateJavascript(
                          source: """console.log('Here is the message!');""",
                        );
                  } else {
                    ScaffoldMessenger.of(context).showSnackBar(
                      const SnackBar(
                        content: Text(
                          'HeadlessInAppWebView is not running. Click on "Run HeadlessInAppWebView"!',
                        ),
                      ),
                    );
                  }
                },
                child: const Text("Send console.log message"),
              ),
            ),
            Container(height: 10),
            Center(
              child: ElevatedButton(
                onPressed: () {
                  headlessWebView?.dispose();
                  setState(() {
                    url = "";
                  });
                },
                child: const Text("Dispose HeadlessInAppWebView"),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
