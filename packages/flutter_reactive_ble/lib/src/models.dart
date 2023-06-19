// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
import 'package:reactive_ble_platform_interface/reactive_ble_platform_interface.dart';

const String kDeviceId = 'device_id';
const String kDeviceName = 'device_name';
const String kServiceIds = 'service_ids';
const String kServiceData = 'service_data';
const String kManufacturerData = 'manufacturer_data';
const String kRssi = 'rssi';
const String kConnectionState = 'connection_state';
const String kCharacteristicId = 'characteristic_id';
const String kServiceId = 'service_id';
const String kIsReadable = 'is_readable';
const String kIsWritableWithResponse = 'is_writable_with_response';
const String kIsWritableWithoutResponse = 'is_writable_without_response';
const String kIsNotifiable = 'is_notifiable';
const String kIsIndicatable = 'is_indicatable';
const String kIncludedServices = 'included_services';
const String kCharacteristicIds = 'characteristic_ids';
const String kCharacteristics = 'characteristics';
const String kQualifiedCharacteristic = 'qualified_characteristic';
const String kResult = 'result';
const String kMtu = 'mtu';
const String kValue = 'value';

extension DiscoveredDeviceExtension on DiscoveredDevice {
  static DiscoveredDevice fromMap(Map<String, Object?> map) {
    final serviceUuids =
        (map[kServiceIds] as List).cast<String>().map(Uuid.parse).toList();
    final serviceData = (map[kServiceData] as Map)
        .cast<String, Uint8List>()
        .map((String key, Uint8List value) => MapEntry(Uuid.parse(key), value));
    return DiscoveredDevice(
      id: map[kDeviceId] as String,
      name: map[kDeviceName] as String? ?? '',
      serviceData: serviceData,
      manufacturerData: map[kManufacturerData] as Uint8List,
      rssi: map[kRssi] as int,
      serviceUuids: serviceUuids,
    );
  }
}

extension ScanResultExtension on ScanResult {
  static ScanResult fromMap(Map<String, Object?> map) {
    final discoveredDevice = DiscoveredDeviceExtension.fromMap(map);
    return ScanResult(result: Result.success(discoveredDevice));
  }
}

extension ConnectionStateUpdateExtension on ConnectionStateUpdate {
  static ConnectionStateUpdate fromMap(Map<String, Object?> map) =>
      ConnectionStateUpdate(
        deviceId: map[kDeviceId] as String,
        connectionState: DeviceConnectionState.values
            .byName(map[kConnectionState] as String),
        failure: null,
      );
}

extension DiscoveredCharacteristicExtension on DiscoveredCharacteristic {
  static DiscoveredCharacteristic fromMap(Map<Object?, Object?> map) =>
      DiscoveredCharacteristic(
        characteristicId: Uuid.parse(map[kCharacteristicId] as String),
        serviceId: Uuid.parse(map[kServiceId] as String),
        isReadable: map[kIsReadable] as bool,
        isWritableWithResponse: map[kIsWritableWithResponse] as bool,
        isWritableWithoutResponse: map[kIsWritableWithoutResponse] as bool,
        isNotifiable: map[kIsNotifiable] as bool,
        isIndicatable: map[kIsIndicatable] as bool,
      );
}

extension DiscoveredServiceExtension on DiscoveredService {
  static DiscoveredService fromMap(Map<Object?, Object?> map) {
    final includedServices = (map[kIncludedServices] as List)
        .cast<Map>()
        .map(DiscoveredServiceExtension.fromMap)
        .toList();
    final characteristics = (map[kCharacteristics] as List)
        .cast<Map>()
        .map(DiscoveredCharacteristicExtension.fromMap)
        .toList();
    final characteristicIds = (map[kCharacteristicIds] as List)
        .cast<String>()
        .map(Uuid.parse)
        .toList();
    return DiscoveredService(
      serviceId: Uuid.parse(map[kServiceId] as String),
      characteristicIds: characteristicIds,
      includedServices: includedServices,
      characteristics: characteristics,
    );
  }
}

extension QualifiedCharacteristicExtension on QualifiedCharacteristic {
  Map<String, Object> toMap() => {
        kDeviceId: deviceId,
        kServiceId: '$serviceId',
        kCharacteristicId: '$characteristicId',
      };

  static QualifiedCharacteristic fromMap(Map<String, Object?> map) =>
      QualifiedCharacteristic(
        deviceId: map[kDeviceId] as String,
        serviceId: Uuid.parse(map[kServiceId] as String),
        characteristicId: Uuid.parse(map[kCharacteristicId] as String),
      );
}

extension CharacteristicValueExtension on CharacteristicValue {
  static CharacteristicValue fromMap(Map<String, Object?> map) {
    final characteristicMap =
        (map[kQualifiedCharacteristic] as Map).cast<String, Object?>();
    final valueList = (map[kResult] as List).cast<int>();
    return CharacteristicValue(
      characteristic:
          QualifiedCharacteristicExtension.fromMap(characteristicMap),
      result: Result.success(valueList),
    );
  }
}

extension WriteCharacteristicInfoExtension on WriteCharacteristicInfo {
  static WriteCharacteristicInfo fromMap(Map<String, Object?> map) {
    final characteristicMap =
        (map[kQualifiedCharacteristic] as Map).cast<String, Object?>();
    return WriteCharacteristicInfo(
      characteristic:
          QualifiedCharacteristicExtension.fromMap(characteristicMap),
      result: const Result.success(Unit()),
    );
  }
}
