#include "wifi_info_flutter_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <net_connection.h>
#include <wifi-manager.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"

class WifiInfoFlutterTizenPlugin : public flutter::Plugin {
 public:
  enum WifiInfoType { ESSID, BSSID };

  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/wifi_info_flutter",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<WifiInfoFlutterTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  WifiInfoFlutterTizenPlugin() : connection_(nullptr), wifi_manager_(nullptr) {
    EnsureConnectionHandle();
  }

  virtual ~WifiInfoFlutterTizenPlugin() {
    if (connection_ != nullptr) {
      connection_destroy(connection_);
      connection_ = nullptr;
    }
    if (wifi_manager_ != nullptr) {
      wifi_manager_deinitialize(wifi_manager_);
      wifi_manager_ = nullptr;
    }
  }

 private:
  std::string GetWifiInfo(WifiInfoType type) {
    std::string result;
    wifi_manager_ap_h current_ap = nullptr;
    char *name = nullptr;
    int errorcode;

    errorcode = wifi_manager_get_connected_ap(wifi_manager_, &current_ap);
    if (errorcode == WIFI_MANAGER_ERROR_NONE && current_ap != nullptr) {
      if (type == WifiInfoType::ESSID) {
        errorcode = wifi_manager_ap_get_essid(current_ap, &name);
      } else {
        errorcode = wifi_manager_ap_get_bssid(current_ap, &name);
      }
      if (errorcode == WIFI_MANAGER_ERROR_NONE) {
        result = name;
        free(name);
      }
      wifi_manager_ap_destroy(current_ap);
    }
    return result;
  }

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if (!EnsureConnectionHandle()) {
      result->Error("-1", "Initialization failed");
      return;
    }
    std::string replay = "";
    if (method_call.method_name().compare("wifiName") == 0) {
      replay = GetWifiInfo(WifiInfoType::ESSID);
    } else if (method_call.method_name().compare("wifiBSSID") == 0) {
      replay = GetWifiInfo(WifiInfoType::BSSID);
    } else if (method_call.method_name().compare("wifiIPAddress") == 0) {
      char *ip_addr = nullptr;
      if (connection_get_ip_address(connection_, CONNECTION_ADDRESS_FAMILY_IPV4,
                                    &ip_addr) != CONNECTION_ERROR_NONE) {
        result->Error("-1", "Couldn't obtain current ip address");
        return;
      }
      replay = ip_addr;
      free(ip_addr);
    } else {
      result->NotImplemented();
      return;
    }
    if (replay.length() == 0) {
      result->Error("-1", "Not valid result");
      return;
    }
    flutter::EncodableValue msg(replay);
    result->Success(msg);
  }

  bool EnsureConnectionHandle() {
    if (connection_ == nullptr) {
      if (connection_create(&connection_) != CONNECTION_ERROR_NONE) {
        connection_ = nullptr;
        return false;
      }
    }
    if (wifi_manager_ == nullptr) {
      if (wifi_manager_initialize(&wifi_manager_) != WIFI_MANAGER_ERROR_NONE) {
        wifi_manager_ = nullptr;
        return false;
      }
    }
    return true;
  }

  connection_h connection_;
  wifi_manager_h wifi_manager_;
};

void WifiInfoFlutterTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WifiInfoFlutterTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
