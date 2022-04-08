#ifndef UTILS_H
#define UTILS_H

#include <flutter/method_channel.h>
#include <flutter/encodable_value.h>
#include <flutterblue.pb.h>

#include <bluetooth.h>
#include <mutex>
#include <exception>

namespace btGatt{
    class PrimaryService;
    class SecondaryService;
}
namespace btu{
    class BluetoothDeviceController;
    using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;

    template<typename T>
    struct SafeType{
        T var;
        std::mutex mut;

        SafeType(const T& t):var(t){}
        SafeType(T&& t):var(std::move(t)){}
        SafeType():var(T()){}
    };
    class BTException : public std::exception{
        std::string _m;
    public:
        BTException(const std::string& m):_m(m){}
        BTException(const int tizen_error, std::string const& m):
        _m(std::string(get_error_message(tizen_error))+": "+m) {}

        BTException(const int tizen_error):
        _m(get_error_message(tizen_error)){}

        auto what() const noexcept -> const char* override{
            return _m.c_str();
        };
    };
    
    
    auto messageToVector(const google::protobuf::MessageLite& messageLite) noexcept -> std::vector<u_int8_t>;


    auto getGattValue(bt_gatt_h handle) -> std::string;
    auto getGattUUID(bt_gatt_h handle) -> std::string;
    auto getGattService(bt_gatt_client_h handle, const std::string& uuid) -> bt_gatt_h;
    auto getGattClientAddress(bt_gatt_client_h handle) -> std::string;
    auto getProtoServiceDiscoveryResult(const BluetoothDeviceController& device, const std::vector<btGatt::PrimaryService*>& services) -> proto::gen::DiscoverServicesResult;

    auto getProtoCharacteristicProperties(int properties) -> proto::gen::CharacteristicProperties;
}
#endif //UTILS_H
