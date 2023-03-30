// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter_tizen/widgets.dart';
import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

/// The channel name of [TizenWebView].
const String kTizenWebViewChannelName = 'plugins.flutter.io/tizen_webview_';

/// A Tizen webview that displays web pages.
class TizenWebView {
  /// Whether the [TizenNavigationDelegate] is set by the [PlatformWebViewController].
  ///
  /// Defaults to false.
  bool hasNavigationDelegate = false;

  late final MethodChannel _tizenWebViewChannel;
  bool _isCreated = false;

  final Map<String, JavaScriptChannelParams> _javaScriptChannelParams =
      <String, JavaScriptChannelParams>{};
  final Map<String, dynamic> _pendingMethodCalls = <String, dynamic>{};

  Future<bool?> _onMethodCall(MethodCall call) async {
    switch (call.method) {
      case 'javaScriptChannelMessage':
        final Map<String, Object?> arguments =
            (call.arguments as Map<Object?, Object?>).cast<String, Object?>();
        final String channel = arguments['channel']! as String;
        final String message = arguments['message']! as String;
        if (_javaScriptChannelParams.containsKey(channel)) {
          _javaScriptChannelParams[channel]
              ?.onMessageReceived(JavaScriptMessage(message: message));
        }

        return true;
    }

    throw MissingPluginException(
      '${call.method} was invoked but has no handler',
    );
  }

  Future<T?> _invokeChannelMethod<T>(String method, [dynamic arguments]) async {
    if (!_isCreated) {
      _pendingMethodCalls[method] = arguments;
      return null;
    }

    return _tizenWebViewChannel.invokeMethod<T>(method, arguments);
  }

  /// Called when [TizenView] is created. Invokes the requested method call before [TizenWebView] is created.
  void onCreate(int viewId) {
    _isCreated = true;
    _tizenWebViewChannel =
        MethodChannel(kTizenWebViewChannelName + viewId.toString());
    _tizenWebViewChannel.setMethodCallHandler(_onMethodCall);

    _callPendingMethodCalls();
  }

  /// Applies the requested settings before [TizenView] is created.
  void _callPendingMethodCalls() {
    if (hasNavigationDelegate) {
      _invokeChannelMethod<void>(
          'hasNavigationDelegate', hasNavigationDelegate);
    }

    _pendingMethodCalls.forEach((String method, dynamic arguments) {
      _tizenWebViewChannel.invokeMethod<void>(method, arguments);
    });
    _pendingMethodCalls.clear();
  }

  /// Loads the file located on the specified [absoluteFilePath].
  Future<void> loadFile(String absoluteFilePath) {
    return _invokeChannelMethod<void>('loadFile', absoluteFilePath);
  }

  /// Loads the Flutter asset specified in the pubspec.yaml file.
  Future<void> loadFlutterAsset(String key) {
    return _invokeChannelMethod<void>('loadFlutterAsset', key);
  }

  /// Loads the supplied HTML string.
  Future<void> loadHtmlString(
    String html, {
    String? baseUrl,
  }) {
    return _invokeChannelMethod<void>('loadHtmlString', <String, Object?>{
      'html': html,
      'baseUrl': baseUrl,
    });
  }

  /// Makes a specific HTTP request ands loads the response in the webview.
  Future<void> loadRequest(String uri) {
    return _invokeChannelMethod<void>(
        'loadRequest', <String?, String?>{'url': uri});
  }

  /// Makes a specific HTTP request with params ands loads the response in the webview.
  Future<void> loadRequestWithParams(LoadRequestParams params) {
    return _invokeChannelMethod<void>(
        'loadRequestWithParams', <String?, Object?>{
      'url': params.uri.toString(),
      'body': params.body,
      'method': params.method.index,
      'headers': params.headers,
    });
  }

  /// Accessor to the current URL that the WebView is displaying.
  Future<String?> currentUrl() => _invokeChannelMethod<String>('currentUrl');

  /// Checks whether there's a back history item.
  Future<bool> canGoBack() async {
    return await _invokeChannelMethod<bool>('canGoBack') ?? false;
  }

  /// Checks whether there's a forward history item.
  Future<bool> canGoForward() async {
    return await _invokeChannelMethod<bool>('canGoForward') ?? false;
  }

  /// Goes back in the history of this WebView.
  Future<void> goBack() => _invokeChannelMethod<void>('goBack');

  /// Goes forward in the history of this WebView.
  Future<void> goForward() => _invokeChannelMethod<void>('goForward');

  /// Reloads the current URL.
  Future<void> reload() => _invokeChannelMethod<void>('reload');

  /// Clears all caches used by the [WebView].
  Future<void> clearCache() => _invokeChannelMethod<void>('clearCache');

  /// Sets the JavaScript execution mode to be used by the webview.
  Future<void> setJavaScriptMode(int javaScriptMode) =>
      _invokeChannelMethod<void>('javaScriptMode', javaScriptMode);

  /// Returns the title of the currently loaded page.
  Future<String?> getTitle() => _invokeChannelMethod<String>('getTitle');

  /// Sets the scrolled position of this view.
  Future<void> scrollTo(int x, int y) =>
      _invokeChannelMethod<void>('scrollTo', <String, int>{
        'x': x,
        'y': y,
      });

  /// Moves the scrolled position of this view.
  Future<void> scrollBy(int x, int y) =>
      _invokeChannelMethod<void>('scrollBy', <String, int>{
        'x': x,
        'y': y,
      });

  /// Returns the scroll position of this view set by [scrollTo].
  Future<Offset> getScrollPosition() async {
    final Map<String, Object?>? position =
        (await _invokeChannelMethod<Map<Object?, Object?>>('getScrollPosition'))
            ?.cast<String, Object?>();
    if (position == null) {
      return Offset.zero;
    }
    return Offset(position['x']! as double, position['y']! as double);
  }

  /// Sets the background color of this WebView.
  Future<void> setBackgroundColor(Color color) =>
      _invokeChannelMethod<void>('backgroundColor', color.value);

  /// Adds a new JavaScript channel to the set of enabled channels.
  Future<void> addJavaScriptChannel(
      JavaScriptChannelParams javaScriptChannelParams) {
    _javaScriptChannelParams[javaScriptChannelParams.name] =
        javaScriptChannelParams;
    return _invokeChannelMethod<void>(
        'addJavaScriptChannel', javaScriptChannelParams.name);
  }

  /// Runs the given JavaScript in the context of the current page.
  Future<void> runJavaScript(String javaScript) =>
      _invokeChannelMethod<void>('runJavaScript', javaScript);

  /// Runs the given JavaScript in the context of the current page, and returns the result.
  Future<Object> runJavaScriptReturningResult(String javaScript) async {
    final String? result = await _invokeChannelMethod<String?>(
        'runJavaScriptReturningResult', javaScript);
    if (result == null) {
      return '';
    } else if (result == 'true') {
      return true;
    } else if (result == 'false') {
      return false;
    }
    return num.tryParse(result) ?? result;
  }

  /// Sets the value used for the HTTP `User-Agent:` request header.
  Future<void> setUserAgent(String? userAgent) =>
      _invokeChannelMethod<void>('userAgent', userAgent);
}
