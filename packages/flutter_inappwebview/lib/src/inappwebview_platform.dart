// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_inappwebview_platform_interface/flutter_inappwebview_platform_interface.dart';

import 'cookie_manager.dart';
import 'in_app_webview/in_app_webview.dart';
import 'in_app_webview/in_app_webview_controller.dart';

/// Implementation of [InAppWebViewPlatform] using the Tizen WebView EWK API.
///
/// Only the implemented surface is overridden; calls to unsupported
/// features fall through to the platform interface defaults, which raise
/// [UnimplementedError].
class TizenInAppWebViewPlatform extends InAppWebViewPlatform {
  /// Registers this class as the default instance of [InAppWebViewPlatform].
  static void register() {
    registerWith();
  }

  /// Registers this class as the default instance of [InAppWebViewPlatform].
  static void registerWith() {
    InAppWebViewPlatform.instance = TizenInAppWebViewPlatform();
  }

  @override
  TizenCookieManager createPlatformCookieManager(
    PlatformCookieManagerCreationParams params,
  ) {
    return TizenCookieManager(params);
  }

  @override
  TizenInAppWebViewController createPlatformInAppWebViewController(
    PlatformInAppWebViewControllerCreationParams params,
  ) {
    return TizenInAppWebViewController(params);
  }

  @override
  TizenInAppWebViewController createPlatformInAppWebViewControllerStatic() {
    return TizenInAppWebViewController.static();
  }

  @override
  TizenInAppWebViewWidget createPlatformInAppWebViewWidget(
    PlatformInAppWebViewWidgetCreationParams params,
  ) {
    return TizenInAppWebViewWidget(params);
  }
}
