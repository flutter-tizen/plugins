// Copyright 2023 Lorenzo Pichilli. All rights reserved.
// Licensed under the Apache License, Version 2.0.
// Imported from https://pub.dev/packages/flutter_inappwebview.

import 'dart:collection';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
// TIZENFIX: Detect Tizen so unsupported features can be skipped at build time.
import 'package:flutter_tizen/flutter_tizen.dart' as flutter_tizen;
import 'package:url_launcher/url_launcher.dart';

import 'main.dart';

class InAppWebViewExampleScreen extends StatefulWidget {
  const InAppWebViewExampleScreen({super.key});

  @override
  State<InAppWebViewExampleScreen> createState() =>
      _InAppWebViewExampleScreenState();
}

class _InAppWebViewExampleScreenState extends State<InAppWebViewExampleScreen> {
  final GlobalKey webViewKey = GlobalKey();

  InAppWebViewController? webViewController;
  InAppWebViewSettings settings = InAppWebViewSettings(
    isInspectable: kDebugMode,
    mediaPlaybackRequiresUserGesture: false,
    allowsInlineMediaPlayback: true,
    iframeAllow: "camera; microphone",
    iframeAllowFullscreen: true,
  );

  PullToRefreshController? pullToRefreshController;

  late ContextMenu contextMenu;
  String url = "";
  double progress = 0;
  final urlController = TextEditingController();

  @override
  void initState() {
    super.initState();

    contextMenu = ContextMenu(
      menuItems: [
        ContextMenuItem(
          id: 1,
          title: "Special",
          action: () async {
            debugPrint("Menu item Special clicked!");
            debugPrint(await webViewController?.getSelectedText());
            await webViewController?.clearFocus();
          },
        ),
      ],
      settings: ContextMenuSettings(hideDefaultSystemContextMenuItems: false),
      onCreateContextMenu: (hitTestResult) async {
        debugPrint("onCreateContextMenu");
        debugPrint(hitTestResult.extra);
        debugPrint(await webViewController?.getSelectedText());
      },
      onHideContextMenu: () {
        debugPrint("onHideContextMenu");
      },
      onContextMenuActionItemClicked: (contextMenuItemClicked) async {
        var id = contextMenuItemClicked.id;
        debugPrint(
          "onContextMenuActionItemClicked: $id ${contextMenuItemClicked.title}",
        );
      },
    );

    pullToRefreshController =
        kIsWeb ||
            ![
              TargetPlatform.iOS,
              TargetPlatform.android,
            ].contains(defaultTargetPlatform)
        ? null
        : PullToRefreshController(
            settings: PullToRefreshSettings(color: Colors.blue),
            onRefresh: () async {
              if (defaultTargetPlatform == TargetPlatform.android) {
                webViewController?.reload();
              } else if (defaultTargetPlatform == TargetPlatform.iOS) {
                webViewController?.loadUrl(
                  urlRequest: URLRequest(
                    url: await webViewController?.getUrl(),
                  ),
                );
              }
            },
          );
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text("InAppWebView")),
      backgroundColor: Colors
          .transparent, // for tizen - entire app transparent for WebView video buffer
      drawer: myDrawer(context: context),
      body: SafeArea(
        child: Column(
          children: <Widget>[
            ColoredBox(
              color: Theme.of(context).scaffoldBackgroundColor, // for tizen
              child: TextField(
                decoration: const InputDecoration(
                  prefixIcon: Icon(Icons.search),
                ),
                controller: urlController,
                keyboardType: TextInputType.text,
                onSubmitted: (value) {
                  var url = WebUri(value);
                  if (url.scheme.isEmpty) {
                    url = WebUri(
                      (!kIsWeb
                              ? "https://www.google.com/search?q="
                              : "https://www.bing.com/search?q=") +
                          value,
                    );
                  }
                  webViewController?.loadUrl(urlRequest: URLRequest(url: url));
                },
              ),
            ),
            Expanded(
              child: Stack(
                children: [
                  InAppWebView(
                    key: webViewKey,
                    webViewEnvironment: webViewEnvironment,
                    initialUrlRequest: URLRequest(
                      url: WebUri('https://flutter.dev'),
                    ),
                    // initialUrlRequest:
                    // URLRequest(url: WebUri(Uri.base.toString().replaceFirst("/#/", "/") + 'page.html')),
                    // initialFile: "assets/index.html",
                    initialUserScripts: UnmodifiableListView<UserScript>([]),
                    initialSettings: settings,
                    // TIZENFIX: ContextMenu is not implemented on Tizen and
                    // the widget would throw on construction. Pass null on Tizen.
                    contextMenu: flutter_tizen.isTizen ? null : contextMenu,
                    pullToRefreshController: pullToRefreshController,
                    onWebViewCreated: (controller) async {
                      webViewController = controller;
                    },
                    onLoadStart: (controller, url) async {
                      setState(() {
                        this.url = url.toString();
                        urlController.text = this.url;
                      });
                    },
                    // TIZENFIX: onPermissionRequest is not implemented on Tizen
                    // and is rejected by the widget. Skip the callback there.
                    onPermissionRequest: flutter_tizen.isTizen
                        ? null
                        : (controller, request) async {
                            return PermissionResponse(
                              resources: request.resources,
                              action: PermissionResponseAction.GRANT,
                            );
                          },
                    shouldOverrideUrlLoading:
                        (controller, navigationAction) async {
                          var uri = navigationAction.request.url!;

                          if (![
                            "http",
                            "https",
                            "file",
                            "chrome",
                            "data",
                            "javascript",
                            "about",
                          ].contains(uri.scheme)) {
                            if (await canLaunchUrl(uri)) {
                              // Launch the App
                              await launchUrl(uri);
                              // and cancel the request
                              return NavigationActionPolicy.CANCEL;
                            }
                          }

                          return NavigationActionPolicy.ALLOW;
                        },
                    onLoadStop: (controller, url) async {
                      pullToRefreshController?.endRefreshing();
                      setState(() {
                        this.url = url.toString();
                        urlController.text = this.url;
                      });
                    },
                    onReceivedError: (controller, request, error) {
                      pullToRefreshController?.endRefreshing();
                    },
                    onProgressChanged: (controller, progress) {
                      if (progress == 100) {
                        pullToRefreshController?.endRefreshing();
                      }
                      setState(() {
                        this.progress = progress / 100;
                        urlController.text = url;
                      });
                    },
                    onUpdateVisitedHistory: (controller, url, isReload) {
                      setState(() {
                        this.url = url.toString();
                        urlController.text = this.url;
                      });
                    },
                    onConsoleMessage: (controller, consoleMessage) {
                      debugPrint(consoleMessage.toString());
                    },
                  ),
                  progress < 1.0
                      ? LinearProgressIndicator(value: progress)
                      : Container(),
                ],
              ),
            ),
            ColoredBox(
              color: Theme.of(context).scaffoldBackgroundColor, // for tizen
              child: ButtonBar(
                alignment: MainAxisAlignment.center,
                children: <Widget>[
                  ElevatedButton(
                    onPressed: () {
                      webViewController?.goBack();
                    },
                    child: const Icon(Icons.arrow_back),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      webViewController?.goForward();
                    },
                    child: const Icon(Icons.arrow_forward),
                  ),
                  ElevatedButton(
                    onPressed: () {
                      webViewController?.reload();
                    },
                    child: const Icon(Icons.refresh),
                  ),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
