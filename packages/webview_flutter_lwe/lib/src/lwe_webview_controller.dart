// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_tizen/widgets.dart';
import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

import 'lwe_webview.dart';

/// The channel name of [LweNavigationDelegate].
const String kLweNavigationDelegateChannelName =
    'plugins.flutter.io/lwe_webview_navigation_delegate_';

/// An implementation of [PlatformWebViewController] using the Lightweight Web Engine.
class LweWebViewController extends PlatformWebViewController {
  /// Constructs a [LweWebViewController].
  LweWebViewController(super.params)
    : _webview = LweWebView(),
      super.implementation();

  final LweWebView _webview;
  late LweNavigationDelegate _lweNavigationDelegate;

  /// Called when [TizenView] is created.
  void onCreate(int viewId) {
    if (_webview.hasNavigationDelegate) {
      _lweNavigationDelegate.onCreate(viewId);
    }
    _webview.onCreate(viewId);
  }

  @override
  Future<void> loadFile(String absoluteFilePath) {
    assert(absoluteFilePath.isNotEmpty);
    return _webview.loadFile(absoluteFilePath);
  }

  @override
  Future<void> loadFlutterAsset(String key) {
    assert(key.isNotEmpty);
    return _webview.loadFlutterAsset(key);
  }

  @override
  Future<void> loadHtmlString(String html, {String? baseUrl}) {
    assert(html.isNotEmpty);
    return _webview.loadHtmlString(html, baseUrl: baseUrl);
  }

  @override
  Future<void> loadRequest(LoadRequestParams params) {
    if (!params.uri.hasScheme) {
      throw ArgumentError(
        'LoadRequestParams#uri is required to have a scheme.',
      );
    }

    switch (params.method) {
      case LoadRequestMethod.get:
        return _webview.loadRequest(params.uri.toString());
      case LoadRequestMethod.post:
        break;
    }
    // The enum comes from a different package, which could get a new value at
    // any time, so a fallback case is necessary. Since there is no reasonable
    // default behavior, throw to alert the client that they need an updated
    // version. This is deliberately outside the switch rather than a `default`
    // so that the linter will flag the switch as needing an update.
    // ignore: dead_code
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation for HTTP method ${params.method.serialize()} in '
      'loadRequest.',
    );
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
      'This version of `LweWebViewController` currently has no '
      'implementation.',
    );
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
  Future<void> enableZoom(bool enabled) {
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation of `enableZoom`.',
    );
  }

  @override
  Future<void> setBackgroundColor(Color color) =>
      _webview.setBackgroundColor(color);

  @override
  Future<void> setPlatformNavigationDelegate(
    covariant LweNavigationDelegate handler,
  ) async {
    _lweNavigationDelegate = handler;
    _webview.hasNavigationDelegate = true;
  }

  @override
  Future<void> addJavaScriptChannel(
    JavaScriptChannelParams javaScriptChannelParams,
  ) => _webview.addJavaScriptChannel(javaScriptChannelParams);

  @override
  Future<void> removeJavaScriptChannel(String javaScriptChannelName) =>
      _webview.removeJavaScriptChannel(javaScriptChannelName);

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
  Future<void> setOnScrollPositionChange(
    void Function(ScrollPositionChange scrollPositionChange)?
    onScrollPositionChange,
  ) async {
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation of `setOnScrollPositionChange`.',
    );
  }

  @override
  Future<String?> getUserAgent() => _webview.getUserAgent();

  @override
  Future<void> setOnPlatformPermissionRequest(
    void Function(PlatformWebViewPermissionRequest request) onPermissionRequest,
  ) async {
    // The current version of LWE does not provide any functionality related to a 'permission request'.
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation of `setOnPlatformPermissionRequest`.',
    );
  }

  @override
  Future<void> setOnConsoleMessage(
    void Function(JavaScriptConsoleMessage consoleMessage) onConsoleMessage,
  ) async {
    // The current version of LWE does not provide any functionality related to a 'console message'.
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation of `setOnConsoleMessage`.',
    );
  }

  @override
  Future<void> setOnJavaScriptAlertDialog(
    Future<void> Function(JavaScriptAlertDialogRequest request)
    onJavaScriptAlertDialog,
  ) async {
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation of `setOnJavaScriptAlertDialog`.',
    );
  }

  @override
  Future<void> setOnJavaScriptConfirmDialog(
    Future<bool> Function(JavaScriptConfirmDialogRequest request)
    onJavaScriptConfirmDialog,
  ) async {
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation of `setOnJavaScriptConfirmDialog`.',
    );
  }

  @override
  Future<void> setOnJavaScriptTextInputDialog(
    Future<String> Function(JavaScriptTextInputDialogRequest request)
    onJavaScriptTextInputDialog,
  ) async {
    throw UnimplementedError(
      'This version of `LweWebViewController` currently has no '
      'implementation of `setOnJavaScriptTextInputDialog`.',
    );
  }
}

/// An implementation of [PlatformWebViewWidget] with the Lightweight Web Engine.
class LweWebViewWidget extends PlatformWebViewWidget {
  /// Constructs a [LweWebViewWidget].
  LweWebViewWidget(super.params) : super.implementation();

  @override
  Widget build(BuildContext context) {
    return TizenView(
      key: params.key,
      viewType: 'plugins.flutter.io/webview',
      onPlatformViewCreated: (int id) {
        final LweWebViewController controller =
            params.controller as LweWebViewController;
        controller.onCreate(id);
      },
      layoutDirection: params.layoutDirection,
      gestureRecognizers: params.gestureRecognizers,
    );
  }
}

