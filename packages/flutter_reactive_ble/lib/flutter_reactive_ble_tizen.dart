// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: prefer_expression_function_bodies

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:reactive_ble_platform_interface/reactive_ble_platform_interface.dart';

import 'src/models.dart';

/// Tizen implementation of [ReactiveBlePlatform].
class ReactiveBleTizen extends ReactiveBlePlatform {
  /// Registers this class as the default instance of [ReactiveBlePlatform].
  static void register() {
    ReactiveBlePlatform.instance = ReactiveBleTizen();
  }

  @visibleForTesting
  final MethodChannel methodChannel =
      const MethodChannel('flutter_reactive_ble_method');

  @override
  final Stream<BleStatus> bleStatusStream =
      const EventChannel('flutter_reactive_ble_status')
          .receiveBroadcastStream()
          .map((dynamic event) => BleStatus.values.byName(event as String));

  @override
  final Stream<ScanResult> scanStream = const EventChannel(
          'flutter_reactive_ble_scan')
      .receiveBroadcastStream()
      .map((dynamic event) =>
          ScanResultExtension.fromMap((event as Map).cast<String, Object?>()));

  @override
  final Stream<ConnectionStateUpdate> connectionUpdateStream =
      const EventChannel('flutter_reactive_ble_connected_device')
          .receiveBroadcastStream()
          .map((dynamic event) => ConnectionStateUpdateExtension.fromMap(
              (event as Map).cast<String, Object?>()));

  @override
  final Stream<CharacteristicValue> charValueUpdateStream =
      const EventChannel('flutter_reactive_ble_char_update')
          .receiveBroadcastStream()
          .map((dynamic event) => CharacteristicValueExtension.fromMap(
              (event as Map).cast<String, Object?>()));

  @override
  Future<void> initialize() => methodChannel.invokeMethod('initialize');

  @override
  Future<void> deinitialize() => methodChannel.invokeMethod('deinitialize');

  @override
  Stream<void> scanForDevices({
    required List<Uuid> withServices,
    required ScanMode scanMode, // ignored
    required bool requireLocationServicesEnabled, // ignored
  }) {
    return methodChannel.invokeMethod<void>(
      'scanForDevices',
      <String, Object>{
        kServiceIds: withServices.map((Uuid uuid) => '$uuid').toList(),
      },
    ).asStream();
  }

  @override
  Stream<void> connectToDevice(
    String id,
    Map<Uuid, List<Uuid>>? servicesWithCharacteristicsToDiscover,
    Duration? connectionTimeout,
  ) {
    // TODO(swift-kim): Support the connectionTimeout parameter.
    return methodChannel.invokeMethod<void>(
      'connectToDevice',
      <String, Object>{kDeviceId: id},
    ).asStream();
  }

  @override
  Future<void> disconnectDevice(String deviceId) => methodChannel
      .invokeMethod('disconnectDevice', <String, Object>{kDeviceId: deviceId});

  @override
  Future<List<DiscoveredService>> discoverServices(String deviceId) async {
    final result = await methodChannel.invokeListMethod<Map>(
      'discoverServices',
      <String, Object>{kDeviceId: deviceId},
    );
    return result!.map(DiscoveredServiceExtension.fromMap).toList();
  }

  @override
  Future<ConnectionPriorityInfo> requestConnectionPriority(
    String deviceId,
    ConnectionPriority priority,
  ) {
    throw UnimplementedError();
  }

  @override
  Future<int> requestMtuSize(String deviceId, int? mtu) async {
    final mtuSize = await methodChannel.invokeMethod<int>(
      'requestMtuSize',
      <String, Object?>{kDeviceId: deviceId, kMtu: mtu},
    );
    return mtuSize!;
  }

  @override
  Stream<void> readCharacteristic(QualifiedCharacteristic characteristic) {
    return methodChannel.invokeMethod<void>(
      'readCharacteristic',
      <String, Object>{kQualifiedCharacteristic: characteristic.toMap()},
    ).asStream();
  }

  @override
  Future<WriteCharacteristicInfo> writeCharacteristicWithResponse(
    QualifiedCharacteristic characteristic,
    List<int> value,
  ) async {
    final result = await methodChannel.invokeMapMethod<String, Object?>(
      'writeCharacteristicWithResponse',
      <String, Object>{
        kQualifiedCharacteristic: characteristic.toMap(),
        kValue: Uint8List.fromList(value),
      },
    );
    return WriteCharacteristicInfoExtension.fromMap(result!);
  }

  @override
  Future<WriteCharacteristicInfo> writeCharacteristicWithoutResponse(
    QualifiedCharacteristic characteristic,
    List<int> value,
  ) async {
    final result = await methodChannel.invokeMapMethod<String, Object?>(
      'writeCharacteristicWithoutResponse',
      <String, Object>{
        kQualifiedCharacteristic: characteristic.toMap(),
        kValue: Uint8List.fromList(value),
      },
    );
    return WriteCharacteristicInfoExtension.fromMap(result!);
  }

  @override
  Stream<void> subscribeToNotifications(
    QualifiedCharacteristic characteristic,
  ) {
    return methodChannel.invokeMethod<void>(
      'subscribeToNotifications',
      <String, Object>{kQualifiedCharacteristic: characteristic.toMap()},
    ).asStream();
  }

  @override
  Future<void> stopSubscribingToNotifications(
    QualifiedCharacteristic characteristic,
  ) {
    return methodChannel.invokeMethod(
      'stopSubscribingToNotifications',
      <String, Object>{kQualifiedCharacteristic: characteristic.toMap()},
    );
  }
}
