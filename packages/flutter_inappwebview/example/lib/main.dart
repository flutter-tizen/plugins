// Copyright 2023 Lorenzo Pichilli. All rights reserved.
// Licensed under the Apache License, Version 2.0.
// Imported from https://pub.dev/packages/flutter_inappwebview.

import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter_inappwebview/flutter_inappwebview.dart';
// TIZENFIX: Detect Tizen so the drawer/routes can hide unsupported screens.
import 'package:flutter_tizen/flutter_tizen.dart' as flutter_tizen;

import 'package:flutter_inappwebview_tizen_example/chrome_safari_browser_example.screen.dart';
import 'package:flutter_inappwebview_tizen_example/headless_in_app_webview.screen.dart';
import 'package:flutter_inappwebview_tizen_example/in_app_webiew_example.screen.dart';
import 'package:flutter_inappwebview_tizen_example/in_app_browser_example.screen.dart';
import 'package:flutter_inappwebview_tizen_example/web_authentication_session_example.screen.dart';
import 'package:pointer_interceptor/pointer_interceptor.dart';

// import 'package:path_provider/path_provider.dart';
// import 'package:permission_handler/permission_handler.dart';

// TIZENFIX: InAppLocalhostServer is not implemented on Tizen. The reference is
// kept (matching upstream) but it is null so no native call is made.
final localhostServer = !flutter_tizen.isTizen
    ? InAppLocalhostServer(documentRoot: 'assets')
    : null;
WebViewEnvironment? webViewEnvironment;

Future main() async {
  WidgetsFlutterBinding.ensureInitialized();
  // await Permission.camera.request();
  // await Permission.microphone.request();
  // await Permission.storage.request();

  if (!kIsWeb && defaultTargetPlatform == TargetPlatform.windows) {
    final availableVersion = await WebViewEnvironment.getAvailableVersion();
    assert(
      availableVersion != null,
      'Failed to find an installed WebView2 runtime or non-stable Microsoft Edge installation.',
    );

    webViewEnvironment = await WebViewEnvironment.create(
      settings: WebViewEnvironmentSettings(userDataFolder: 'custom_path'),
    );
  }

  if (!kIsWeb && defaultTargetPlatform == TargetPlatform.android) {
    await InAppWebViewController.setWebContentsDebuggingEnabled(kDebugMode);
  }

  runApp(MyApp());
}

PointerInterceptor myDrawer({required BuildContext context}) {
  var children = [
    ListTile(
      title: Text('InAppWebView'),
      onTap: () {
        Navigator.pushReplacementNamed(context, '/');
      },
    ),
    ListTile(
      title: Text('InAppBrowser'),
      onTap: () {
        Navigator.pushReplacementNamed(context, '/InAppBrowser');
      },
    ),
    ListTile(
      title: Text('ChromeSafariBrowser'),
      onTap: () {
        Navigator.pushReplacementNamed(context, '/ChromeSafariBrowser');
      },
    ),
    ListTile(
      title: Text('WebAuthenticationSession'),
      onTap: () {
        Navigator.pushReplacementNamed(context, '/WebAuthenticationSession');
      },
    ),
    ListTile(
      title: Text('HeadlessInAppWebView'),
      onTap: () {
        Navigator.pushReplacementNamed(context, '/HeadlessInAppWebView');
      },
    ),
  ];
  if (kIsWeb) {
    children = [
      ListTile(
        title: Text('InAppWebView'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/');
        },
      ),
    ];
  } else if (defaultTargetPlatform == TargetPlatform.macOS) {
    children = [
      ListTile(
        title: Text('InAppWebView'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/');
        },
      ),
      ListTile(
        title: Text('InAppBrowser'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/InAppBrowser');
        },
      ),
      ListTile(
        title: Text('WebAuthenticationSession'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/WebAuthenticationSession');
        },
      ),
      ListTile(
        title: Text('HeadlessInAppWebView'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/HeadlessInAppWebView');
        },
      ),
    ];
  } else if (defaultTargetPlatform == TargetPlatform.windows ||
      defaultTargetPlatform == TargetPlatform.linux) {
    children = [
      ListTile(
        title: Text('InAppWebView'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/');
        },
      ),
      ListTile(
        title: Text('InAppBrowser'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/InAppBrowser');
        },
      ),
      ListTile(
        title: Text('HeadlessInAppWebView'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/HeadlessInAppWebView');
        },
      ),
    ];
  }
  // TIZENFIX: Tizen reports as TargetPlatform.linux but only InAppWebView is
  // supported. Replace the drawer entries with a Tizen-only list so that
  // navigating to one of the unsupported screens cannot crash the example.
  if (flutter_tizen.isTizen) {
    children = [
      ListTile(
        title: Text('InAppWebView'),
        onTap: () {
          Navigator.pushReplacementNamed(context, '/');
        },
      ),
    ];
  }
  return PointerInterceptor(
    child: Drawer(
      child: ListView(
        padding: EdgeInsets.zero,
        children: <Widget>[
          DrawerHeader(
            child: Text('flutter_inappwebview example'),
            decoration: BoxDecoration(color: Colors.blue),
          ),
          ...children,
        ],
      ),
    ),
  );
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  void initState() {
    super.initState();
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (kIsWeb) {
      return MaterialApp(
        initialRoute: '/',
        routes: {'/': (context) => InAppWebViewExampleScreen()},
      );
    }
    // TIZENFIX: Only register the InAppWebView route on Tizen. The other
    // routes from the upstream sample point at unsupported features that
    // would crash on instantiation if the user navigated to them.
    if (flutter_tizen.isTizen) {
      return MaterialApp(
        initialRoute: '/',
        routes: {'/': (context) => InAppWebViewExampleScreen()},
      );
    }
    if (defaultTargetPlatform == TargetPlatform.macOS) {
      return MaterialApp(
        initialRoute: '/',
        routes: {
          '/': (context) => InAppWebViewExampleScreen(),
          '/InAppBrowser': (context) => InAppBrowserExampleScreen(),
          '/HeadlessInAppWebView': (context) =>
              HeadlessInAppWebViewExampleScreen(),
          '/WebAuthenticationSession': (context) =>
              WebAuthenticationSessionExampleScreen(),
        },
      );
    } else if (defaultTargetPlatform == TargetPlatform.windows ||
        defaultTargetPlatform == TargetPlatform.linux) {
      return MaterialApp(
        initialRoute: '/',
        routes: {
          '/': (context) => InAppWebViewExampleScreen(),
          '/InAppBrowser': (context) => InAppBrowserExampleScreen(),
          '/HeadlessInAppWebView': (context) =>
              HeadlessInAppWebViewExampleScreen(),
        },
      );
    }
    return MaterialApp(
      initialRoute: '/',
      routes: {
        '/': (context) => InAppWebViewExampleScreen(),
        '/InAppBrowser': (context) => InAppBrowserExampleScreen(),
        '/ChromeSafariBrowser': (context) => ChromeSafariBrowserExampleScreen(),
        '/HeadlessInAppWebView': (context) =>
            HeadlessInAppWebViewExampleScreen(),
        '/WebAuthenticationSession': (context) =>
            WebAuthenticationSessionExampleScreen(),
      },
    );
  }
}
