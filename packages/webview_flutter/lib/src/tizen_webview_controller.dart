// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_tizen/widgets.dart';
import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

import 'tizen_webview.dart';

/// The channel name of [TizenNavigationDelegate].
const String kTizenNavigationDelegateChannelName =
    'plugins.flutter.io/tizen_webview_navigation_delegate_';

/// An implementation of [PlatformWebViewController] using the Tizen WebView API.
class TizenWebViewController extends PlatformWebViewController {
  /// Constructs a [TizenWebViewController].
  TizenWebViewController(super.params)
      : _webview = TizenWebView(),
        super.implementation();

  final TizenWebView _webview;
  late TizenNavigationDelegate _tizenNavigationDelegate;

  /// Called when [TizenView] is created.
  void onCreate(int viewId) {
    _webview.onCreate(viewId);
    if (_webview.hasNavigationDelegate) {
      _tizenNavigationDelegate.createNavigationDelegateChannel(viewId);
    }
  }

  @override
  Future<void> loadFile(String absoluteFilePath) {
    assert(absoluteFilePath != null);
    return _webview.loadFile(absoluteFilePath);
  }

  @override
  Future<void> loadFlutterAsset(String key) {
    assert(key.isNotEmpty);
    return _webview.loadFlutterAsset(key);
  }

  @override
  Future<void> loadHtmlString(
    String html, {
    String? baseUrl,
  }) {
    assert(html != null);
    return _webview.loadHtmlString(html, baseUrl: baseUrl);
  }

  @override
  Future<void> loadRequest(LoadRequestParams params) {
    if (!params.uri.hasScheme) {
      throw ArgumentError(
          'LoadRequestParams#uri is required to have a scheme.');
    }

    switch (params.method) {
      case LoadRequestMethod.get:
        if (params.headers.isNotEmpty) {
          return _webview.loadRequestWithParams(params);
        }
        return _webview.loadRequest(params.uri.toString());
      case LoadRequestMethod.post:
        if (params.headers.isNotEmpty || params.body != null) {
          return _webview.loadRequestWithParams(params);
        }
        return _webview.loadRequest(params.uri.toString());
    }

    // The enum comes from a different package, which could get a new value at
    // any time, so a fallback case is necessary. Since there is no reasonable
    // default behavior, throw to alert the client that they need an updated
    // version. This is deliberately outside the switch rather than a `default`
    // so that the linter will flag the switch as needing an update.
    // ignore: dead_code
    throw UnimplementedError(
        'This version of `TizenWebViewController` currently has no '
        'implementation for HTTP method ${params.method.serialize()} in '
        'loadRequest.');
  }

  @override
  Future<String?> currentUrl() => _webview.currentUrl();

  @override
  Future<bool> canGoBack() => _webview.canGoBack();

  @override
  Future<bool> canGoForward() => _webview.canGoForward();

  @override
  Future<void> goBack() => _webview.goBack();

  @override
  Future<void> goForward() => _webview.goForward();

  @override
  Future<void> reload() => _webview.reload();

  @override
  Future<void> clearCache() => _webview.clearCache();

  @override
  Future<void> clearLocalStorage() {
    throw UnimplementedError(
        'This version of `TizenWebViewController` currently has no '
        'implementation.');
  }

  @override
  Future<void> setJavaScriptMode(JavaScriptMode javaScriptMode) =>
      _webview.setJavaScriptMode(javaScriptMode.index);

  @override
  Future<String?> getTitle() => _webview.getTitle();

  @override
  Future<void> scrollTo(int x, int y) => _webview.scrollTo(x, y);

  @override
  Future<void> scrollBy(int x, int y) => _webview.scrollBy(x, y);

  @override
  Future<Offset> getScrollPosition() => _webview.getScrollPosition();

  @override
  Future<void> setBackgroundColor(Color color) =>
      _webview.setBackgroundColor(color);

