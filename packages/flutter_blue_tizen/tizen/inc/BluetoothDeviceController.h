#ifndef BLUETOOTH_DEVICE_CONTROLLER_H
#define BLUETOOTH_DEVICE_CONTROLLER_H
#include <flutterblue.pb.h>

#include <condition_variable>

#include <bluetooth.h>

#include <Utils.h>

namespace btGatt{
    class PrimaryService;
    class SecondaryService;
}

namespace btu{
    class NotificationsHandler;
    class BluetoothService;
    class BluetoothDeviceController{
    public:
        enum class State;
    private:
        /**
         * @brief all attributes are depentent on this mutex
         * 
         */
        std::mutex operationM;

        std::vector<proto::gen::BluetoothDevice> _protoBluetoothDevices;

        std::vector<std::unique_ptr<btGatt::PrimaryService>> _services;

        std::string _address;
        std::atomic<bool> isConnecting=false;
        std::atomic<bool> isDisconnecting=false;
        std::condition_variable connection_state_cv;

        NotificationsHandler& _notificationsHandler;

        using requestMtuCallback=std::function<void(bool, const BluetoothDeviceController&)>;


        static inline SafeType<std::map<std::string, BluetoothDeviceController*>> _activeDevices;
        
        static inline SafeType<std::unordered_map<std::string, bt_gatt_client_h>> gatt_clients;

    public:
        enum class State{
            CONNECTED,
            CONNECTING,
            DISCONNECTED,
            DISCONNECTING,
        };

        BluetoothDeviceController(const std::string& address, NotificationsHandler& notificationsHandler) noexcept;
        BluetoothDeviceController(const char* address, NotificationsHandler& notificationsHandler) noexcept;
        ~BluetoothDeviceController() noexcept;

        BluetoothDeviceController()=delete;
        BluetoothDeviceController(const BluetoothDeviceController& address)=delete;
        
        auto cAddress() const noexcept -> const std::string&;
        auto state() const noexcept -> State;
        auto protoBluetoothDevices() noexcept -> std::vector<proto::gen::BluetoothDevice>&;
        auto cProtoBluetoothDevices() const noexcept -> const std::vector<proto::gen::BluetoothDevice>&;

        auto connect(bool autoConnect) -> void;
        auto disconnect() -> void;

        static auto connectionStateCallback(int result, bool connected, const char* remote_address, void* user_data) noexcept -> void;
        static auto getGattClient(const std::string& address) -> bt_gatt_client_h;
        static auto destroyGattClientIfExists(const std::string& address) noexcept -> void;
        static auto localToProtoDeviceState(const BluetoothDeviceController::State& s) -> proto::gen::DeviceStateResponse_BluetoothDeviceState;

        auto discoverServices() -> void;

        auto getServices() noexcept -> std::vector<btGatt::PrimaryService*>;

        auto getService(const std::string& uuid) noexcept -> btGatt::PrimaryService*;

        auto getMtu() const -> u_int32_t;
        
        auto requestMtu(u_int32_t mtu, const requestMtuCallback& callback) -> void;
        
        auto notifyDeviceState() const -> void;
        auto cNotificationsHandler() const noexcept -> const NotificationsHandler&;
    };
};
#endif //BLUETOOTH_DEVICE_CONTROLLER_H