/// Error returned in `WebView.onWebResourceError` when a web resource loading error has occurred.
@immutable
class LweWebResourceError extends WebResourceError {
  /// Creates a new [LweWebResourceError].
  LweWebResourceError._({
    required super.errorCode,
    required super.description,
    super.isForMainFrame,
    this.failingUrl,
  }) : super(errorType: _errorCodeToErrorType(errorCode));

  /// Unknown error.
  static const int unknown = 1;

  /// Server or proxy hostname lookup failed.
  static const int hostLookup = 2;

  /// Unsupported authentication scheme (not basic or digest).
  static const int unsupportedAuthScheme = 3;

  /// User authentication failed on server.
  static const int authentication = 4;

  /// User authentication failed on proxy.
  static const int proxyAuthentication = 5;

  /// Failed to connect to the server.
  static const int connect = 6;

  /// Failed to read or write to the server.
  static const int io = 7;

  /// Connection timed out.
  static const int timeout = 8;

  /// Too many redirects.
  static const int redirectLoop = 9;

  /// Unsupported URI scheme.
  static const int unsupportedScheme = 10;

  /// Failed to perform SSL handshake.
  static const int failedSSLHandshake = 11;

  /// Malformed URL.
  static const int badURL = 12;

  /// Generic file error.
  static const int file = 13;

  /// File not found.
  static const int fileNotFound = 14;

  /// Too many requests during this load.
  static const int tooManyRequests = 15;

  /// Gets the URL for which the failing resource request was made.
  final String? failingUrl;

  static WebResourceErrorType? _errorCodeToErrorType(int errorCode) {
    switch (errorCode) {
      case unknown:
        return WebResourceErrorType.unknown;
      case hostLookup:
        return WebResourceErrorType.hostLookup;
      case unsupportedAuthScheme:
        return WebResourceErrorType.unsupportedAuthScheme;
      case authentication:
        return WebResourceErrorType.authentication;
      case proxyAuthentication:
        return WebResourceErrorType.proxyAuthentication;
      case connect:
        return WebResourceErrorType.connect;
      case io:
        return WebResourceErrorType.io;
      case timeout:
        return WebResourceErrorType.timeout;
      case redirectLoop:
        return WebResourceErrorType.redirectLoop;
      case unsupportedScheme:
        return WebResourceErrorType.unsupportedScheme;
      case failedSSLHandshake:
        return WebResourceErrorType.failedSslHandshake;
      case badURL:
        return WebResourceErrorType.badUrl;
      case file:
        return WebResourceErrorType.file;
      case fileNotFound:
        return WebResourceErrorType.fileNotFound;
      case tooManyRequests:
        return WebResourceErrorType.tooManyRequests;
    }

    throw ArgumentError(
      'Could not find a WebResourceErrorType for errorCode: $errorCode',
    );
  }
}

/// A place to register callback methods responsible to handle navigation events
/// triggered by the [LweWebView].
class LweNavigationDelegate extends PlatformNavigationDelegate {
  /// Creates a new [LweNavigationDelegate].
  LweNavigationDelegate(super.params) : super.implementation();

  late final MethodChannel _navigationDelegateChannel;
  PageEventCallback? _onPageFinished;
  PageEventCallback? _onPageStarted;
  ProgressCallback? _onProgress;
  WebResourceErrorCallback? _onWebResourceError;
  NavigationRequestCallback? _onNavigationRequest;

  /// Called when [TizenView] is created.
  void onCreate(int viewId) {
    _navigationDelegateChannel = MethodChannel(
      kLweNavigationDelegateChannelName + viewId.toString(),
    );
    _navigationDelegateChannel.setMethodCallHandler((MethodCall call) async {
      final Map<String, Object?> arguments =
          (call.arguments as Map<Object?, Object?>).cast<String, Object?>();

      switch (call.method) {
        case 'navigationRequest':
          return _handleNavigation(
            arguments['url']! as String,
            isForMainFrame: arguments['isForMainFrame']! as bool,
          );
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
            _onWebResourceError!(
              LweWebResourceError._(
                errorCode: arguments['errorCode']! as int,
                description: arguments['description']! as String,
                failingUrl: arguments['failingUrl']! as String,
                isForMainFrame: true,
              ),
            );
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

    final FutureOr<NavigationDecision> returnValue = onNavigationRequest(
      NavigationRequest(url: url, isMainFrame: isForMainFrame),
    );

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
  Future<void> setOnPageStarted(PageEventCallback onPageStarted) async {
    _onPageStarted = onPageStarted;
  }

  @override
  Future<void> setOnPageFinished(PageEventCallback onPageFinished) async {
    _onPageFinished = onPageFinished;
  }

  @override
  Future<void> setOnHttpError(HttpResponseErrorCallback onHttpError) async {
    throw UnimplementedError(
      'This version of `LweNavigationDelegate` currently has no '
      'implementation for `setOnHttpError`',
    );
  }

  @override
  Future<void> setOnProgress(ProgressCallback onProgress) async {
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
    throw UnimplementedError(
      'This version of `LweNavigationDelegate` currently has no '
      'implementation for `setOnUrlChange`',
    );
  }

  @override
  Future<void> setOnHttpAuthRequest(
    HttpAuthRequestCallback onHttpAuthRequest,
  ) async {
    throw UnimplementedError(
      'This version of `LweNavigationDelegate` currently has no '
      'implementation for `setOnHttpAuthRequest`',
    );
  }
}