  @override
  Future<void> setPlatformNavigationDelegate(
      covariant TizenNavigationDelegate handler) async {
    _tizenNavigationDelegate = handler;
    if (_webview.hasNavigationDelegate) {
      _tizenNavigationDelegate.createNavigationDelegateChannel(_webview.viewId);
    } else {
      _webview.hasNavigationDelegate = true;
    }
  }

  @override
  Future<void> addJavaScriptChannel(
          JavaScriptChannelParams javaScriptChannelParams) =>
      _webview.addJavaScriptChannel(javaScriptChannelParams);

  @override
  Future<void> runJavaScript(String javaScript) =>
      _webview.runJavaScript(javaScript);

  @override
  Future<Object> runJavaScriptReturningResult(String javaScript) =>
      _webview.runJavaScriptReturningResult(javaScript);

  @override
  Future<void> setUserAgent(String? userAgent) =>
      _webview.setUserAgent(userAgent);

  @override
  Future<void> setOnPlatformPermissionRequest(
    void Function(
      PlatformWebViewPermissionRequest request,
    ) onPermissionRequest,
  ) async {
    throw UnimplementedError(
        'This version of `TizenWebViewController` currently has no '
        'implementation.');
  }

  @override
  Future<String?> getUserAgent() {
    return _webview.getUserAgent();
  }
}

/// An implementation of [PlatformWebViewWidget] with the Tizen WebView API.
class TizenWebViewWidget extends PlatformWebViewWidget {
  /// Constructs a [TizenWebViewWidget].
  TizenWebViewWidget(super.params) : super.implementation();

  @override
  Widget build(BuildContext context) {
    return TizenView(
      key: params.key,
      viewType: 'plugins.flutter.io/webview',
      onPlatformViewCreated: (int id) {
        final TizenWebViewController controller =
            params.controller as TizenWebViewController;
        controller.onCreate(id);
      },
      layoutDirection: params.layoutDirection,
      gestureRecognizers: params.gestureRecognizers,
    );
  }
}

/// Error returned in `WebView.onWebResourceError` when a web resource loading error has occurred.
@immutable
class TizenWebResourceError extends WebResourceError {
  /// Creates a new [TizenWebResourceError].
  TizenWebResourceError._({
    required super.errorCode,
    required super.description,
    super.isForMainFrame,
    this.failingUrl,
  }) : super(errorType: _errorCodeToErrorType(errorCode));

  /// Unknown error.
  static const int unknown = 0;

  /// Failed to file I/O.
  static const int failedFileIO = 3;

  /// Cannot connect to Network.
  static const int cantConnect = 4;

  /// Fail to look up host from DNS.
  static const int cantHostLookup = 5;

  /// Fail to SSL/TLS handshake.
  static const int failedSslHandshake = 6;

  /// Connection timeout.
  static const int requestTimeout = 8;

  /// Too many redirects.
  static const int tooManyRedirect = 9;

  /// Too many requests during this load.
  static const int tooManyRequests = 10;

  /// Malformed url.
  static const int badUrl = 11;

  /// Unsupported scheme
  static const int unsupportedScheme = 12;

  /// User authentication failed on server.
  static const int authenticationFailed = 13;

  /// Gets the URL for which the failing resource request was made.
  final String? failingUrl;

  static WebResourceErrorType? _errorCodeToErrorType(int errorCode) {
    switch (errorCode) {
      case unknown:
        return WebResourceErrorType.unknown;
      case failedFileIO:
        return WebResourceErrorType.file;
      case cantConnect:
        return WebResourceErrorType.connect;
      case cantHostLookup:
        return WebResourceErrorType.hostLookup;
      case failedSslHandshake:
        return WebResourceErrorType.failedSslHandshake;
      case requestTimeout:
        return WebResourceErrorType.timeout;
      case tooManyRedirect:
        return WebResourceErrorType.redirectLoop;
      case tooManyRequests:
        return WebResourceErrorType.tooManyRequests;
      case badUrl:
        return WebResourceErrorType.badUrl;
      case unsupportedScheme:
        return WebResourceErrorType.unsupportedScheme;
      case authenticationFailed:
        return WebResourceErrorType.authentication;
    }

    throw ArgumentError(
      'Could not find a WebResourceErrorType for errorCode: $errorCode',
    );
  }
}

