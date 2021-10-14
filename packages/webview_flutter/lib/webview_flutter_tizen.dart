// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:typed_data';

import 'package:flutter/foundation.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter/semantics.dart';
import 'package:flutter/rendering.dart';

import 'package:webview_flutter/webview_flutter.dart';
import 'package:webview_flutter/platform_interface.dart';
import 'package:webview_flutter_platform_interface/src/method_channel/webview_method_channel.dart';
import 'package:webview_flutter_platform_interface/webview_flutter_platform_interface.dart';

part 'src/platform_view.dart';
part 'src/platform_view_tizen.dart';

/// Builds an Tizen webview.
///
/// This is used as the default implementation for [WebView.platform] on Tizen. It uses a method channel to
/// communicate with the platform code.
class TizenWebView implements WebViewPlatform {
  /// Sets a tizen [WebViewPlatform].
  static void register() {
    WebView.platform = TizenWebView();
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
  Future<bool> clearCookies() => MethodChannelWebViewPlatform.clearCookies();
}
