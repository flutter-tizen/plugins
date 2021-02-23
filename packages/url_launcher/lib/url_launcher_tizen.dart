// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:meta/meta.dart';
import 'package:url_launcher_platform_interface/link.dart';
import 'package:url_launcher_platform_interface/url_launcher_platform_interface.dart';

import 'app_control.dart';

class UrlLauncherPlugin extends UrlLauncherPlatform {
  static final Set<String> _supportedSchemes = <String>{
    'file',
    'http',
    'https',
  };

  /// Registers this class as the default instance of [UrlLauncherPlatform].
  static void register() {
    UrlLauncherPlatform.instance = UrlLauncherPlugin();
  }

  @override
  final LinkDelegate linkDelegate = null;

  String _getUrlScheme(String url) => Uri.tryParse(url)?.scheme;

  @override
  Future<bool> canLaunch(String url) async {
    return _supportedSchemes.contains(_getUrlScheme(url));
  }

  @override
  Future<bool> launch(
    String url, {
    // None of these options are used in Tizen.
    @required bool useSafariVC,
    @required bool useWebView,
    @required bool enableJavaScript,
    @required bool enableDomStorage,
    @required bool universalLinksOnly,
    @required Map<String, String> headers,
    String webOnlyWindowName,
  }) async {
    AppControl appControl;
    try {
      appControl = AppControl()
        ..create()
        ..setOperation(APP_CONTROL_OPERATION_VIEW)
        ..setUri(url)
        ..sendLaunchRequest();
      return true;
    } finally {
      appControl?.destroy();
    }
  }
}
