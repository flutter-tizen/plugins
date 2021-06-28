// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:connectivity_plus_platform_interface/connectivity_plus_platform_interface.dart';
import 'package:connectivity_plus_platform_interface/method_channel_connectivity.dart';

/// The Tizen implementation of [ConnectivityPlatform].
///
/// This class sets the default instance of [ConnectivityPlatform] to
/// Tizen's implementation of [ConnectivityPlatform].
/// This is required to avoid platform interface being set to Linux's.
/// https://github.com/fluttercommunity/plus_plugins/blob/c7c942d1d19595a536c6ae18168d3781ddef7785/packages/connectivity_plus/lib/connectivity_plus.dart#L43-L48
class ConnectivityPlugin extends ConnectivityPlatform {
  /// Registers this class as the default instance of [ConnectivityPlatform].
  static void register() {
    ConnectivityPlatform.instance = ConnectivityPlugin();
  }

  final MethodChannelConnectivity _methodChannelConnectivity =
      MethodChannelConnectivity();

  @override
  Stream<ConnectivityResult> get onConnectivityChanged =>
      _methodChannelConnectivity.onConnectivityChanged;

  @override
  Future<ConnectivityResult> checkConnectivity() =>
      _methodChannelConnectivity.checkConnectivity();
}
