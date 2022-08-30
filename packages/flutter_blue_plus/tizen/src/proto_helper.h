// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_BLUE_PLUS_TIZEN_PROTO_HELPER_H
#define FLUTTER_BLUE_PLUS_TIZEN_PROTO_HELPER_H

#include "GATT/bluetooth_characteristic.h"
#include "GATT/bluetooth_descriptor.h"
#include "GATT/bluetooth_service.h"
#include "bluetooth_device_controller.h"
#include "bluetooth_manager.h"
#include "flutterblue.pb.h"
#include "utils.h"

namespace flutter_blue_plus_tizen {

proto::gen::DiscoverServicesResult GetProtoServiceDiscoveryResult(
    const BluetoothDeviceController& device,
    const std::vector<btGatt::PrimaryService*>& services);

proto::gen::CharacteristicProperties GetProtoCharacteristicProperties(
    int properties);

proto::gen::BluetoothDevice ToProtoDevice(
    const BluetoothDeviceController& device) noexcept;

proto::gen::BluetoothService ToProtoService(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service) noexcept;

proto::gen::BluetoothCharacteristic ToProtoCharacteristic(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service,
    const btGatt::BluetoothCharacteristic& characteristic) noexcept;

proto::gen::BluetoothDescriptor ToProtoDescriptor(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service,
    const btGatt::BluetoothCharacteristic& characteristic,
    const btGatt::BluetoothDescriptor& descriptor) noexcept;

std::vector<uint8_t> MessageToVector(
    const google::protobuf::MessageLite& message_lite) noexcept;

proto::gen::DeviceStateResponse_BluetoothDeviceState ToProtoDeviceState(
    const BluetoothDeviceController::State state);

BleScanSettings FromProtoScanSettings(const proto::gen::ScanSettings& settings);

proto::gen::AdvertisementData ToProtoAdvertisementData(
    const AdvertisementData& advertisement_data) noexcept;

proto::gen::BluetoothState_State ToProtoBluetoothState(
    const BluetoothManager::BluetoothState bluetooth_state);

}  // namespace flutter_blue_plus_tizen
#endif  // FLUTTER_BLUE_PLUS_TIZEN_PROTO_HELPER_H
