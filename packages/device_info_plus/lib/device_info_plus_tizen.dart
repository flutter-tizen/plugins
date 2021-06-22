// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:device_info_plus_platform_interface/device_info_plus_platform_interface.dart';
import 'package:flutter/services.dart';

/// Information derived from `system_info`.
///
/// See: https://docs.tizen.org/application/native/guides/device/system
class TizenDeviceInfo {
  /// Tizen device info class.
  TizenDeviceInfo({
    required this.modelName,
    required this.cpuArch,
    required this.nativeApiVersion,
    required this.platformVersion,
    required this.webApiVersion,
    required this.buildDate,
    required this.buildId,
    required this.buildString,
    required this.buildTime,
    required this.buildType,
    required this.buildVariant,
    required this.buildRelease,
    required this.deviceType,
    required this.manufacturer,
  });

  /// http://tizen.org/system/model_name
  ///
  /// The value is an empty String if it is not available.
  final String modelName;

  /// http://tizen.org/feature/platform.core.cpu.arch
  ///
  /// The value is an empty String if it is not available.
  final String cpuArch;

  /// http://tizen.org/feature/platform.native.api.version
  ///
  /// The value is an empty String if it is not available.
  final String nativeApiVersion;

  /// http://tizen.org/feature/platform.version
  ///
  /// The value is an empty String if it is not available.
  final String platformVersion;

  /// http://tizen.org/feature/platform.web.api.version
  ///
  /// The value is an empty String if it is not available.
  final String webApiVersion;

  /// http://tizen.org/system/build.date
  ///
  /// The value is an empty String if it is not available.
  final String buildDate;

  /// http://tizen.org/system/build.id
  ///
  /// The value is an empty String if it is not available.
  final String buildId;

  /// http://tizen.org/system/build.string
  ///
  /// The value is an empty String if it is not available.
  final String buildString;

  /// http://tizen.org/system/build.time
  ///
  /// The value is an empty String if it is not available.
  final String buildTime;

  /// http://tizen.org/system/build.type
  ///
  /// The value is an empty String if it is not available.
  final String buildType;

  /// http://tizen.org/system/build.variant
  ///
  /// The value is an empty String if it is not available.
  final String buildVariant;

  /// http://tizen.org/system/build.release
  ///
  /// The value is an empty String if it is not available.
  final String buildRelease;

  /// http://tizen.org/system/device_type
  ///
  /// The value is an empty String if it is not available.
  final String deviceType;

  /// http://tizen.org/system/manufacturer
  ///
  /// The value is an empty String if it is not available.
  final String manufacturer;

  /// Deserializes from the message received from [_kChannel].
  static TizenDeviceInfo fromMap(Map<String, dynamic> map) {
    return TizenDeviceInfo(
      modelName: map['modelName'] as String? ?? '',
      cpuArch: map['cpuArch'] as String? ?? '',
      nativeApiVersion: map['nativeApiVersion'] as String? ?? '',
      platformVersion: map['platformVersion'] as String? ?? '',
      webApiVersion: map['webApiVersion'] as String? ?? '',
      buildDate: map['buildDate'] as String? ?? '',
      buildId: map['buildId'] as String? ?? '',
      buildString: map['buildString'] as String? ?? '',
      buildTime: map['buildTime'] as String? ?? '',
      buildType: map['buildType'] as String? ?? '',
      buildVariant: map['buildVariant'] as String? ?? '',
      buildRelease: map['buildRelease'] as String? ?? '',
      deviceType: map['deviceType'] as String? ?? '',
      manufacturer: map['manufacturer'] as String? ?? '',
    );
  }
}

/// An implementation of [DeviceInfoPlatform] that uses method channels.
class MethodChannelDeviceInfoTizen extends DeviceInfoPlatform {
  /// The method channel used to interact with the native platform.
  MethodChannel channel =
      const MethodChannel('dev.fluttercommunity.plus/device_info');

  /// Method channel for Tizen devices
  Future<TizenDeviceInfo> tizenInfo() async {
    return TizenDeviceInfo.fromMap((await channel
            .invokeMapMethod<String, dynamic>('getTizenDeviceInfo')) ??
        <String, dynamic>{});
  }
}

/// Provides device and operating system information of a Tizen device.
class DeviceInfoPluginTizen {
  /// No work is done when instantiating the plugin. It's safe to call this
  /// repeatedly or in performance-sensitive blocks.
  DeviceInfoPluginTizen();

  static final MethodChannelDeviceInfoTizen _tizenInstance =
      MethodChannelDeviceInfoTizen();

  /// This information does not change from call to call. Cache it.
  TizenDeviceInfo? _cachedTizenDeviceInfo;

  /// Information derived from `system_info`.
  ///
  /// See: https://docs.tizen.org/application/native/guides/device/system
  Future<TizenDeviceInfo> get tizenInfo async =>
      _cachedTizenDeviceInfo ??= await _tizenInstance.tizenInfo();
}
