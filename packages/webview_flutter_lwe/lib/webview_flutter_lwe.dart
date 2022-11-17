// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_tizen/widgets.dart';
import 'package:webview_flutter/webview_flutter.dart';
import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

/// Builds a Tizen webview.
///
/// This is used as the default implementation for [WebView.platform] on Tizen. It uses a method channel to
/// communicate with the platform code.
class LweWebView implements WebViewPlatform {
  /// Sets a tizen [WebViewPlatform].
  static void register() {
    WebView.platform = LweWebView();
    WebViewCookieManagerPlatform.instance = WebViewTizenCookieManager();
  }

  @override
  Widget build({
    required BuildContext context,
    required CreationParams creationParams,
    required WebViewPlatformCallbacksHandler webViewPlatformCallbacksHandler,
    required JavascriptChannelRegistry javascriptChannelRegistry,
    WebViewPlatformCreatedCallback? onWebViewPlatformCreated,
    Set<Factory<OneSequenceGestureRecognizer>>? gestureRecognizers,
  }) {
    assert(webViewPlatformCallbacksHandler != null);
    return GestureDetector(
      onLongPress: () {},
      excludeFromSemantics: true,
      child: TizenView(
        viewType: 'plugins.flutter.io/webview',
        onPlatformViewCreated: (int id) {
          if (onWebViewPlatformCreated == null) {
            return;
          }
          onWebViewPlatformCreated(MethodChannelWebViewPlatform(
            id,
            webViewPlatformCallbacksHandler,
            javascriptChannelRegistry,
          ));
        },
        gestureRecognizers: gestureRecognizers,
        layoutDirection: Directionality.maybeOf(context) ?? TextDirection.rtl,
        creationParams:
            MethodChannelWebViewPlatform.creationParamsToMap(creationParams),
        creationParamsCodec: const StandardMessageCodec(),
      ),
    );
  }

  @override
  Future<bool> clearCookies() {
    if (WebViewCookieManagerPlatform.instance == null) {
      throw Exception(
          'Could not clear cookies as no implementation for WebViewCookieManagerPlatform has been registered.');
    }
    return WebViewCookieManagerPlatform.instance!.clearCookies();
  }
}

/// Handles all cookie operations for the current platform.
class WebViewTizenCookieManager extends WebViewCookieManagerPlatform {
  @override
  Future<bool> clearCookies() => MethodChannelWebViewPlatform.clearCookies();

  @override
  Future<void> setCookie(WebViewCookie cookie) async {
    if (!_isValidPath(cookie.path)) {
      throw ArgumentError(
          'The path property for the provided cookie was not given a legal value.');
    }
    return MethodChannelWebViewPlatform.setCookie(cookie);
  }

  bool _isValidPath(String path) {
    // Permitted ranges based on RFC6265bis: https://datatracker.ietf.org/doc/html/draft-ietf-httpbis-rfc6265bis-02#section-4.1.1
    for (final int char in path.codeUnits) {
      if ((char < 0x20 || char > 0x3A) && (char < 0x3C || char > 0x7E)) {
        return false;
      }
    }
    return true;
  }
}
