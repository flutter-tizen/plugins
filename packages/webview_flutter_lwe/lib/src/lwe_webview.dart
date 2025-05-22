// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter_tizen/widgets.dart';
import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

/// The channel name of [LweWebView].
const String kLweWebViewChannelName = 'plugins.flutter.io/lwe_webview_';

/// A lwe webview that displays web pages.
class LweWebView {
  /// Whether the [LweNavigationDelegate] is set by the [PlatformWebViewController].
  ///
  /// Defaults to false.
  bool hasNavigationDelegate = false;

  late final MethodChannel _lweWebViewChannel;
  bool _isCreated = false;

  final Map<String, JavaScriptChannelParams> _javaScriptChannelParams =
      <String, JavaScriptChannelParams>{};
  final List<(String, dynamic)> _pendingMethodCalls = <(String, dynamic)>[];

  Future<bool?> _onMethodCall(MethodCall call) async {
    switch (call.method) {
      case 'javaScriptChannelMessage':
        final Map<String, Object?> arguments =
            (call.arguments as Map<Object?, Object?>).cast<String, Object?>();
        final String channel = arguments['channel']! as String;
        final String message = arguments['message']! as String;
        if (_javaScriptChannelParams.containsKey(channel)) {
          _javaScriptChannelParams[channel]?.onMessageReceived(
            JavaScriptMessage(message: message),
          );
        }

        return true;
    }

    throw MissingPluginException(
      '${call.method} was invoked but has no handler',
    );
  }

  Future<T?> _invokeChannelMethod<T>(String method, [dynamic arguments]) async {
    if (!_isCreated) {
      _pendingMethodCalls.add((method, arguments));
      return null;
    }

    return _lweWebViewChannel.invokeMethod<T>(method, arguments);
  }

  /// Called when [TizenView] is created. Invokes the requested method call before [LweWebView] is created.
  void onCreate(int viewId) {
    _isCreated = true;
    _lweWebViewChannel = MethodChannel(
      kLweWebViewChannelName + viewId.toString(),
    );
    _lweWebViewChannel.setMethodCallHandler(_onMethodCall);

    _callPendingMethodCalls();
  }

  /// Applies the requested settings before [TizenView] is created.
  Future<void> _callPendingMethodCalls() async {
    if (hasNavigationDelegate) {
      await _invokeChannelMethod<void>(
        'hasNavigationDelegate',
        hasNavigationDelegate,
      );
    }

    for (final (String method, dynamic arguments) in _pendingMethodCalls) {
      await _lweWebViewChannel.invokeMethod<void>(method, arguments);
    }
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
  Future<void> loadHtmlString(String html, {String? baseUrl}) {
    return _invokeChannelMethod<void>('loadHtmlString', <String, Object?>{
      'html': html,
      'baseUrl': baseUrl,
    });
  }

  /// Makes a specific HTTP request ands loads the response in the webview.
  Future<void> loadRequest(String uri) {
    return _invokeChannelMethod<void>('loadRequest', <String?, String?>{
      'url': uri,
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
      _invokeChannelMethod<void>('scrollTo', <String, int>{'x': x, 'y': y});

  /// Moves the scrolled position of this view.
  Future<void> scrollBy(int x, int y) =>
      _invokeChannelMethod<void>('scrollBy', <String, int>{'x': x, 'y': y});

  /// Returns the current scroll position of this view.
  Future<Offset> getScrollPosition() async {
    final Map<String, Object?>? position =
        (await _invokeChannelMethod<Map<Object?, Object?>>(
      'getScrollPosition',
    ))
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
    JavaScriptChannelParams javaScriptChannelParams,
  ) {
    // When JavaScript channel with the same name exists make sure to remove it
    // before registering the new channel.
    if (_javaScriptChannelParams.containsKey(javaScriptChannelParams.name)) {
      _invokeChannelMethod<void>(
        'removeJavaScriptChannel',
        javaScriptChannelParams.name,
      );
    }

    _javaScriptChannelParams[javaScriptChannelParams.name] =
        javaScriptChannelParams;

    return _invokeChannelMethod<void>(
      'addJavaScriptChannel',
      javaScriptChannelParams.name,
    );
  }

  /// Removes the JavaScript channel with the matching name from the set of
  /// enabled channels.
  Future<void> removeJavaScriptChannel(String javaScriptChannelName) async {
    final JavaScriptChannelParams? params =
        _javaScriptChannelParams[javaScriptChannelName];

    if (params == null) {
      return;
    }

    _javaScriptChannelParams.remove(javaScriptChannelName);
    return _invokeChannelMethod<void>(
      'removeJavaScriptChannel',
      javaScriptChannelName,
    );
  }

  /// Runs the given JavaScript in the context of the current page.
  Future<void> runJavaScript(String javaScript) =>
      _invokeChannelMethod<void>('runJavaScript', javaScript);

  /// Runs the given JavaScript in the context of the current page, and returns the result.
  Future<Object> runJavaScriptReturningResult(String javaScript) async {
    final String? result = await _invokeChannelMethod<String?>(
      'runJavaScriptReturningResult',
      javaScript,
    );
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
      _invokeChannelMethod<void>('setUserAgent', userAgent);

  /// Gets the HTTP 'User-Agent:' request header.
  Future<String?> getUserAgent() async {
    final String? result = await _invokeChannelMethod<String?>('getUserAgent');
    return result;
  }
}
