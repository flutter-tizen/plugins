#include "flutter_blue_tizen_plugin.h"

#include <app_control.h>
#include <bluetooth.h>
#include <bluetooth_device_controller.h>
#include <bluetooth_manager.h>
#include <flutter/event_channel.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutterblue.pb.h>
#include <log.h>
#include <notifications_handler.h>
#include <state_handler.h>
#include <system_info.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

namespace {

class FlutterBlueTizenPlugin : public flutter::Plugin {
 public:
  const static inline std::string channel_name =
      "plugins.pauldemarco.com/flutter_blue/";

  static inline std::shared_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      methodChannel;

  static inline std::shared_ptr<flutter::EventChannel<flutter::EncodableValue>>
      stateChannel;

  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    methodChannel =
        std::make_shared<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), channel_name + "methods",
            &flutter::StandardMethodCodec::GetInstance());

    stateChannel =
        std::make_shared<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), channel_name + "state",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FlutterBlueTizenPlugin>();

    methodChannel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    stateChannel->SetStreamHandler(
        std::make_unique<flutter_blue_tizen::StateHandler>());  // todo

    registrar->AddPlugin(std::move(plugin));
  }

  flutter_blue_tizen::NotificationsHandler notificationsHandler;

  std::unique_ptr<flutter_blue_tizen::BluetoothManager> bluetoothManager;

  FlutterBlueTizenPlugin()
      : notificationsHandler(methodChannel),
        bluetoothManager(std::make_unique<flutter_blue_tizen::BluetoothManager>(
            notificationsHandler)) {}

  virtual ~FlutterBlueTizenPlugin() {
    bluetoothManager = nullptr;

    google::protobuf::ShutdownProtobufLibrary();

    auto res = bt_deinitialize();

    LOG_ERROR("bt_adapter_le_is_discovering", get_error_message(res));
  }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const flutter::EncodableValue& args = *method_call.arguments();

    if (method_call.method_name() == "isAvailable") {
      try {
        result->Success(
            flutter::EncodableValue(bluetoothManager->IsBLEAvailable()));

      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "setLogLevel") {
      /**
       * @brief plugin should log everything despite the log level.
       *
       */

      result->Success(flutter::EncodableValue(NULL));

    } else if (method_call.method_name() == "state") {
      result->Success(
          flutter::EncodableValue(flutter_blue_tizen::MessageToVector(
              bluetoothManager->BluetoothState())));

    } else if (method_call.method_name() == "isOn") {
      result->Success(flutter::EncodableValue(
          (bluetoothManager->BluetoothState().state() ==
           proto::gen::BluetoothState_State::BluetoothState_State_ON)));

    } else if (method_call.method_name() == "startScan") {
      proto::gen::ScanSettings scanSettings;
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);

      try {
        scanSettings.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->StartBluetoothDeviceScanLE(scanSettings);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "stopScan") {
      try {
        bluetoothManager->StopBluetoothDeviceScanLE();
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "getConnectedDevices") {
      proto::gen::ConnectedDevicesResponse response;
      auto p = bluetoothManager->GetConnectedProtoBluetoothDevices();

      for (auto& dev : p) {
        *response.add_devices() = std::move(dev);
      }

      result->Success(flutter::EncodableValue(
          flutter_blue_tizen::MessageToVector(response)));

    } else if (method_call.method_name() == "connect") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ConnectRequest connectRequest;

      try {
        connectRequest.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->Connect(connectRequest);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "disconnect") {
      std::string deviceID = std::get<std::string>(args);
      try {
        bluetoothManager->Disconnect(deviceID);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "deviceState") {
      std::string deviceID = std::get<std::string>(args);
      std::scoped_lock lock(bluetoothManager->bluetoothDevices().mutex_);
      auto it = bluetoothManager->bluetoothDevices().var_.find(deviceID);

      if (it != bluetoothManager->bluetoothDevices().var_.end()) {
        auto& device = it->second;

        proto::gen::DeviceStateResponse res;
        res.set_remote_id(device->cAddress());
        res.set_state(flutter_blue_tizen::BluetoothDeviceController::
                          LocalToProtoDeviceState(device->GetState()));

        result->Success(
            flutter::EncodableValue(flutter_blue_tizen::MessageToVector(res)));
      } else
        result->Error("device not available");

    } else if (method_call.method_name() == "discoverServices") {
      std::string deviceID = std::get<std::string>(args);
      std::scoped_lock lock(bluetoothManager->bluetoothDevices().mutex_);
      auto it = bluetoothManager->bluetoothDevices().var_.find(deviceID);

      if (it != bluetoothManager->bluetoothDevices().var_.end()) {
        auto& device = it->second;
        result->Success(flutter::EncodableValue(NULL));

        device->DiscoverServices();
        auto services = device->GetServices();
        notificationsHandler.NotifyUIThread(
            "DiscoverServicesResult",
            flutter_blue_tizen::GetProtoServiceDiscoveryResult(*device,
                                                               services));
      } else
        result->Error("device not available");

    } else if (method_call.method_name() == "services") {
      std::string deviceID = std::get<std::string>(args);
      std::scoped_lock lock(bluetoothManager->bluetoothDevices().mutex_);

      auto it = bluetoothManager->bluetoothDevices().var_.find(deviceID);

      if (it != bluetoothManager->bluetoothDevices().var_.end()) {
        auto& device = it->second;

        auto protoServices = flutter_blue_tizen::GetProtoServiceDiscoveryResult(
            *device, device->GetServices());
        result->Success(flutter::EncodableValue(
            flutter_blue_tizen::MessageToVector(protoServices)));
      } else
        result->Error("device not available");

    } else if (method_call.method_name() == "readCharacteristic") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ReadCharacteristicRequest request;
      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->ReadCharacteristic(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "readDescriptor") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::ReadDescriptorRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->ReadDescriptor(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "writeCharacteristic") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::WriteCharacteristicRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->WriteCharacteristic(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "writeDescriptor") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::WriteDescriptorRequest request;
      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->writeDescriptor(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "setNotification") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::SetNotificationRequest request;

      try {
        request.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->SetNotification(request);
        result->Success(flutter::EncodableValue(NULL));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "mtu") {
      std::string deviceID = std::get<std::string>(args);

      try {
        proto::gen::MtuSizeResponse res;
        res.set_remote_id(deviceID);
        res.set_mtu(bluetoothManager->GetMtu(deviceID));
        result->Success(
            flutter::EncodableValue(flutter_blue_tizen::MessageToVector(res)));
      } catch (const std::exception& e) {
        result->Error(e.what());
      }

    } else if (method_call.method_name() == "requestMtu") {
      std::vector<uint8_t> encoded = std::get<std::vector<uint8_t>>(args);
      proto::gen::MtuSizeRequest req;

      try {
        req.ParseFromArray(encoded.data(), encoded.size());
        bluetoothManager->RequestMtu(req);
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
