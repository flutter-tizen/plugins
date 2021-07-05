#include "service_manager.h"

#include <locations.h>

#include "log.h"
#include "type.h"

ServiceManager::ServiceManager() {}

ServiceManager::~ServiceManager() {}

void ServiceManager::CheckServiceStatus(PermissionGroup permission,
                                        OnServiceChecked success_callback,
                                        OnServiceError error_callback) {
  if (permission == PermissionGroup::kLocation ||
      permission == PermissionGroup::kLocationAlways ||
      permission == PermissionGroup::kLocationWhenInUse) {
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
      success_callback(ServiceStatus::kEnabled);
    } else {
      success_callback(ServiceStatus::kDisabled);
    }
    return;
  }

  success_callback(ServiceStatus::kNotApplicable);
}
