// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "network_info_plus_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <tizen.h>
#include <wifi-manager.h>

#include <memory>
#include <string>

namespace {

class NetworkInfoPlusTizenPlugin : public flutter::Plugin {
 public:
  enum class WifiInfoType {
    kESSID,
    kBSSID,
    kIPv4Address,
    kIPv6Address,
    kSubnetMask,
    kGatewayAddress
  };

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

  NetworkInfoPlusTizenPlugin() {}

  virtual ~NetworkInfoPlusTizenPlugin() {}

 private:
  std::string GetWifiInfo(WifiInfoType type) {
    wifi_manager_h wifi_manager = nullptr;
    int ret = wifi_manager_initialize(&wifi_manager);
    if (ret != WIFI_MANAGER_ERROR_NONE) {
      return std::string();
    }

    wifi_manager_ap_h current_ap = nullptr;
    ret = wifi_manager_get_connected_ap(wifi_manager, &current_ap);
    if (ret != WIFI_MANAGER_ERROR_NONE) {
      wifi_manager_deinitialize(wifi_manager);
      return std::string();
    }

    char *value = nullptr;
    if (type == WifiInfoType::kESSID) {
      ret = wifi_manager_ap_get_essid(current_ap, &value);
    } else if (type == WifiInfoType::kBSSID) {
      ret = wifi_manager_ap_get_bssid(current_ap, &value);
    } else if (type == WifiInfoType::kIPv4Address) {
      ret = wifi_manager_ap_get_ip_address(
          current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV4, &value);
    } else if (type == WifiInfoType::kIPv6Address) {
      ret = wifi_manager_ap_get_ip_address(
          current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV6, &value);
    } else if (type == WifiInfoType::kSubnetMask) {
      ret = wifi_manager_ap_get_subnet_mask(
          current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV4, &value);
    } else if (type == WifiInfoType::kGatewayAddress) {
      // The requested gateway address is implicitly IPv4.
      ret = wifi_manager_ap_get_gateway_address(
          current_ap, WIFI_MANAGER_ADDRESS_FAMILY_IPV4, &value);
    }

    std::string result;
    if (value && ret == WIFI_MANAGER_ERROR_NONE) {
      result = value;
      free(value);
    }

    wifi_manager_ap_destroy(current_ap);
    wifi_manager_deinitialize(wifi_manager);

    return result;
  }

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();

    std::string value;
    if (method_name == "wifiName") {
      value = GetWifiInfo(WifiInfoType::kESSID);
    } else if (method_name == "wifiBSSID") {
      value = GetWifiInfo(WifiInfoType::kBSSID);
    } else if (method_name == "wifiIPAddress") {
      value = GetWifiInfo(WifiInfoType::kIPv4Address);
    } else if (method_name == "wifiIPv6Address") {
      value = GetWifiInfo(WifiInfoType::kIPv6Address);
    } else if (method_name == "wifiSubmask") {
      value = GetWifiInfo(WifiInfoType::kSubnetMask);
    } else if (method_name == "wifiGatewayAddress") {
      value = GetWifiInfo(WifiInfoType::kGatewayAddress);
    } else if (method_name == "wifiBroadcast") {
      std::string ipv4 = GetWifiInfo(WifiInfoType::kIPv4Address);
      std::string subnet_mask = GetWifiInfo(WifiInfoType::kSubnetMask);
      if (!ipv4.empty() && !subnet_mask.empty()) {
        value = IntegerToDottedDecimal(DottedDecimalToInteger(ipv4) |
                                       ~DottedDecimalToInteger(subnet_mask));
      }
    } else {
      result->NotImplemented();
      return;
    }

    if (value.empty()) {
      result->Error(std::to_string(get_last_result()),
                    get_error_message(get_last_result()));
      return;
    }
    result->Success(flutter::EncodableValue(value));
  }

  unsigned int DottedDecimalToInteger(std::string dotted_decimal) {
    size_t pos = 0;
    size_t len = dotted_decimal.size();
    std::string token;
    unsigned int value = 0U;
    unsigned int base = 1U << 24U;
    while (pos < len) {
      if (dotted_decimal[pos] == '.') {
        value += std::stoul(token) * base;
        base >>= 8U;
        token.clear();
      } else {
        token.push_back(dotted_decimal[pos]);
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
};

}  // namespace

void NetworkInfoPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  NetworkInfoPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
