#include "in_app_purchase_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "billing_manager.h"
#include "log.h"

class InAppPurchaseTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar);
  InAppPurchaseTizenPlugin(FlutterDesktopPluginRegistrarRef registrar_ref,
                           flutter::PluginRegistrar *plugin_registrar);
  virtual ~InAppPurchaseTizenPlugin() { DisposeAllPlayers(); }

 private:
  void DisposeAllPlayers();
  FlutterDesktopPluginRegistrarRef registrar_ref_ = nullptr;
  flutter::PluginRegistrar *plugin_registrar_ = nullptr;
  std::unique_ptr<BillingManager> billing_ = nullptr;
};

void InAppPurchaseTizenPlugin::DisposeAllPlayers() {
  if (billing_) {
    billing_->Dispose();
  }
  billing_ = nullptr;
}

void InAppPurchaseTizenPlugin::RegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar) {
  auto plugin = std::make_unique<InAppPurchaseTizenPlugin>(registrar_ref,
                                                           plugin_registrar);
  plugin_registrar->AddPlugin(std::move(plugin));
}

InAppPurchaseTizenPlugin::InAppPurchaseTizenPlugin(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar)
    : registrar_ref_(registrar_ref), plugin_registrar_(plugin_registrar) {
  billing_ = std::make_unique<BillingManager>(registrar_ref_);
  billing_->Init();
}

void InAppPurchaseTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  InAppPurchaseTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
