// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:network_info_plus_platform_interface/network_info_plus_platform_interface.dart';
import 'package:network_info_plus_platform_interface/method_channel_network_info.dart';

/// The Tizen implementation of [NetworkInfoPlatform].
///
/// This class sets the default instance of [NetworkInfoPlatform] to
/// Tizen's implementation of [NetworkInfoPlatform].
/// This is required to avoid platform interface being set to Linux's.
/// https://github.com/fluttercommunity/plus_plugins/blob/c517f7d46b9de4e301a6d844f4f3197045af89dc/packages/network_info_plus/lib/network_info_plus.dart#L44-L48
class NetworkInfoPlugin extends NetworkInfoPlatform {
  /// Registers this class as the default instance of [NetworkInfoPlatform].
  static void register() {
    NetworkInfoPlatform.instance = NetworkInfoPlugin();
  }

  final MethodChannelNetworkInfo _methodChannelNetworkInfo =
      MethodChannelNetworkInfo();

  @override
  Future<String?> getWifiName() => _methodChannelNetworkInfo.getWifiName();

  @override
  Future<String?> getWifiBSSID() => _methodChannelNetworkInfo.getWifiBSSID();

  @override
  Future<String?> getWifiIP() => _methodChannelNetworkInfo.getWifiIP();

  @override
  Future<String?> getWifiIPv6() => _methodChannelNetworkInfo.getWifiIPv6();

  @override
  Future<String?> getWifiSubmask() =>
      _methodChannelNetworkInfo.getWifiSubmask();

  @override
  Future<String?> getWifiGatewayIP() =>
      _methodChannelNetworkInfo.getWifiGatewayIP();

  @override
  Future<String?> getWifiBroadcast() =>
      _methodChannelNetworkInfo.getWifiBroadcast();

  @override
  Future<LocationAuthorizationStatus> requestLocationServiceAuthorization({
    bool requestAlwaysLocationUsage = false,
  }) =>
      _methodChannelNetworkInfo.requestLocationServiceAuthorization(
          requestAlwaysLocationUsage: requestAlwaysLocationUsage);

  @override
  Future<LocationAuthorizationStatus> getLocationServiceAuthorization() =>
      _methodChannelNetworkInfo.getLocationServiceAuthorization();
}
