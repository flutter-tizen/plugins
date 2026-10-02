// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';

import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

/// Handles all cookie operations for the Tizen platform.
class TizenWebViewCookieManager extends PlatformWebViewCookieManager {
  /// Creates a new [TizenWebViewCookieManager].
  TizenWebViewCookieManager(super.params) : super.implementation();

  static const MethodChannel _cookieManagerChannel = MethodChannel(
    'plugins.flutter.io/tizen_cookie_manager',
  );

  @override
  Future<bool> clearCookies() async {
    return await _cookieManagerChannel.invokeMethod<bool>('clearCookies') ?? false;
  }

  @override
  Future<List<WebViewCookie>> getCookies(Uri url) async {
    final String? cookies = await _cookieManagerChannel.invokeMethod<String>(
      'getCookies',
      url.toString(),
    );
    if (cookies == null || cookies.isEmpty) {
      return <WebViewCookie>[];
    }
    return cookies
        .split(';')
        .map((String cookie) => cookie.trim())
        .where((String cookie) => cookie.isNotEmpty)
        .map((String cookie) {
          final int separator = cookie.indexOf('=');
          return WebViewCookie(
            name: separator < 0 ? cookie : cookie.substring(0, separator),
            value: separator < 0 ? '' : cookie.substring(separator + 1),
            domain: url.host,
          );
        })
        .toList();
  }

  @override
  Future<void> setCookie(WebViewCookie cookie) async {
    if (!_isValidPath(cookie.path)) {
      throw ArgumentError('The path property for the provided cookie was not given a legal value.');
    }
    final String host = cookie.domain.startsWith('.') ? cookie.domain.substring(1) : cookie.domain;
    final url = Uri(scheme: 'https', host: host, path: cookie.path).toString();
    final line =
        '${Uri.encodeComponent(cookie.name)}=${Uri.encodeComponent(cookie.value)}; path=${cookie.path}';
    try {
      await _cookieManagerChannel.invokeMethod<void>('setCookie', <String, String>{
        'url': url,
        'cookie': line,
      });
    } on PlatformException catch (e) {
      if (e.code == 'Unsupported') {
        throw UnimplementedError('setCookie is not supported by this web engine.');
      }
      rethrow;
    }
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
