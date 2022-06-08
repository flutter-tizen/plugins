#include "proto_helper.h"

namespace flutter_blue_tizen {

std::vector<uint8_t> MessageToVector(
    const google::protobuf::MessageLite& message_lite) noexcept {
  std::vector<uint8_t> encoded(message_lite.ByteSizeLong());
  message_lite.SerializeToArray(encoded.data(), message_lite.ByteSizeLong());
  return encoded;
}

proto::gen::CharacteristicProperties GetProtoCharacteristicProperties(
    int properties) {
  proto::gen::CharacteristicProperties proto_properties;
  proto_properties.set_broadcast((properties & 0x01) != 0);
  proto_properties.set_read((properties & 0x02) != 0);
  proto_properties.set_write_without_response((properties & 0x04) != 0);
  proto_properties.set_write((properties & 0x08) != 0);
  proto_properties.set_notify((properties & 0x10) != 0);
  proto_properties.set_indicate((properties & 0x20) != 0);
  proto_properties.set_authenticated_signed_writes((properties & 0x40) != 0);
  proto_properties.set_extended_properties((properties & 0x80) != 0);
  // p.set_notify_encryption_required((properties & 256) != 0);
  // p.set_indicate_encryption_required((properties & 512) != 0);
  return proto_properties;
}

proto::gen::DiscoverServicesResult GetProtoServiceDiscoveryResult(
    const BluetoothDeviceController& device,
    const std::vector<btGatt::PrimaryService*>& services) {
  proto::gen::DiscoverServicesResult discovery_result;

  for (const auto service : services) {
    *discovery_result.add_services() = ToProtoService(device, *service);
  }

  discovery_result.set_remote_id(device.cAddress());
  return discovery_result;
}

proto::gen::BluetoothCharacteristic ToProtoCharacteristic(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service,
    const btGatt::BluetoothCharacteristic& characteristic) noexcept {
  proto::gen::BluetoothCharacteristic proto;
  proto.set_remote_id(device.cAddress());
  proto.set_uuid(characteristic.Uuid());
  proto.set_allocated_properties(new proto::gen::CharacteristicProperties(
      GetProtoCharacteristicProperties(characteristic.Properties())));
  proto.set_value(characteristic.Value());
  if (service.GetType() == btGatt::ServiceType::kPrimary) {
    proto.set_serviceuuid(service.Uuid());
  } else {
    auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(service);
    proto.set_serviceuuid(secondary.PrimaryUuid());
    proto.set_secondaryserviceuuid(secondary.Uuid());
  }
  for (const auto descriptor : characteristic.GetDescriptors()) {
    *proto.add_descriptors() =
        ToProtoDescriptor(device, service, characteristic, *descriptor);
  }
  return proto;
}

proto::gen::BluetoothDescriptor ToProtoDescriptor(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service,
    const btGatt::BluetoothCharacteristic& characteristic,
    const btGatt::BluetoothDescriptor& descriptor) noexcept {
  proto::gen::BluetoothDescriptor proto;
  proto.set_remote_id(device.cAddress());
  proto.set_serviceuuid(service.Uuid());
  proto.set_characteristicuuid(characteristic.Uuid());
  proto.set_uuid(descriptor.Uuid());
  return proto;
}

proto::gen::BluetoothService ToProtoService(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service) noexcept {
  proto::gen::BluetoothService proto;

  proto.set_uuid(service.Uuid());
  proto.set_remote_id(device.cAddress());

  for (const auto characteristic : service.GetCharacteristics()) {
    *proto.add_characteristics() =
        ToProtoCharacteristic(device, service, *characteristic);
  }

  if (service.GetType() == btGatt::ServiceType::kPrimary) {
    proto.set_is_primary(true);

    for (const auto secondary :
         dynamic_cast<const btGatt::PrimaryService&>(service)
             .getSecondaryServices()) {
      *proto.add_included_services() = ToProtoService(device, *secondary);
    }

  } else {
    proto.set_is_primary(false);
  }

  return proto;
}

proto::gen::DeviceStateResponse_BluetoothDeviceState ToProtoDeviceState(
    const BluetoothDeviceController::State state) {
  using State = BluetoothDeviceController::State;
  switch (state) {
    case State::kConnected:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_CONNECTED;
    case State::kConnecting:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_CONNECTING;
    case State::kDisconnected:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
    case State::kDisconnecting:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTING;
    default:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
  }
}

}  // namespace flutter_blue_tizen