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

  WifiInfoFlutterTizenPlugin() : m_connection(nullptr), m_wifiManager(nullptr) {
    ensureConnectionHandle();
  }

  virtual ~WifiInfoFlutterTizenPlugin() {
    if (m_connection != nullptr) {
      connection_destroy(m_connection);
      m_connection = nullptr;
    }
    if (m_wifiManager != nullptr) {
      wifi_manager_deinitialize(m_wifiManager);
      m_wifiManager = nullptr;
    }
  }

 private:
  std::string getWifiInfo(WifiInfoType type) {
    std::string result;
    wifi_manager_ap_h currentAP = nullptr;
    char *name = NULL;
    int errorcode;

    errorcode = wifi_manager_get_connected_ap(m_wifiManager, &currentAP);
    if (errorcode == WIFI_MANAGER_ERROR_NONE && currentAP != nullptr) {
      if (type == WifiInfoType::ESSID) {
        errorcode = wifi_manager_ap_get_essid(currentAP, &name);
      } else {
        errorcode = wifi_manager_ap_get_bssid(currentAP, &name);
      }
      if (errorcode == WIFI_MANAGER_ERROR_NONE) {
        result = name;
        free(name);
      }
      wifi_manager_ap_destroy(currentAP);
    }
    return result;
  }
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    if (!ensureConnectionHandle()) {
      result->Error("-1", "Connection has problem");
      return;
    }
    std::string replay = "";
    if (method_call.method_name().compare("wifiName") == 0) {
      replay = getWifiInfo(WifiInfoType::ESSID);
    } else if (method_call.method_name().compare("wifiBSSID") == 0) {
      replay = getWifiInfo(WifiInfoType::BSSID);
    } else if (method_call.method_name().compare("wifiIPAddress") == 0) {
      char *ip_addr = NULL;
      if (connection_get_ip_address(m_connection,
                                    CONNECTION_ADDRESS_FAMILY_IPV4,
                                    &ip_addr) != CONNECTION_ERROR_NONE) {
        result->Error("-1", "Couldn't know current ip address");
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
    result->Success(&msg);
  }

  bool ensureConnectionHandle() {
    if (m_connection == nullptr) {
      if (connection_create(&m_connection) != CONNECTION_ERROR_NONE) {
        m_connection = nullptr;
        return false;
      }
    }
    if (m_wifiManager == nullptr) {
      if (wifi_manager_initialize(&m_wifiManager) != WIFI_MANAGER_ERROR_NONE) {
        m_wifiManager = nullptr;
        return false;
      }
    }
    return true;
  }
  connection_h m_connection;
  wifi_manager_h m_wifiManager;
};

void WifiInfoFlutterTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WifiInfoFlutterTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
