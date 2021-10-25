// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "network_info_plus_tizen_plugin.h"

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

class NetworkInfoPlusTizenPlugin : public flutter::Plugin {
 public:
  enum WifiInfoType { ESSID, BSSID, IPV4, IPV6, SUBNET_MASK, GATEWAY_ADDR };

  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "dev.fluttercommunity.plus/network_info",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<NetworkInfoPlusTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  NetworkInfoPlusTizenPlugin() : wifi_manager_(nullptr) {
    EnsureConnectionHandle();
  }

  virtual ~NetworkInfoPlusTizenPlugin() {
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
      } else if (type == WifiInfoType::BSSID) {
        errorcode = wifi_manager_ap_get_bssid(current_ap, &name);
      } else if (type == WifiInfoType::IPV4) {
        errorcode = wifi_manager_ap_get_ip_address(
            current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV4, &name);
      } else if (type == WifiInfoType::IPV6) {
        errorcode = wifi_manager_ap_get_ip_address(
            current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV6, &name);
      } else if (type == WifiInfoType::SUBNET_MASK) {
        errorcode = wifi_manager_ap_get_subnet_mask(
            current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV4, &name);
      } else {  // WifiInfoType::GATEWAY_ADDR
        // The requested gateway address is implicitly ipv4.
        // https://github.com/fluttercommunity/plus_plugins/blob/bd0262e5f4627358bfb42481a84122f60921d98b/packages/network_info_plus/network_info_plus/android/src/main/java/dev/fluttercommunity/plus/network_info/NetworkInfo.java#L108
        errorcode = wifi_manager_ap_get_gateway_address(
            current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV4, &name);
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
    std::string reply = "";
    if (method_call.method_name().compare("wifiName") == 0) {
      reply = GetWifiInfo(WifiInfoType::ESSID);
    } else if (method_call.method_name().compare("wifiBSSID") == 0) {
      reply = GetWifiInfo(WifiInfoType::BSSID);
    } else if (method_call.method_name().compare("wifiIPAddress") == 0) {
      reply = GetWifiInfo(WifiInfoType::IPV4);
    } else if (method_call.method_name().compare("wifiIPv6Address") == 0) {
      reply = GetWifiInfo(WifiInfoType::IPV6);
    } else if (method_call.method_name().compare("wifiSubmask") == 0) {
      reply = GetWifiInfo(WifiInfoType::SUBNET_MASK);
    } else if (method_call.method_name().compare("wifiGatewayAddress") == 0) {
      reply = GetWifiInfo(WifiInfoType::GATEWAY_ADDR);
    } else if (method_call.method_name().compare("wifiBroadcast") == 0) {
      std::string ipv4 = GetWifiInfo(WifiInfoType::IPV4);
      std::string subnet_mask = GetWifiInfo(WifiInfoType::SUBNET_MASK);
      reply = IntegerToDottedDecimal(DottedDecimalToInteger(ipv4) |
                                     ~DottedDecimalToInteger(subnet_mask));
    } else {
      result->NotImplemented();
      return;
    }
    if (reply.length() == 0) {
      result->Error("-1", "Not valid result");
      return;
    }
    flutter::EncodableValue msg(reply);
    result->Success(msg);
  }

  bool EnsureConnectionHandle() {
    if (wifi_manager_ == nullptr) {
      if (wifi_manager_initialize(&wifi_manager_) != WIFI_MANAGER_ERROR_NONE) {
        wifi_manager_ = nullptr;
        return false;
      }
    }
    return true;
  }

  unsigned int DottedDecimalToInteger(std::string dottedDecimal) {
    size_t pos = 0;
    size_t len = dottedDecimal.size();
    std::string token;
    unsigned int value = 0U;
    unsigned int base = 1U << 24U;
    while (pos < len) {
      if (dottedDecimal[pos] == '.') {
        value += std::stoul(token) * base;
        base >>= 8U;
        token.clear();
      } else {
        token.push_back(dottedDecimal[pos]);
      }
      pos++;
    }
    return value + std::stoul(token);
  }

  std::string IntegerToDottedDecimal(unsigned int value) {
    return std::to_string((value >> 24U) % 256U) + "." +
           std::to_string((value >> 16U) % 256U) + "." +
           std::to_string((value >> 8U) % 256U) + "." +
           std::to_string(value % 256U);
  }

  wifi_manager_h wifi_manager_;
};

void NetworkInfoPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  NetworkInfoPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
