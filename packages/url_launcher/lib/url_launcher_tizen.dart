// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:tizen_app_control/tizen_app_control.dart';
import 'package:url_launcher_platform_interface/link.dart';
import 'package:url_launcher_platform_interface/url_launcher_platform_interface.dart';

/// The Tizen implementation of [UrlLauncherPlatform].
class UrlLauncherPlugin extends UrlLauncherPlatform {
  static final Set<String> _supportedSchemes = <String>{
    'http',
    'https',
  };

  /// Registers this class as the default instance of [UrlLauncherPlatform].
  static void register() {
    UrlLauncherPlatform.instance = UrlLauncherPlugin();
  }

  @override
  final LinkDelegate? linkDelegate = null;

  String? _getUrlScheme(String url) => Uri.tryParse(url)?.scheme;

  @override
  Future<bool> canLaunch(String url) async {
    return _supportedSchemes.contains(_getUrlScheme(url));
  }

  @override
  Future<bool> launch(
    String url, {
    required bool useSafariVC,
    required bool useWebView,
    required bool enableJavaScript,
    required bool enableDomStorage,
    required bool universalLinksOnly,
    required Map<String, String> headers,
    String? webOnlyWindowName,
  }) async {
    await AppControl(
      operation: 'http://tizen.org/appcontrol/operation/view',
      uri: url,
    ).sendLaunchRequest();
    return true;
  }

  @override
  Future<void> closeWebView() {
    throw UnimplementedError('closeWebView() is not supported.');
  }

  @override
  Future<bool> supportsCloseForMode(PreferredLaunchMode mode) {
    return Future<bool>.value(false);
  }
}
