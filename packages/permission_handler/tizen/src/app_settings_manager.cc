#include "app_settings_manager.h"

#include <app_control.h>

#include "log.h"

static std::string ErrorToString(int error) {
  switch (error) {
    case APP_CONTROL_ERROR_NONE:
      return "AppControl - Successful";
    case APP_CONTROL_ERROR_INVALID_PARAMETER:
      return "AppControl - Invalid parameter";
    case APP_CONTROL_ERROR_OUT_OF_MEMORY:
      return "AppControl - Out of memory";
    default:
      return "AppControl - Unknown error";
  }
}

AppSettingsManager::AppSettingsManager() {}

AppSettingsManager::~AppSettingsManager() {}

void AppSettingsManager::OpenAppSettings(OnAppSettingsOpened success_callback,
                                         OnAppSettingsError error_callback) {
  app_control_h service = nullptr;
  int result = app_control_create(&service);
  if (result != APP_CONTROL_ERROR_NONE) {
    error_callback(ErrorToString(result),
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
