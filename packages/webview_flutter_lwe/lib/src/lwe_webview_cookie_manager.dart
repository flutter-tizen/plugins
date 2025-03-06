// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';

import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

/// Handles all cookie operations for the current platform.
class LweWebViewCookieManager extends PlatformWebViewCookieManager {
  /// Creates a new [LweWebViewCookieManager].
  LweWebViewCookieManager(super.params) : super.implementation();

  static const MethodChannel _cookieManagerChannel = MethodChannel(
    'plugins.flutter.io/lwe_cookie_manager',
  );

  @override
  Future<bool> clearCookies() async {
    return await _cookieManagerChannel.invokeMethod<bool>('clearCookies') ??
        false;
  }

  @override
  Future<void> setCookie(WebViewCookie cookie) async {
    if (!_isValidPath(cookie.path)) {
      throw ArgumentError(
        'The path property for the provided cookie was not given a legal value.',
      );
    }
    throw UnimplementedError(
      'This version of `LweWebViewCookieManager` currently has no '
      'implementation for setCookie method.',
    );
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
