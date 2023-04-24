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
  static void RegisterWithRegistrar(flutter::PluginRegistrar *plugin_registrar);
  InAppPurchaseTizenPlugin(flutter::PluginRegistrar *plugin_registrar);
  virtual ~InAppPurchaseTizenPlugin() { Dispose(); }

 private:
  void Dispose();
  flutter::PluginRegistrar *plugin_registrar_ = nullptr;
  std::unique_ptr<BillingManager> billing_ = nullptr;
};

void InAppPurchaseTizenPlugin::Dispose() {
  if (billing_) {
    billing_->Dispose();
  }
  billing_ = nullptr;
}

void InAppPurchaseTizenPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *plugin_registrar) {
  auto plugin = std::make_unique<InAppPurchaseTizenPlugin>(plugin_registrar);
  plugin_registrar->AddPlugin(std::move(plugin));
}

InAppPurchaseTizenPlugin::InAppPurchaseTizenPlugin(
    flutter::PluginRegistrar *plugin_registrar)
    : plugin_registrar_(plugin_registrar) {
  billing_ = std::make_unique<BillingManager>(plugin_registrar_);
  if (!billing_->Init()) {
    Dispose();
  }
}

void InAppPurchaseTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  InAppPurchaseTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
