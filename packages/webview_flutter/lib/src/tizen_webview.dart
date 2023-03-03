// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter_tizen/widgets.dart';
import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

/// Signature for callbacks that report a method calls by navigation delegate method channel.
typedef NavigationDelegateMethodChannelCallback = Future<dynamic> Function(
    MethodCall method);

/// A Tizen webview that displays web pages.
class TizenWebview {
  /// Constructs a new WebView.
  TizenWebview()
      : _controllerChannel =
            const MethodChannel('plugins.flutter.io/tizen_webview_controller'),
        _navigationDelegateChannel = const MethodChannel(
            'plugins.flutter.io/tizen_webview_navigation_delegate') {
    _controllerChannel.setMethodCallHandler(_onMethodCall);
  }

  final MethodChannel _controllerChannel;
  final MethodChannel _navigationDelegateChannel;
  bool _isCreated = false;
  bool _hasNavigationDelegate = false;
  final Map<String, JavaScriptChannelParams> _javaScriptChannelParams =
      <String, JavaScriptChannelParams>{};
  final Map<String, dynamic> _pendingMethodCalls = <String, dynamic>{};

  Future<bool?> _onMethodCall(MethodCall call) async {
    switch (call.method) {
      case 'javaScriptChannelMessage':
        final String channel = call.arguments['channel']! as String;
        final String message = call.arguments['message']! as String;
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
      return Future<T?>(() => null);
    }

    return _controllerChannel.invokeMethod<T>(method, arguments);
  }

  /// Called when [TizenView] is created. Invokes the requested method call before [TizenWebView] is created.
  void onCreate() {
    _isCreated = true;
    _callPendingMethodCalls();
  }

  /// Set the method call handler of the navigation delegate channel.
  void setNavigationDelegateMethodCallHandler(
      NavigationDelegateMethodChannelCallback method) {
    _navigationDelegateChannel.setMethodCallHandler(method);
  }

  /// Applies the requested settings before [TizenView] is created.
  void _callPendingMethodCalls() {
    if (_hasNavigationDelegate) {
      _invokeChannelMethod<void>(
          'setNavigationDelegate', _hasNavigationDelegate);
    }

    _pendingMethodCalls.forEach((String method, dynamic arguments) {
      _controllerChannel.invokeMethod<void>(method, arguments);
    });
    _pendingMethodCalls.clear();
  }

  /// Loads the file located on the specified [absoluteFilePath].
  Future<void> loadFile(String absoluteFilePath) async {
    assert(absoluteFilePath != null);
    _invokeChannelMethod<void>('loadFile', absoluteFilePath);
  }

  /// Loads the Flutter asset specified in the pubspec.yaml file.
  Future<void> loadFlutterAsset(String key) async {
    assert(key.isNotEmpty);
    _invokeChannelMethod<void>('loadFlutterAsset', key);
  }

  /// Loads the supplied HTML string.
  Future<void> loadHtmlString(
    String html, {
    String? baseUrl,
  }) async {
    assert(html != null);
    _invokeChannelMethod<void>('loadHtmlString', <String, dynamic>{
      'html': html,
      'baseUrl': baseUrl,
    });
  }

  /// Makes a specific HTTP request ands loads the response in the webview.
  Future<void> loadRequest(String uri) async {
    _invokeChannelMethod<void>('loadRequest', <String?, String?>{'url': uri});
  }

  /// Accessor to the current URL that the WebView is displaying.
  Future<String?> currentUrl() => _invokeChannelMethod<String>('currentUrl');

  /// Checks whether there's a back history item.
  Future<bool> canGoBack() =>
      _invokeChannelMethod<bool>('canGoBack').then((bool? result) => result!);

  /// Checks whether there's a forward history item.
  Future<bool> canGoForward() => _invokeChannelMethod<bool>('canGoForward')
      .then((bool? result) => result!);

  /// Goes back in the history of this WebView.
  Future<void> goBack() => _invokeChannelMethod<void>('goBack');

  /// Goes forward in the history of this WebView.
  Future<void> goForward() => _invokeChannelMethod<void>('goForward');

  /// Reloads the current URL.
  Future<void> reload() async {
    _invokeChannelMethod<void>('reload');
  }

  /// Clears all caches used by the [WebView].
  Future<void> clearCache() async {
    _invokeChannelMethod<void>('clearCache');
  }

  /// Sets the JavaScript execution mode to be used by the webview.
  Future<void> setJavaScriptMode(int javaScriptMode) async {
    _invokeChannelMethod<void>('javaScriptMode', javaScriptMode);
  }

  /// Returns the title of the currently loaded page.
  Future<String?> getTitle() {
    return _invokeChannelMethod<String>('getTitle');
  }

  /// Set the scrolled position of this view.
  Future<void> scrollTo(int x, int y) async {
    _invokeChannelMethod<void>('scrollTo', <String, int>{
      'x': x,
      'y': y,
    });
  }

  /// Move the scrolled position of this view.
  Future<void> scrollBy(int x, int y) {
    return _invokeChannelMethod<void>('scrollBy', <String, int>{
      'x': x,
      'y': y,
    });
  }

  /// Return the set scroll position of this view.
  Future<Offset> getScrollPosition() async {
    final dynamic position =
        await _invokeChannelMethod<dynamic>('getScrollPosition');
    if (position == null) {
      return Offset.zero;
    }
    return Offset(position['x'] as double, position['y'] as double);
  }

  /// Set the current background color of this view.
  Future<void> setBackgroundColor(Color color) async {
    _invokeChannelMethod<void>('backgroundColor', color.value);
  }

  /// Set whether controller have a NavigationDelegate.
  Future<void> setHasNavigationDelegate(bool hasNavigationDelegate) async {
    _hasNavigationDelegate = hasNavigationDelegate;
  }

  /// Adds a new JavaScript channel to the set of enabled channels.
  Future<void> addJavaScriptChannel(
      JavaScriptChannelParams javaScriptChannelParams) async {
    // When JavaScript channel with the same name exists make sure to remove it
    // before registering the new channel.
    if (_javaScriptChannelParams.containsKey(javaScriptChannelParams.name)) {
      _javaScriptChannelParams.remove(javaScriptChannelParams.name);
    }

    _javaScriptChannelParams[javaScriptChannelParams.name] =
        javaScriptChannelParams;

    _invokeChannelMethod<void>(
        'addJavaScriptChannel', javaScriptChannelParams.name);
  }

  /// Runs the given JavaScript in the context of the current page.
  Future<void> runJavaScript(String javaScript) {
    return _invokeChannelMethod<void>('runJavascript', javaScript);
  }

  /// Runs the given JavaScript in the context of the current page, and returns the result.
  Future<Object> runJavaScriptReturningResult(String javaScript) async {
    final String? result = await _invokeChannelMethod<String?>(
        'runJavascriptReturningResult', javaScript);
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
  Future<void> setUserAgent(String? userAgent) async {
    _invokeChannelMethod<void>('userAgent', userAgent);
  }
}
