// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:flutter_inappwebview_platform_interface/flutter_inappwebview_platform_interface.dart';

/// Object specifying creation parameters for creating a [TizenCookieManager].
@immutable
class TizenCookieManagerCreationParams
    extends PlatformCookieManagerCreationParams {
  /// Creates a new [TizenCookieManagerCreationParams] instance.
  const TizenCookieManagerCreationParams();

  /// Creates a [TizenCookieManagerCreationParams] from a generic
  /// [PlatformCookieManagerCreationParams].
  factory TizenCookieManagerCreationParams.fromPlatformCookieManagerCreationParams(
    // ignore: avoid_unused_constructor_parameters
    PlatformCookieManagerCreationParams params,
  ) {
    return const TizenCookieManagerCreationParams();
  }
}

/// Tizen implementation of [PlatformCookieManager].
///
/// Only [deleteAllCookies] is supported on Tizen. Per-cookie mutation and
/// query APIs throw [UnsupportedError].
class TizenCookieManager extends PlatformCookieManager with ChannelController {
  /// Creates a new [TizenCookieManager].
  TizenCookieManager(PlatformCookieManagerCreationParams params)
    : super.implementation(
        params is TizenCookieManagerCreationParams
            ? params
            : TizenCookieManagerCreationParams.fromPlatformCookieManagerCreationParams(
                params,
              ),
      ) {
    channel = const MethodChannel(
      'com.pichillilorenzo/flutter_inappwebview_cookiemanager',
    );
    handler = _handleMethod;
    initMethodCallHandler();
  }

  Future<dynamic> _handleMethod(MethodCall call) async {}

  static TizenCookieManager? _instance;

  /// Returns the lazily-constructed singleton.
  static TizenCookieManager instance() {
    return _instance ??= TizenCookieManager(
      const TizenCookieManagerCreationParams(),
    );
  }

  Never _unsupported(String method) {
    throw UnsupportedError(
      '$method is not implemented on flutter_inappwebview_tizen.',
    );
  }

  @override
  Future<bool> setCookie({
    required WebUri url,
    required String name,
    required String value,
    String path = '/',
    String? domain,
    int? expiresDate,
    int? maxAge,
    bool? isSecure,
    bool? isHttpOnly,
    HTTPCookieSameSitePolicy? sameSite,
    @Deprecated('Use webViewController instead')
    PlatformInAppWebViewController? iosBelow11WebViewController,
    PlatformInAppWebViewController? webViewController,
  }) async {
    _unsupported('setCookie');
  }

  @override
  Future<List<Cookie>> getCookies({
    required WebUri url,
    @Deprecated('Use webViewController instead')
    PlatformInAppWebViewController? iosBelow11WebViewController,
    PlatformInAppWebViewController? webViewController,
  }) async {
    _unsupported('getCookies');
  }

  @override
  Future<Cookie?> getCookie({
    required WebUri url,
    required String name,
    @Deprecated('Use webViewController instead')
    PlatformInAppWebViewController? iosBelow11WebViewController,
    PlatformInAppWebViewController? webViewController,
  }) async {
    _unsupported('getCookie');
  }

  @override
  Future<bool> deleteCookie({
    required WebUri url,
    required String name,
    String path = '/',
    String? domain,
    @Deprecated('Use webViewController instead')
    PlatformInAppWebViewController? iosBelow11WebViewController,
    PlatformInAppWebViewController? webViewController,
  }) async {
    _unsupported('deleteCookie');
  }

  @override
  Future<bool> deleteCookies({
    required WebUri url,
    String path = '/',
    String? domain,
    @Deprecated('Use webViewController instead')
    PlatformInAppWebViewController? iosBelow11WebViewController,
    PlatformInAppWebViewController? webViewController,
  }) async {
    _unsupported('deleteCookies');
  }

  @override
  Future<bool> deleteAllCookies() async {
    final bool result =
        await channel?.invokeMethod<bool>('deleteAllCookies') ?? false;
    return result;
  }

  @override
  void dispose() {
    disposeChannel();
  }
}

/// Internal hook that exposes [_handleMethod] without making it part of the
/// public surface.
extension InternalCookieManager on TizenCookieManager {
  /// Returns the channel handler bound to [_handleMethod].
  Future<dynamic> Function(MethodCall call) get handleMethod => _handleMethod;
}
