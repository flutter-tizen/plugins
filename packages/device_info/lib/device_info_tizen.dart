// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:device_info_platform_interface/device_info_platform_interface.dart';
import 'package:device_info_platform_interface/method_channel/method_channel_device_info.dart';

// model
class TizenDeviceInfo {
  TizenDeviceInfo(
      {this.modelName,
      this.cpuArch,
      this.nativeApiVersion,
      this.platformVersion,
      this.webApiVersion,
      this.buildDate,
      this.buildId,
      this.buildString,
      this.buildTime,
      this.buildType,
      this.buildVariant,
      this.buildRelease,
      this.deviceType,
      this.manufacturer});

  final String modelName;
  final String cpuArch;
  final String nativeApiVersion;
  final String platformVersion;
  final String webApiVersion;
  final String buildDate;
  final String buildId;
  final String buildString;
  final String buildTime;
  final String buildType;
  final String buildVariant;
  final String buildRelease;
  final String deviceType;
  final String manufacturer;

  /// Deserializes from the message received from [_kChannel].
  static TizenDeviceInfo fromMap(Map<String, dynamic> map) {
    return TizenDeviceInfo(
      modelName: map['modelName'],
      cpuArch: map['cpuArch'],
      nativeApiVersion: map['nativeApiVersion'],
      platformVersion: map['platformVersion'],
      webApiVersion: map['webApiVersion'],
      buildDate: map['buildDate'],
      buildId: map['buildId'],
      buildString: map['buildString'],
      buildTime: map['buildTime'],
      buildType: map['buildType'],
      buildVariant: map['buildVariant'],
      buildRelease: map['buildRelease'],
      deviceType: map['deviceType'],
      manufacturer: map['manufacturer'],
    );
  }

  /// Deserializes message as List<String>
  static List<String> _fromList(dynamic message) {
    final List<dynamic> list = message;
    return List<String>.from(list);
  }
}

// method_channel
/// An implementation of [DeviceInfoPlatform] that uses method channels.
class MethodChannelDeviceInfoTizen extends MethodChannelDeviceInfo {
  // Method channel for Tizen devices
  Future<TizenDeviceInfo> tizenInfo() async {
    return TizenDeviceInfo.fromMap(
      (await channel.invokeMethod('getTizenDeviceInfo'))
          .cast<String, dynamic>(),
    );
  }
}

class DeviceInfoPluginTizen {
  static MethodChannelDeviceInfoTizen _tizenInstance =
      MethodChannelDeviceInfoTizen();

  DeviceInfoPluginTizen();

  /// This information does not change from call to call. Cache it.
  TizenDeviceInfo _cachedTizenDeviceInfo;

  Future<TizenDeviceInfo> get tizenInfo async =>
      _cachedTizenDeviceInfo ??= await _tizenInstance.tizenInfo();
}