/// A place to register callback methods responsible to handle navigation events
/// triggered by the [TizenWebView].
class TizenNavigationDelegate extends PlatformNavigationDelegate {
  /// Creates a new [TizenNavigationDelegate].
  TizenNavigationDelegate(super.params) : super.implementation();

  late final MethodChannel _navigationDelegateChannel;
  PageEventCallback? _onPageFinished;
  PageEventCallback? _onPageStarted;
  ProgressCallback? _onProgress;
  WebResourceErrorCallback? _onWebResourceError;
  NavigationRequestCallback? _onNavigationRequest;
  UrlChangeCallback? _onUrlChange;

  /// Called when [TizenView] is created.
  void createNavigationDelegateChannel(int viewId) {
    _navigationDelegateChannel =
        MethodChannel(kTizenNavigationDelegateChannelName + viewId.toString());
    _navigationDelegateChannel.setMethodCallHandler((MethodCall call) async {
      final Map<String, Object?> arguments =
          (call.arguments as Map<Object?, Object?>).cast<String, Object?>();
      switch (call.method) {
        case 'navigationRequest':
          return _handleNavigation(arguments['url']! as String,
              isForMainFrame: arguments['isForMainFrame']! as bool);
        case 'onPageFinished':
          if (_onPageFinished != null) {
            _onPageFinished!(arguments['url']! as String);
          }
          return null;
        case 'onProgress':
          if (_onProgress != null) {
            _onProgress!(arguments['progress']! as int);
          }
          return null;
        case 'onPageStarted':
          if (_onPageStarted != null) {
            _onPageStarted!(arguments['url']! as String);
          }
          return null;
        case 'onWebResourceError':
          if (_onWebResourceError != null) {
            _onWebResourceError!(TizenWebResourceError._(
              errorCode: arguments['errorCode']! as int,
              description: arguments['description']! as String,
              failingUrl: arguments['failingUrl']! as String,
              isForMainFrame: true,
            ));
          }
          return null;
        case 'onUrlChange':
          if (_onUrlChange != null) {
            _onUrlChange!(UrlChange(url: arguments['url']! as String));
          }
          return null;
      }

      throw MissingPluginException(
        '${call.method} was invoked but has no handler',
      );
    });
  }

  Future<bool> _handleNavigation(
    String url, {
    required bool isForMainFrame,
  }) async {
    final NavigationRequestCallback? onNavigationRequest = _onNavigationRequest;

    if (onNavigationRequest == null) {
      return true;
    }

    final FutureOr<NavigationDecision> returnValue =
        onNavigationRequest(NavigationRequest(
      url: url,
      isMainFrame: isForMainFrame,
    ));

    if (returnValue is NavigationDecision &&
        returnValue == NavigationDecision.navigate) {
      return true;
    } else if (returnValue is Future<NavigationDecision>) {
      return returnValue.then((NavigationDecision shouldLoadUrl) {
        if (shouldLoadUrl == NavigationDecision.navigate) {
          return true;
        }
        return false;
      });
    }
    return false;
  }

  @override
  Future<void> setOnNavigationRequest(
    NavigationRequestCallback onNavigationRequest,
  ) async {
    _onNavigationRequest = onNavigationRequest;
  }

  @override
  Future<void> setOnPageStarted(
    PageEventCallback onPageStarted,
  ) async {
    _onPageStarted = onPageStarted;
  }

  @override
  Future<void> setOnPageFinished(
    PageEventCallback onPageFinished,
  ) async {
    _onPageFinished = onPageFinished;
  }

  @override
  Future<void> setOnProgress(
    ProgressCallback onProgress,
  ) async {
    _onProgress = onProgress;
  }

  @override
  Future<void> setOnWebResourceError(
    WebResourceErrorCallback onWebResourceError,
  ) async {
    _onWebResourceError = onWebResourceError;
  }

  @override
  Future<void> setOnUrlChange(UrlChangeCallback onUrlChange) async {
    _onUrlChange = onUrlChange;
  }
}
