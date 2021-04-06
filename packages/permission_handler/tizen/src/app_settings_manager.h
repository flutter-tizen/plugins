#ifndef APP_SETTINGS_MANAGER_H_
#define APP_SETTINGS_MANAGER_H_

#include <functional>
#include <memory>

using OnAppSettingsOpened = std::function<void(bool result)>;
using OnAppSettingsError =
    std::function<void(const std::string &code, const std::string &message)>;

class AppSettingsManager {
 public:
  AppSettingsManager();
  ~AppSettingsManager();

  void OpenAppSettings(OnAppSettingsOpened success_callback,
                       OnAppSettingsError error_callback);
};

#endif  // APP_SETTINGS_MANAGER_H_
