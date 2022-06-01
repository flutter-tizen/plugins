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

namespace {

class FlutterBlueTizenPlugin : public flutter::Plugin {
 public:
  const static inline std::string channel_name_ =
      "plugins.pauldemarco.com/flutter_blue/";

  static inline std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      method_channel_;

  static inline std::shared_ptr<flutter::EventChannel<flutter::EncodableValue>>
      state_channel_;

  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    method_channel_ =
        std::make_shared<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), channel_name_ + "methods",
            &flutter::StandardMethodCodec::GetInstance());

    state_channel_ =
        std::make_shared<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), channel_name_ + "state",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin_ = std::make_unique<FlutterBlueTizenPlugin>();

    method_channel_->SetMethodCallHandler(
        [plugin_pointer = plugin_.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    state_channel_->SetStreamHandler(
        std::make_unique<flutter_blue_tizen::StateHandler>());  // todo

    registrar->AddPlugin(std::move(plugin_));
  }

  flutter_blue_tizen::NotificationsHandler notifications_handler_;

  std::unique_ptr<flutter_blue_tizen::BluetoothManager> bluetooth_manager_;

  FlutterBlueTizenPlugin()
      : notifications_handler_(method_channel_),
        bluetooth_manager_(
            std::make_unique<flutter_blue_tizen::BluetoothManager>(
                notifications_handler_)) {}

  virtual ~FlutterBlueTizenPlugin() {
    bluetooth_manager_ = nullptr;

    google::protobuf::ShutdownProtobufLibrary();

    auto ret = bt_deinitialize();

    LOG_ERROR("bt_adapter_le_is_discovering", get_error_message(ret));
  }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const flutter::EncodableValue& args = *method_call.arguments();

    if (method_call.method_name() == "isAvailable") {
      try {
        result->Success(
            flutter::EncodableValue(bluetooth_manager_->IsBLEAvailable()));

      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "setLogLevel") {
      result->Success(flutter::EncodableValue(NULL));

    } else if (method_call.method_name() == "state") {
      result->Success(
          flutter::EncodableValue(flutter_blue_tizen::MessageToVector(
              bluetooth_manager_->BluetoothState())));

    } else if (method_call.method_name() == "isOn") {
      result->Success(flutter::EncodableValue(
          (bluetooth_manager_->BluetoothState().state() ==
           proto::gen::BluetoothState_State::BluetoothState_State_ON)));

    } else if (method_call.method_name() == "startScan") {
      proto::gen::ScanSettings scan_settings;
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);

      try {
        scan_settings.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->StartBluetoothDeviceScanLE(scan_settings);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "stopScan") {
      try {
        bluetooth_manager_->StopBluetoothDeviceScanLE();
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "getConnectedDevices") {
      proto::gen::ConnectedDevicesResponse response;
      auto bluetooth_proto_devices =
          bluetooth_manager_->GetConnectedProtoBluetoothDevices();

      for (auto& device : bluetooth_proto_devices) {
        *response.add_devices() = std::move(device);
      }

      result->Success(flutter::EncodableValue(
          flutter_blue_tizen::MessageToVector(response)));

    } else if (method_call.method_name() == "connect") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ConnectRequest connect_request;

      try {
        connect_request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->Connect(connect_request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "disconnect") {
      std::string device_id = std::get<std::string>(args);
      try {
        bluetooth_manager_->Disconnect(device_id);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "deviceState") {
      std::string device_id = std::get<std::string>(args);
      std::scoped_lock lock(bluetooth_manager_->bluetoothDevices().mutex_);

      auto device = bluetooth_manager_->LocateDevice(device_id);

      proto::gen::DeviceStateResponse device_state_response;
      device_state_response.set_remote_id(device->cAddress());
      device_state_response.set_state(
          flutter_blue_tizen::BluetoothDeviceController::
              LocalToProtoDeviceState(device->GetState()));

      result->Success(flutter::EncodableValue(
          flutter_blue_tizen::MessageToVector(device_state_response)));

    } else if (method_call.method_name() == "discoverServices") {
      std::string device_id = std::get<std::string>(args);
      std::scoped_lock lock(bluetooth_manager_->bluetoothDevices().mutex_);

      auto device = bluetooth_manager_->LocateDevice(device_id);

      result->Success(flutter::EncodableValue(NULL));

      device->DiscoverServices();
      auto services = device->GetServices();
      notifications_handler_.NotifyUIThread(
          "DiscoverServicesResult",
          flutter_blue_tizen::GetProtoServiceDiscoveryResult(*device,
                                                             services));

    } else if (method_call.method_name() == "services") {
      std::string device_id = std::get<std::string>(args);
      std::scoped_lock lock(bluetooth_manager_->bluetoothDevices().mutex_);

      auto device = bluetooth_manager_->LocateDevice(device_id);

      auto proto_services = flutter_blue_tizen::GetProtoServiceDiscoveryResult(
          *device, device->GetServices());
      result->Success(flutter::EncodableValue(
          flutter_blue_tizen::MessageToVector(proto_services)));

    } else if (method_call.method_name() == "readCharacteristic") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ReadCharacteristicRequest request;
      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->ReadCharacteristic(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "readDescriptor") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ReadDescriptorRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->ReadDescriptor(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "writeCharacteristic") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::WriteCharacteristicRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->WriteCharacteristic(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "writeDescriptor") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::WriteDescriptorRequest request;
      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->WriteDescriptor(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "setNotification") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::SetNotificationRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->SetNotification(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "mtu") {
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

    } else if (method_call.method_name() == "requestMtu") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::MtuSizeRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetooth_manager_->RequestMtu(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else {
      result->NotImplemented();
    }
  }
};

}  // namespace

void FlutterBlueTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterBlueTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
