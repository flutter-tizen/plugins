#include <GATT/bluetooth_service.h>
#include <bluetooth_device_controller.h>
#include <bluetooth_manager.h>
#include <log.h>
#include <notifications_handler.h>

#include <mutex>
#include <unordered_set>

namespace flutter_blue_tizen {

using State = BluetoothDeviceController::State;

BluetoothDeviceController::BluetoothDeviceController(
    const std::string& address,
    NotificationsHandler& notificationsHandler) noexcept
    : BluetoothDeviceController(address.c_str(), notificationsHandler) {
  std::scoped_lock lock(activeDevices_.mutex_);
  activeDevices_.var_.insert({address, this});
}

BluetoothDeviceController::BluetoothDeviceController(
    const char* address, NotificationsHandler& notificationsHandler) noexcept
    : address_(address), notifications_handler_(notificationsHandler) {}

BluetoothDeviceController::~BluetoothDeviceController() noexcept {
  std::scoped_lock lock(activeDevices_.mutex_);
  activeDevices_.var_.erase(cAddress());
  disconnect();
}

const std::string& BluetoothDeviceController::cAddress() const noexcept {
  return address_;
}

State BluetoothDeviceController::state() const noexcept {
  if (is_connecting_ ^ is_disconnecting_) {
    return (is_connecting_ ? State::CONNECTING : State::DISCONNECTING);
  } else {
    bool isConnected = false;
    int res = bt_device_is_profile_connected(address_.c_str(), BT_PROFILE_GATT,
                                             &isConnected);
    LOG_ERROR("bt_device_is_profile_connected", get_error_message(res));
    return (isConnected ? State::CONNECTED : State::DISCONNECTED);
  }
}

std::vector<proto::gen::BluetoothDevice>&
BluetoothDeviceController::protoBluetoothDevices() noexcept {
  return proto_bluetoothDevices_;
}

std::vector<proto::gen::BluetoothDevice> const&
BluetoothDeviceController::cProtoBluetoothDevices() const noexcept {
  return proto_bluetoothDevices_;
}

void BluetoothDeviceController::connect(bool autoConnect) {
  std::unique_lock lock(operation_mutex_);
  autoConnect = false;  // TODO - fix. API fails when autoconnect==true
  if (state() == State::DISCONNECTED) {
    is_connecting_ = true;
    int res = bt_gatt_connect(address_.c_str(), autoConnect);
    LOG_ERROR("bt_gatt_connect", get_error_message(res));
  } else {
    std::string mes =
        "trying to connect to a device with state!=DISCONNECTED " +
        std::to_string((int)state());
    throw BTException(mes);
  }
}

void BluetoothDeviceController::disconnect() {
  std::scoped_lock lock(operation_mutex_);
  auto st = state();
  if (st == State::CONNECTED) {
    services_.clear();
    is_disconnecting_ = true;
    int res = bt_gatt_disconnect(address_.c_str());
    LOG_ERROR("bt_gatt_disconnect", get_error_message(res));
  }
}

bt_gatt_client_h BluetoothDeviceController::getGattClient(
    std::string const& address) {
  std::scoped_lock lock(gatt_clients_.mutex_);

  auto it = gatt_clients_.var_.find(address);
  bt_gatt_client_h client = nullptr;

  if (it == gatt_clients_.var_.end()) {
    int res = bt_gatt_client_create(address.c_str(), &client);
    LOG_ERROR("bt_gatt_client_create", get_error_message(res));

    if ((res == BT_ERROR_NONE || res == BT_ERROR_ALREADY_DONE) && client) {
      gatt_clients_.var_.emplace(address, client);
    } else
      throw BTException(res, "bt_gatt_client_create");
  } else {
    client = it->second;
  }
  return client;
}

void BluetoothDeviceController::destroyGattClientIfExists(
    std::string const& address) noexcept {
  std::scoped_lock lock(gatt_clients_.mutex_);
  auto it = gatt_clients_.var_.find(address);
  if (it != gatt_clients_.var_.end()) {
    auto res = bt_gatt_client_destroy(it->second);
    if (!res) gatt_clients_.var_.erase(address);
    LOG_ERROR("bt_gatt_client_destroy", get_error_message(res));
  }
}

void BluetoothDeviceController::discoverServices() {
  std::scoped_lock lock(operation_mutex_);

  struct Scope {
    BluetoothDeviceController& device;
    std::vector<std::unique_ptr<btGatt::PrimaryService>>& services;
  };
  services_.clear();
  Scope scope{*this, services_};
  int res = bt_gatt_client_foreach_services(
      getGattClient(address_),
      [](int total, int index, bt_gatt_h service_handle,
         void* scope_ptr) -> bool {
        auto& scope = *static_cast<Scope*>(scope_ptr);

        scope.services.emplace_back(std::make_unique<btGatt::PrimaryService>(
            service_handle, scope.device));

        return true;
      },
      &scope);

  LOG_ERROR("bt_gatt_client_foreach_services", get_error_message(res));
}

std::vector<btGatt::PrimaryService*>
BluetoothDeviceController::getServices() noexcept {
  auto result = std::vector<btGatt::PrimaryService*>();
  for (auto& s : services_) result.emplace_back(s.get());
  return result;
}

btGatt::PrimaryService* BluetoothDeviceController::getService(
    std::string const& uuid) noexcept {
  for (auto& s : services_) {
    if (s->UUID() == uuid) return s.get();
  }
  return nullptr;
}

NotificationsHandler const& BluetoothDeviceController::cNotificationsHandler()
    const noexcept {
  return notifications_handler_;
}

void BluetoothDeviceController::connectionStateCallback(
    int res, bool connected, const char* remote_address,
    void* user_data) noexcept {
  LOG_ERROR("bt_gatt_connection_state_changed_cb", get_error_message(res));

  auto& bluetoothManager = *static_cast<BluetoothManager*>(user_data);
  std::scoped_lock lock(bluetoothManager.bluetoothDevices().mutex_);
  auto it = bluetoothManager.bluetoothDevices().var_.find(remote_address);

  if (it != bluetoothManager.bluetoothDevices().var_.end()) {
    auto device = it->second;
    std::scoped_lock devLock(device->operation_mutex_);
    device->is_connecting_ = false;
    device->is_disconnecting_ = false;
    if (!connected) {
      device->services_.clear();
    } else if (!res) {
      getGattClient(device->cAddress());  // this function creates gatt client
                                          // if it does not exists.
    }
    device->notifyDeviceState();
  }
  if (!connected) {
    destroyGattClientIfExists(remote_address);
  }
}

u_int32_t BluetoothDeviceController::getMtu() const {
  u_int32_t mtu = -1;
  auto res = bt_gatt_client_get_att_mtu(getGattClient(address_), &mtu);
  LOG_ERROR("bt_gatt_client_get_att_mtu", get_error_message(res));

  if (res) throw BTException(res, "could not get mtu of the device!");
  return mtu;
}

void BluetoothDeviceController::requestMtu(u_int32_t mtu,
                                           const requestMtuCallback& callback) {
  struct Scope {
    const std::string device_address;
    requestMtuCallback callback;
  };
  auto scope = new Scope{address_, callback};

  auto res = bt_gatt_client_set_att_mtu_changed_cb(
      getGattClient(address_),
      [](bt_gatt_client_h client, const bt_gatt_client_att_mtu_info_s* mtu_info,
         void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(activeDevices_.mutex_);
        auto it = activeDevices_.var_.find(scope->device_address);
        if (it != activeDevices_.var_.end()) {
          scope->callback(mtu_info->status == 0, *it->second);
        }
        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_set_att_mtu_changed_cb", get_error_message(res));

  if (res) throw BTException(res, "bt_gatt_client_set_att_mtu_changed_cb");

  res = bt_gatt_client_request_att_mtu_change(getGattClient(address_), mtu);

  LOG_ERROR("bt_gatt_client_request_att_mtu_change", get_error_message(res));

  if (res) throw BTException(res, "bt_gatt_client_request_att_mtu_change");
}

void BluetoothDeviceController::notifyDeviceState() const {
  proto::gen::DeviceStateResponse devState;
  devState.set_remote_id(cAddress());
  devState.set_state(localToProtoDeviceState(state()));
  notifications_handler_.notifyUIThread("DeviceState", devState);
}

proto::gen::DeviceStateResponse_BluetoothDeviceState
BluetoothDeviceController::localToProtoDeviceState(
    BluetoothDeviceController::State const& s) {
  using State = BluetoothDeviceController::State;
  switch (s) {
    case State::CONNECTED:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_CONNECTED;
    case State::CONNECTING:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_CONNECTING;
    case State::DISCONNECTED:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
    case State::DISCONNECTING:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTING;
    default:
      return proto::gen::DeviceStateResponse_BluetoothDeviceState_DISCONNECTED;
  }
}

}  // namespace flutter_blue_tizen