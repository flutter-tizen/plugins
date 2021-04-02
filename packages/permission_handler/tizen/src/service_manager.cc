#include "service_manager.h"

#include <locations.h>

#include "log.h"
#include "type.h"

ServiceManager::ServiceManager() {}

ServiceManager::~ServiceManager() {}

void ServiceManager::CheckServiceStatus(int permission,
                                        OnServiceChecked successCallback,
                                        OnServiceError errorCallback) {
  if (permission == PERMISSION_GROUP_LOCATION ||
      permission == PERMISSION_GROUP_LOCATION_ALWAYS ||
      permission == PERMISSION_GROUP_LOCATION_WHEN_IN_USE) {
    bool isGPSEnabled, isWPSEnabled;
    int ret =
        location_manager_is_enabled_method(LOCATIONS_METHOD_GPS, &isGPSEnabled);
    if (ret != LOCATIONS_ERROR_NONE) {
      isGPSEnabled = false;
    }

    ret =
        location_manager_is_enabled_method(LOCATIONS_METHOD_WPS, &isWPSEnabled);
    if (ret != LOCATIONS_ERROR_NONE) {
      isWPSEnabled = false;
    }

    if (isGPSEnabled || isWPSEnabled) {
      successCallback(SERVICE_STATUS_ENABLED);
    } else {
      successCallback(SERVICE_STATUS_DISABLED);
    }
    return;
  }

  successCallback(SERVICE_STATUS_NOT_APPLICABLE);
}
