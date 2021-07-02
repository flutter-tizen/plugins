#ifndef APP_SETTINGS_MANAGER_H_
#define APP_SETTINGS_MANAGER_H_

#include <functional>
#include <memory>

class AppSettingsManager {
 public:
  AppSettingsManager();
  ~AppSettingsManager();

  bool OpenAppSettings();
};

#endif  // APP_SETTINGS_MANAGER_H_
