#include <BluetoothDeviceController.h>
#include <BluetoothManager.h>
#include <GATT/BluetoothService.h>
#include <Logger.h>
#include <NotificationsHandler.h>

#include <mutex>
#include <unordered_set>

namespace btu {
using btlog::Logger;
using btlog::LogLevel;
using btu::BTException;
using btu::NotificationsHandler;
using State = BluetoothDeviceController::State;

BluetoothDeviceController::BluetoothDeviceController(
    const std::string& address,
    NotificationsHandler& notificationsHandler) noexcept
    : BluetoothDeviceController(address.c_str(), notificationsHandler) {
  std::scoped_lock lock(_activeDevices.mut);
  _activeDevices.var.insert({address, this});
}

BluetoothDeviceController::BluetoothDeviceController(
    const char* address, NotificationsHandler& notificationsHandler) noexcept
    : _address(address), _notificationsHandler(notificationsHandler) {}

BluetoothDeviceController::~BluetoothDeviceController() noexcept {
  std::scoped_lock lock(_activeDevices.mut);
  _activeDevices.var.erase(cAddress());
  disconnect();

  Logger::log(LogLevel::DEBUG, "reporting destroy!");
}

const std::string& BluetoothDeviceController::cAddress() const noexcept {
  return _address;
}
State BluetoothDeviceController::state() const noexcept {
  if (isConnecting ^ isDisconnecting) {
    return (isConnecting ? State::CONNECTING : State::DISCONNECTING);
  } else {
    bool isConnected = false;
    int res = bt_device_is_profile_connected(_address.c_str(), BT_PROFILE_GATT,
                                             &isConnected);
    Logger::showResultError("bt_device_is_profile_connected", res);
    return (isConnected ? State::CONNECTED : State::DISCONNECTED);
  }
}
std::vector<proto::gen::BluetoothDevice>& BluetoothDeviceController::protoBluetoothDevices() noexcept {
  return _protoBluetoothDevices;
}
std::vector<proto::gen::BluetoothDevice> const& BluetoothDeviceController::cProtoBluetoothDevices() const noexcept {
  return _protoBluetoothDevices;
}
void BluetoothDeviceController::connect(bool autoConnect){
  std::unique_lock lock(operationM);
  autoConnect = false;  // TODO - fix. API fails when autoconnect==true
  Logger::log(LogLevel::DEBUG, "connecting...");
  if (state() == State::DISCONNECTED) {
    isConnecting = true;
    int res = bt_gatt_connect(_address.c_str(), autoConnect);
    Logger::showResultError("bt_gatt_connect", res);
  } else {
    std::string mes =
        "trying to connect to a device with state!=DISCONNECTED " +
        std::to_string((int)state());
    Logger::log(LogLevel::WARNING, mes);
    throw BTException(mes);
  }
}
void BluetoothDeviceController::disconnect(){
  std::scoped_lock lock(operationM);
  auto st = state();
  if (st == State::CONNECTED) {
    Logger::log(LogLevel::DEBUG, "explicit disconnect call");
    _services.clear();
    isDisconnecting = true;
    int res = bt_gatt_disconnect(_address.c_str());
    Logger::showResultError("bt_gatt_disconnect", res);
  }
}

