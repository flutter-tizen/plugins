#include "permission_handler_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include "app_settings_manager.h"
#include "log.h"
#include "permission_manager.h"
#include "service_manager.h"

#define PERMISSION_HANDLER_CHANNEL_NAME "flutter.baseflow.com/permissions/methods"

class PermissionHandlerTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), PERMISSION_HANDLER_CHANNEL_NAME,
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<PermissionHandlerTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  PermissionHandlerTizenPlugin() {}

  virtual ~PermissionHandlerTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &methodCall,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_INFO("method : %s", methodCall.method_name().data());

    if (methodCall.method_name().compare("checkServiceStatus") == 0) {
      const flutter::EncodableValue *arguments = methodCall.arguments();
      if (std::holds_alternative<int32_t>(*arguments)) {
        int permission = std::get<int32_t>(*arguments);
        auto reply = result.release();
        auto onSuccess = [reply](int status) {
          reply->Success(flutter::EncodableValue(status));
          delete reply;
        };
        auto onError = [reply](const std::string &code,
                               const std::string &message) {
          reply->Error(code, message);
          delete reply;
        };
        _serviceManager.CheckServiceStatus(permission, onSuccess, onError);
      } else {
        result->Error("MethodCall - Invalid arguments",
                      "arguments type of method checkServiceStatus isn't int");
      }
    } else if (methodCall.method_name().compare("checkPermissionStatus") == 0) {
      const flutter::EncodableValue *arguments = methodCall.arguments();
      if (std::holds_alternative<int32_t>(*arguments)) {
        int permission = std::get<int32_t>(*arguments);
        auto reply = result.release();
        auto onSuccess = [reply](int status) {
          reply->Success(flutter::EncodableValue(status));
          delete reply;
        };
        auto onError = [reply](const std::string &code,
                               const std::string &message) {
          reply->Error(code, message);
          delete reply;
        };
        _permissionManager.CheckPermissionStatus(permission, onSuccess,
                                                 onError);
      } else {
        result->Error(
            "MethodCall - Invalid arguments",
            "arguments type of method checkPermissionStatus isn't int");
      }
    } else if (methodCall.method_name().compare("requestPermissions") == 0) {
      const flutter::EncodableValue *arguments = methodCall.arguments();
      if (std::holds_alternative<flutter::EncodableList>(*arguments)) {
        std::vector<int> permissions;
        for (auto iter : std::get<flutter::EncodableList>(*arguments)) {
          permissions.push_back(std::get<int32_t>(iter));
        }
        auto reply = result.release();
        auto onSuccess = [reply](const std::map<int, int> &results) {
          flutter::EncodableMap encodables;
          for (auto [key, value] : results) {
            encodables.emplace(flutter::EncodableValue(key),
                               flutter::EncodableValue(value));
          }
          reply->Success(flutter::EncodableValue(encodables));
          delete reply;
        };
        auto onError = [reply](const std::string &code,
                               const std::string &message) {
          reply->Error(code, message);
          delete reply;
        };
        _permissionManager.RequestPermissions(permissions, onSuccess, onError);
      } else {
        result->Error("MethodCall - Invalid arguments",
                      "arguments type of method requestPermissions isn't "
                      "vector<int32_t>");
      }
    } else if (methodCall.method_name().compare("openAppSettings") == 0) {
      auto reply = result.release();
      auto onSuccess = [reply](bool result) {
        reply->Success(flutter::EncodableValue(result));
        delete reply;
      };
      auto onError = [reply](const std::string &code,
                             const std::string &message) {
        reply->Error(code, message);
        delete reply;
      };
      _appSettingsManager.OpenAppSettings(onSuccess, onError);
    } else {
      result->NotImplemented();
      return;
    }
  }

  PermissionManager _permissionManager;
  AppSettingsManager _appSettingsManager;
  ServiceManager _serviceManager;
};

void PermissionHandlerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  PermissionHandlerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
