#include "flutter_blue_tizen_plugin.h"

#include <app_control.h>
#include <bluetooth.h>
#include <flutter/event_channel.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutterblue.pb.h>
#include <log.h>
#include <state_handler.h>
#include <system_info.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "bluetooth_device_controller.h"
#include "bluetooth_manager.h"
#include "notifications_handler.h"
#include "proto_helper.h"
#include "utils.h"

namespace {

class FlutterBluePlusTizenPlugin : public flutter::Plugin {
 public:
  const static inline std::string channel_name_ = "flutter_blue_plus";

  static inline std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      method_channel_;

  static inline std::shared_ptr<flutter::EventChannel<flutter::EncodableValue>>
      state_channel_;

  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    method_channel_ =
        std::make_shared<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), channel_name_ + "/methods",
            &flutter::StandardMethodCodec::GetInstance());

    state_channel_ =
        std::make_shared<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), channel_name_ + "/state",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin_ = std::make_unique<FlutterBluePlusTizenPlugin>();

    method_channel_->SetMethodCallHandler(
        [plugin_pointer = plugin_.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    state_channel_->SetStreamHandler(
        std::make_unique<flutter_blue_tizen::StateHandler>());

    registrar->AddPlugin(std::move(plugin_));
  }

  flutter_blue_tizen::NotificationsHandler notifications_handler_;

  std::unique_ptr<flutter_blue_tizen::BluetoothManager> bluetooth_manager_;

  FlutterBluePlusTizenPlugin()
      : notifications_handler_(method_channel_),
        bluetooth_manager_(
            std::make_unique<flutter_blue_tizen::BluetoothManager>(
                notifications_handler_)) {}

  virtual ~FlutterBluePlusTizenPlugin() {
    bluetooth_manager_ = nullptr;

    google::protobuf::ShutdownProtobufLibrary();

    auto ret = bt_deinitialize();

    LOG_ERROR("bt_adapter_le_is_discovering %s", get_error_message(ret));
  }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const flutter::EncodableValue& args = *method_call.arguments();

    const std::string& method_name = method_call.method_name();
    if (method_name == "isAvailable") {
      try {
        result->Success(
            flutter::EncodableValue(bluetooth_manager_->IsBLEAvailable()));

      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "setLogLevel") {
      result->Success();

    } else if (method_name == "state") {
      auto state_proto_e = flutter_blue_tizen::ToProtoBluetoothState(
          bluetooth_manager_->GetBluetoothState());

      proto::gen::BluetoothState state_proto;
      state_proto.set_state(state_proto_e);

      result->Success(flutter::EncodableValue(
          flutter_blue_tizen::MessageToVector(state_proto)));

    } else if (method_name == "isOn") {
      result->Success(flutter::EncodableValue(
          bluetooth_manager_->GetBluetoothState() ==
          flutter_blue_tizen::BluetoothManager::BluetoothState::kAdapterOn));

    } else if (method_name == "startScan") {
      proto::gen::ScanSettings scan_settings;
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);

      try {
        scan_settings.ParseFromArray(encoded.data(), encoded.size());

        flutter_blue_tizen::BleScanSettings bleScanSettings =
            flutter_blue_tizen::FromProtoScanSettings(scan_settings);

        bluetooth_manager_->StartBluetoothDeviceScanLE(bleScanSettings);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "stopScan") {
      try {
        bluetooth_manager_->StopBluetoothDeviceScanLE();
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "getConnectedDevices") {
      proto::gen::ConnectedDevicesResponse response;
      auto bluetooth_proto_devices =
          bluetooth_manager_->GetConnectedProtoBluetoothDevices();

      for (auto& device : bluetooth_proto_devices) {
        *response.add_devices() = std::move(device);
      }

      result->Success(flutter::EncodableValue(
          flutter_blue_tizen::MessageToVector(response)));

    } else if (method_name == "connect") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ConnectRequest connect_request;

      try {
        connect_request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->Connect(connect_request);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "disconnect") {
      std::string device_id = std::get<std::string>(args);

      try {
        bluetooth_manager_->Disconnect(device_id);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "deviceState") {
      std::string device_id = std::get<std::string>(args);

      try {
        const auto& device = bluetooth_manager_->LocateDevice(device_id);

        proto::gen::DeviceStateResponse device_state_response;
        device_state_response.set_remote_id(device.address());
        device_state_response.set_state(
            flutter_blue_tizen::ToProtoDeviceState(device.GetState()));

        result->Success(flutter::EncodableValue(
            flutter_blue_tizen::MessageToVector(device_state_response)));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "discoverServices") {
      std::string device_id = std::get<std::string>(args);

      try {
        auto& device = bluetooth_manager_->LocateDevice(device_id);

        // Return early to discover services asynchronously.
        result->Success();

        device.DiscoverServices();
        auto services = device.GetServices();
        notifications_handler_.NotifyUIThread(
            "DiscoverServicesResult",
            flutter_blue_tizen::GetProtoServiceDiscoveryResult(device,
                                                               services));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "services") {
      std::string device_id = std::get<std::string>(args);

      try {
        auto& device = bluetooth_manager_->LocateDevice(device_id);

        auto proto_services =
            flutter_blue_tizen::GetProtoServiceDiscoveryResult(
                device, device.GetServices());
        result->Success(flutter::EncodableValue(
            flutter_blue_tizen::MessageToVector(proto_services)));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "readCharacteristic") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ReadCharacteristicRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->ReadCharacteristic(request);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "readDescriptor") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ReadDescriptorRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->ReadDescriptor(request);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "writeCharacteristic") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::WriteCharacteristicRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->WriteCharacteristic(request);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "writeDescriptor") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::WriteDescriptorRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->WriteDescriptor(request);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "setNotification") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::SetNotificationRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->SetNotification(request);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "mtu") {
      std::string device_id = std::get<std::string>(args);

      try {
        proto::gen::MtuSizeResponse response;
        response.set_remote_id(device_id);
        response.set_mtu(bluetooth_manager_->GetMtu(device_id));
        result->Success(flutter::EncodableValue(
            flutter_blue_tizen::MessageToVector(response)));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "requestMtu") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::MtuSizeRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->RequestMtu(request);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "readRssi") {
      std::string device_id = std::get<std::string>(args);

      try {
        bluetooth_manager_->ReadRssi(device_id);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_name == "pair") {
      std::string device_id = std::get<std::string>(args);

      try {
        bluetooth_manager_->Pair(device_id);
        result->Success();
      } catch (const std::exception& e) {
        result->Error(e.what());
      }
    } else if (method_name == "getBondedDevices") {
      proto::gen::ConnectedDevicesResponse response;
      for (auto device : bluetooth_manager_->GetBondedDevices()) {
        *response.add_devices() = flutter_blue_tizen::ToProtoDevice(*device);
      }

      result->Success(flutter::EncodableValue(
          flutter_blue_tizen::MessageToVector(response)));
    } else {
      result->NotImplemented();
    }
  }
};

}  // namespace

void FlutterBluePlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterBluePlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
