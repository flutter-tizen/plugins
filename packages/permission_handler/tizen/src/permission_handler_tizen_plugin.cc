#include "permission_handler_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include "app_settings_manager.h"
#include "log.h"
#include "permission_manager.h"
#include "service_manager.h"

namespace {

class PermissionHandlerTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "flutter.baseflow.com/permissions/methods",
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
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    auto method_name = method_call.method_name();
    LOG_INFO("method : %s", method_name.c_str());

    if (method_name.compare("checkServiceStatus") == 0) {
      const flutter::EncodableValue *arguments = method_call.arguments();
      if (std::holds_alternative<int32_t>(*arguments)) {
        int permission = std::get<int32_t>(*arguments);
        auto reply = result.release();
        auto on_success = [reply](ServiceStatus status) {
          reply->Success(flutter::EncodableValue(static_cast<int>(status)));
          delete reply;
        };
        auto on_error = [reply](const std::string &code,
                                const std::string &message) {
          reply->Error(code, message);
          delete reply;
        };
        service_manager_.CheckServiceStatus(
            static_cast<PermissionGroup>(permission), on_success, on_error);
      } else {
        result->Error("MethodCall - Invalid arguments",
                      "arguments type of method checkServiceStatus isn't int");
      }
    } else if (method_name.compare("checkPermissionStatus") == 0) {
      const flutter::EncodableValue *arguments = method_call.arguments();
      if (std::holds_alternative<int32_t>(*arguments)) {
        int permission = std::get<int32_t>(*arguments);
        auto reply = result.release();
        auto on_success = [reply](PermissionStatus status) {
          reply->Success(flutter::EncodableValue(static_cast<int>(status)));
          delete reply;
        };
        auto on_error = [reply](const std::string &code,
                                const std::string &message) {
          reply->Error(code, message);
          delete reply;
        };
        permission_manager_.CheckPermissionStatus(
            static_cast<PermissionGroup>(permission), on_success, on_error);
      } else {
        result->Error(
            "MethodCall - Invalid arguments",
            "arguments type of method checkPermissionStatus isn't int");
      }
    } else if (method_name.compare("requestPermissions") == 0) {
      const flutter::EncodableValue *arguments = method_call.arguments();
      if (std::holds_alternative<flutter::EncodableList>(*arguments)) {
        std::vector<PermissionGroup> permissions;
        for (auto iter : std::get<flutter::EncodableList>(*arguments)) {
          permissions.push_back(
              static_cast<PermissionGroup>(std::get<int32_t>(iter)));
        }
        auto reply = result.release();
        auto on_success =
            [reply](
                const std::map<PermissionGroup, PermissionStatus> &results) {
              flutter::EncodableMap encodables;
              for (auto [key, value] : results) {
                encodables.emplace(
                    flutter::EncodableValue(static_cast<int>(key)),
                    flutter::EncodableValue(static_cast<int>(value)));
              }
              reply->Success(flutter::EncodableValue(encodables));
              delete reply;
            };
        auto on_error = [reply](const std::string &code,
                                const std::string &message) {
          reply->Error(code, message);
          delete reply;
        };
        permission_manager_.RequestPermissions(permissions, on_success,
                                               on_error);
      } else {
        result->Error("MethodCall - Invalid arguments",
                      "arguments type of method requestPermissions isn't "
                      "vector<int32_t>");
      }
    } else if (method_name.compare("openAppSettings") == 0) {
      bool ret = app_settings_manager_.OpenAppSettings();
      result->Success(flutter::EncodableValue(ret));
    } else {
      result->NotImplemented();
      return;
    }
  }

  PermissionManager permission_manager_;
  AppSettingsManager app_settings_manager_;
  ServiceManager service_manager_;
};

}  // namespace

void PermissionHandlerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  PermissionHandlerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
