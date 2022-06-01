#include "bluetooth_device_controller.h"

#include <mutex>
#include <unordered_set>

#include "GATT/bluetooth_service.h"
#include "bluetooth_manager.h"
#include "log.h"
#include "notifications_handler.h"

namespace flutter_blue_tizen {

using State = BluetoothDeviceController::State;

BluetoothDeviceController::BluetoothDeviceController(
    const std::string& address,
    NotificationsHandler& notifications_handler) noexcept
    : BluetoothDeviceController(address.c_str(), notifications_handler) {
  std::scoped_lock lock(active_devices_.mutex_);
  active_devices_.var_.insert({address, this});
}

BluetoothDeviceController::BluetoothDeviceController(
    const char* address, NotificationsHandler& notifications_handler) noexcept
    : address_(address), notifications_handler_(notifications_handler) {}

BluetoothDeviceController::~BluetoothDeviceController() noexcept {
  std::scoped_lock lock(active_devices_.mutex_);
  active_devices_.var_.erase(cAddress());
  Disconnect();
}

const std::string& BluetoothDeviceController::cAddress() const noexcept {
  return address_;
}

State BluetoothDeviceController::GetState() const noexcept {
  if (is_connecting_ ^ is_disconnecting_) {
    return (is_connecting_ ? State::kConnecting : State::kDisconnecting);
  } else {
    bool is_connected = false;
    int ret = bt_device_is_profile_connected(address_.c_str(), BT_PROFILE_GATT,
                                             &is_connected);
    LOG_ERROR("bt_device_is_profile_connected", get_error_message(ret));
    return (is_connected ? State::kConnected : State::kDisconnected);
  }
}

std::vector<proto::gen::BluetoothDevice>&
BluetoothDeviceController::ProtoBluetoothDevices() noexcept {
  return proto_bluetooth_devices_;
}

const std::vector<proto::gen::BluetoothDevice>&
BluetoothDeviceController::cProtoBluetoothDevices() const noexcept {
  return proto_bluetooth_devices_;
}

void BluetoothDeviceController::Connect(bool auto_connect) {
  std::unique_lock lock(operation_mutex_);
  auto_connect = false;  // TODO - fix. API fails when autoconnect==true
  if (GetState() == State::kDisconnected) {
    is_connecting_ = true;
    int ret = bt_gatt_connect(address_.c_str(), auto_connect);
    LOG_ERROR("bt_gatt_connect", get_error_message(ret));
  } else {
    std::string message =
        "trying to connect to a device with State!=DISCONNECTED " +
        std::to_string((int)GetState());
    throw BtException(message);
  }
}

void BluetoothDeviceController::Disconnect() {
  std::scoped_lock lock(operation_mutex_);
  auto st = GetState();
  if (st == State::kConnected) {
    services_.clear();
    is_disconnecting_ = true;
    int ret = bt_gatt_disconnect(address_.c_str());
    LOG_ERROR("bt_gatt_disconnect", get_error_message(ret));
  }
}

bt_gatt_client_h BluetoothDeviceController::GetGattClient(
    const std::string& address) {
  std::scoped_lock lock(gatt_clients_.mutex_);

  auto it = gatt_clients_.var_.find(address);
  bt_gatt_client_h client = nullptr;

  if (it == gatt_clients_.var_.end()) {
    int ret = bt_gatt_client_create(address.c_str(), &client);
    LOG_ERROR("bt_gatt_client_create", get_error_message(ret));

    if ((ret == BT_ERROR_NONE || ret == BT_ERROR_ALREADY_DONE) && client) {
      gatt_clients_.var_.emplace(address, client);
    } else {
      throw BtException(ret, "bt_gatt_client_create");
    }
  } else {
    client = it->second;
  }
  return client;
}

void BluetoothDeviceController::DestroyGattClientIfExists(
    const std::string& address) noexcept {
  std::scoped_lock lock(gatt_clients_.mutex_);
  auto it = gatt_clients_.var_.find(address);
  if (it != gatt_clients_.var_.end()) {
    auto ret = bt_gatt_client_destroy(it->second);
    if (!ret) gatt_clients_.var_.erase(address);
    LOG_ERROR("bt_gatt_client_destroy", get_error_message(ret));
  }
}

void BluetoothDeviceController::DiscoverServices() {
  std::scoped_lock lock(operation_mutex_);

  struct Scope {
    BluetoothDeviceController& device;
    std::vector<std::unique_ptr<btGatt::PrimaryService>>& services;
  };
  services_.clear();
  Scope scope{*this, services_};
  int ret = bt_gatt_client_foreach_services(
      GetGattClient(address_),
      [](int total, int index, bt_gatt_h service_handle,
         void* scope_ptr) -> bool {
        auto& scope = *static_cast<Scope*>(scope_ptr);

        scope.services.emplace_back(
            std::make_unique<btGatt::PrimaryService>(service_handle));

        return true;
      },
      &scope);

  LOG_ERROR("bt_gatt_client_foreach_services", get_error_message(ret));
}

std::vector<btGatt::PrimaryService*>
BluetoothDeviceController::GetServices() noexcept {
  auto result = std::vector<btGatt::PrimaryService*>();
  for (auto& service : services_) result.emplace_back(service.get());
  return result;
}

btGatt::PrimaryService* BluetoothDeviceController::GetService(
    const std::string& uuid) noexcept {
  for (auto& service : services_) {
    if (service->Uuid() == uuid) return service.get();
  }
  return nullptr;
}

const NotificationsHandler& BluetoothDeviceController::cNotificationsHandler()
    const noexcept {
  return notifications_handler_;
}

void BluetoothDeviceController::ConnectionStateCallback(
    int ret, bool connected, const char* remote_address,
    void* user_data) noexcept {
  LOG_ERROR("bt_gatt_connection_state_changed_cb", get_error_message(ret));

  auto& bluetooth_manager = *static_cast<BluetoothManager*>(user_data);
  std::scoped_lock lock(bluetooth_manager.bluetoothDevices().mutex_);
  auto it = bluetooth_manager.bluetoothDevices().var_.find(remote_address);

  if (it != bluetooth_manager.bluetoothDevices().var_.end()) {
    auto device = it->second;
    std::scoped_lock devLock(device->operation_mutex_);
    device->is_connecting_ = false;
    device->is_disconnecting_ = false;
    if (!connected) {
      device->services_.clear();
    } else if (!ret) {
      GetGattClient(device->cAddress());  // this function creates gatt client
                                          // if it does not exists.
    }
    device->NotifyDeviceState();
  }
  if (!connected) {
    DestroyGattClientIfExists(remote_address);
  }
}

uint32_t BluetoothDeviceController::GetMtu() const {
  uint32_t mtu = -1;
  auto ret = bt_gatt_client_get_att_mtu(GetGattClient(address_), &mtu);
  LOG_ERROR("bt_gatt_client_get_att_mtu", get_error_message(ret));

  if (ret) throw BtException(ret, "could not get mtu of the device!");
  return mtu;
}

void BluetoothDeviceController::RequestMtu(uint32_t mtu,
                                           const requestMtuCallback& callback) {
  struct Scope {
    const std::string device_address;
    requestMtuCallback callback;
  };
  auto scope = new Scope{address_, callback};

  auto ret = bt_gatt_client_set_att_mtu_changed_cb(
      GetGattClient(address_),
      [](bt_gatt_client_h client, const bt_gatt_client_att_mtu_info_s* mtu_info,
         void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_devices_.mutex_);
        auto it = active_devices_.var_.find(scope->device_address);
        if (it != active_devices_.var_.end())
          scope->callback(mtu_info->status == 0, *it->second);
        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_set_att_mtu_changed_cb", get_error_message(ret));

  if (ret) throw BtException(ret, "bt_gatt_client_set_att_mtu_changed_cb");

  ret = bt_gatt_client_request_att_mtu_change(GetGattClient(address_), mtu);

  LOG_ERROR("bt_gatt_client_request_att_mtu_change", get_error_message(ret));

  if (ret) throw BtException(ret, "bt_gatt_client_request_att_mtu_change");
}

void BluetoothDeviceController::NotifyDeviceState() const {
  proto::gen::DeviceStateResponse device_state;
  device_state.set_remote_id(cAddress());
  device_state.set_state(LocalToProtoDeviceState(GetState()));
  notifications_handler_.NotifyUIThread("DeviceState", device_state);
}

proto::gen::DeviceStateResponse_BluetoothDeviceState
BluetoothDeviceController::LocalToProtoDeviceState(
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