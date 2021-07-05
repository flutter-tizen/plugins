#ifndef APP_SETTINGS_MANAGER_H_
#define APP_SETTINGS_MANAGER_H_

#include <functional>
#include <memory>
#include <string>

namespace {
class AppPermissions;
}

class AppSettingsManager {
 public:
  AppSettingsManager();
  ~AppSettingsManager();

  bool OpenAppSettings();

 private:
  std::unique_ptr<AppPermissions> app_permissions_;
  std::string package_name_;
};

#endif  // APP_SETTINGS_MANAGER_H_
