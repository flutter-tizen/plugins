#include "app_settings_manager.h"

#include <app_control.h>

#include "log.h"

AppSettingsManager::AppSettingsManager() {}

AppSettingsManager::~AppSettingsManager() {}

void AppSettingsManager::OpenAppSettings(OnAppSettingsOpened success_callback,
                                         OnAppSettingsError error_callback) {
  app_control_h service = nullptr;
  int result = app_control_create(&service);
  if (result != APP_CONTROL_ERROR_NONE) {
    error_callback(get_error_message(result),
                   "An error occurred when call app_control_create()");
  }

  app_control_set_operation(service, APP_CONTROL_OPERATION_SETTING);
  result = app_control_send_launch_request(service, NULL, NULL);
  app_control_destroy(service);
  if (result == APP_CONTROL_ERROR_NONE) {
    LOG_DEBUG("successed to open app settings");
    success_callback(true);
  } else {
    LOG_DEBUG("failed to open app settings");
    success_callback(false);
  }
}
