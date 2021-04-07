#include "service_manager.h"

#include <locations.h>

#include "log.h"
#include "type.h"

ServiceManager::ServiceManager() {}

ServiceManager::~ServiceManager() {}

void ServiceManager::CheckServiceStatus(int permission,
                                        OnServiceChecked success_callback,
                                        OnServiceError error_callback) {
  if (permission == PERMISSION_GROUP_LOCATION ||
      permission == PERMISSION_GROUP_LOCATION_ALWAYS ||
      permission == PERMISSION_GROUP_LOCATION_WHEN_IN_USE) {
    bool gps_enabled, wps_enabled;
    if (location_manager_is_enabled_method(
            LOCATIONS_METHOD_GPS, &gps_enabled) != LOCATIONS_ERROR_NONE) {
      gps_enabled = false;
    }
    if (location_manager_is_enabled_method(
            LOCATIONS_METHOD_WPS, &wps_enabled) != LOCATIONS_ERROR_NONE) {
      wps_enabled = false;
    }

    if (gps_enabled || wps_enabled) {
      success_callback(SERVICE_STATUS_ENABLED);
    } else {
      success_callback(SERVICE_STATUS_DISABLED);
    }
    return;
  }

  success_callback(SERVICE_STATUS_NOT_APPLICABLE);
}