 bt_gatt_client_h BluetoothDeviceController::getGattClient(std::string const& address){
  std::scoped_lock lock(gatt_clients.mut);

  auto it = gatt_clients.var.find(address);
  bt_gatt_client_h client = nullptr;

  if (it == gatt_clients.var.end()) {
    int res = bt_gatt_client_create(address.c_str(), &client);
    Logger::showResultError("bt_gatt_client_create", res);
    if ((res == BT_ERROR_NONE || res == BT_ERROR_ALREADY_DONE) && client) {
      gatt_clients.var.emplace(address, client);
      Logger::log(LogLevel::DEBUG, "creating new gatt client for " + address);
    } else
      throw BTException(res, "bt_gatt_client_create");
  } else {
    client = it->second;
    Logger::log(LogLevel::DEBUG,
                "gatt client already exists. Returning for " + address);
  }
  return client;
}
void BluetoothDeviceController::destroyGattClientIfExists(
    std::string const& address) noexcept {
  std::scoped_lock lock(gatt_clients.mut);
  auto it = gatt_clients.var.find(address);
  if (it != gatt_clients.var.end()) {
    Logger::log(LogLevel::DEBUG, "destroying gatt client for " + address);
    auto res = bt_gatt_client_destroy(it->second);
    if (!res) gatt_clients.var.erase(address);
    Logger::showResultError("bt_gatt_client_destroy", res);
  }
}

void BluetoothDeviceController::discoverServices(){
  std::scoped_lock lock(operationM);

  struct Scope {
    BluetoothDeviceController& device;
    std::vector<std::unique_ptr<btGatt::PrimaryService>>& services;
  };
  _services.clear();
  Scope scope{*this, _services};
  int res = bt_gatt_client_foreach_services(
      getGattClient(_address),
      [](int total, int index, bt_gatt_h service_handle,
         void* scope_ptr) -> bool {
        auto& scope = *static_cast<Scope*>(scope_ptr);

        scope.services.emplace_back(std::make_unique<btGatt::PrimaryService>(
            service_handle, scope.device));

        return true;
      },
      &scope);

  Logger::showResultError("bt_gatt_client_foreach_services", res);
}

std::vector<btGatt::PrimaryService*> BluetoothDeviceController::getServices() noexcept{
  auto result = std::vector<btGatt::PrimaryService*>();
  for (auto& s : _services) result.emplace_back(s.get());
  return result;
}

btGatt::PrimaryService* BluetoothDeviceController::getService(std::string const& uuid) noexcept{
  for (auto& s : _services) {
    if (s->UUID() == uuid) return s.get();
  }
  return nullptr;
}
NotificationsHandler const& BluetoothDeviceController::cNotificationsHandler() const noexcept{
  return _notificationsHandler;
}

void BluetoothDeviceController::connectionStateCallback(
    int res, bool connected, const char* remote_address,
    void* user_data) noexcept{
  std::string err = get_error_message(res);
  Logger::log(LogLevel::DEBUG, "callback called for device " +
                                   std::string(remote_address) +
                                   " with state=" + std::to_string(connected) +
                                   " and result=" + err);
  Logger::showResultError("bt_gatt_connection_state_changed_cb", res);

  auto& bluetoothManager = *static_cast<BluetoothManager*>(user_data);
  std::scoped_lock lock(bluetoothManager.bluetoothDevices().mut);
  auto ptr = bluetoothManager.bluetoothDevices().var.find(remote_address);

  if (ptr != bluetoothManager.bluetoothDevices().var.end()) {
    auto device = ptr->second;
    std::scoped_lock devLock(device->operationM);
    device->isConnecting = false;
    device->isDisconnecting = false;
    if (!connected) {
      device->_services.clear();
    } else if (!res) {
      getGattClient(device->cAddress());  // this function creates gatt client
                                          // if it does not exists.
    }
    device->notifyDeviceState();
    Logger::log(LogLevel::DEBUG, "sending state notification = " +
                                     std::to_string((int)device->state()));
  }
  if (!connected) {
    destroyGattClientIfExists(remote_address);
  }
}
u_int32_t BluetoothDeviceController::getMtu() const{
  u_int32_t mtu = -1;
  auto res = bt_gatt_client_get_att_mtu(getGattClient(_address), &mtu);
  Logger::showResultError("bt_gatt_client_get_att_mtu", res);
  if (res) throw BTException(res, "could not get mtu of the device!");
  Logger::log(LogLevel::DEBUG, "fetched mtu size successfully");
  return mtu;
}
void BluetoothDeviceController::requestMtu(u_int32_t mtu,
                                           const requestMtuCallback& callback){
  struct Scope {
    const std::string device_address;
    requestMtuCallback callback;
  };
  auto scope = new Scope{_address, callback};

  auto res = bt_gatt_client_set_att_mtu_changed_cb(
      getGattClient(_address),
      [](bt_gatt_client_h client, const bt_gatt_client_att_mtu_info_s* mtu_info,
         void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);

        Logger::log(LogLevel::DEBUG, "called NATIVE request mtu cb");
        std::scoped_lock lock(_activeDevices.mut);
        auto it = _activeDevices.var.find(scope->device_address);
        if (it != _activeDevices.var.end()) {
          scope->callback(mtu_info->status == 0, *it->second);
        }
        delete scope;
      },
      scope);

  Logger::showResultError("bt_gatt_client_set_att_mtu_changed_cb", res);
  if (res) throw BTException(res, "bt_gatt_client_set_att_mtu_changed_cb");

  res = bt_gatt_client_request_att_mtu_change(getGattClient(_address), mtu);

  Logger::showResultError("bt_gatt_client_request_att_mtu_change", res);
  if (res) throw BTException(res, "bt_gatt_client_request_att_mtu_change");
}
void BluetoothDeviceController::notifyDeviceState() const{
  proto::gen::DeviceStateResponse devState;
  devState.set_remote_id(cAddress());
  devState.set_state(localToProtoDeviceState(state()));
  _notificationsHandler.notifyUIThread("DeviceState", devState);
}

proto::gen::DeviceStateResponse_BluetoothDeviceState BluetoothDeviceController::localToProtoDeviceState(
    BluetoothDeviceController::State const& s){
  using State = btu::BluetoothDeviceController::State;
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
}  // namespace btu
