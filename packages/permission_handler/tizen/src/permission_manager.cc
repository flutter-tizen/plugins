#include "permission_manager.h"

#include "log.h"
#include "type.h"

const char* PRIVILEGE_CALENDAR_READ =
    "http://tizen.org/privilege/calendar.read";
const char* PRIVILEGE_CALENDAR_WRITE =
    "http://tizen.org/privilege/calendar.write";
const char* PRIVILEGE_CAMERA = "http://tizen.org/privilege/camera";
const char* PRIVILEGE_CONTACT_READ = "http://tizen.org/privilege/contact.read";
const char* PRIVILEGE_CONTACT_WRITE =
    "http://tizen.org/privilege/contact.write";
const char* PRIVILEGE_LOCATION = "http://tizen.org/privilege/location";
const char* PRIVILEGE_LOCATION_COARSE =
    "http://tizen.org/privilege/location.coarse";
const char* PRIVILEGE_RECORDER = "http://tizen.org/privilege/recorder";
const char* PRIVILEGE_CALL = "http://tizen.org/privilege/call";
const char* PRIVILEGE_SENSORS = "http://tizen.org/privilege/healthinfo";
const char* PRIVILEGE_MESSAGE_READ = "http://tizen.org/privilege/message.read";
const char* PRIVILEGE_MESSAGE_WRITE =
    "http://tizen.org/privilege/message.write";
const char* PRIVILEGE_EXTERNAL_STORAGE =
    "http://tizen.org/privilege/externalstorage";
const char* PRIVILEGE_MEDIA_STORAGE = "http://tizen.org/privilege/mediastorage";

static std::string PPMErrorToString(int error) {
  switch (error) {
    case PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE:
      return "PrivacyPrivilegeManager - Successful";
    case PRIVACY_PRIVILEGE_MANAGER_ERROR_IO_ERROR:
      return "PrivacyPrivilegeManager - IO error";
    case PRIVACY_PRIVILEGE_MANAGER_ERROR_INVALID_PARAMETER:
      return "PrivacyPrivilegeManager - Invalid parameter";
    case PRIVACY_PRIVILEGE_MANAGER_ERROR_ALREADY_IN_PROGRESS:
      return "PrivacyPrivilegeManager - Operation already in progress";
    case PRIVACY_PRIVILEGE_MANAGER_ERROR_OUT_OF_MEMORY:
      return "PrivacyPrivilegeManager - Out of memory";
    default:
      return "PrivacyPrivilegeManager - Unknown error";
  }
}

static std::string CheckResultToString(int result) {
  switch (result) {
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
      return "CHECK_RESULT_ALLOW";
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
      return "CHECK_RESULT_DENY";
    case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
      return "CHECK_RESULT_ASK";
    default:
      return "CHECK_RESULT_UNKNOWN";
  }
}

static std::string RequestResultToString(int result) {
  switch (result) {
    case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
      return "REQUEST_RESULT_ALLOW_FOREVER";
    case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
      return "REQUEST_RESULT_DENY_FOREVER";
    case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
      return "REQUEST_RESULT_DENY_ONCE";
    default:
      return "REQUEST_RESULT_UNKNOWN";
  }
}

PermissionManager::PermissionManager() : on_going_(false) {}

PermissionManager::~PermissionManager() {}

void PermissionManager::CheckPermissionStatus(
    int permission, OnPermissionChecked success_callback,
    OnPermissionError error_callback) {
  LOG_DEBUG("Check permission %d status", permission);

  int status;
  int result = DeterminePermissionStatus(permission, &status);
  if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
    error_callback(PPMErrorToString(result),
                   "An error occurred when call ppm_check_permission()");
  } else {
    success_callback(status);
  }
}

void PermissionManager::RequestPermissions(
    std::vector<int> permissions, OnPermissionRequested success_callback,
    OnPermissionError error_callback) {
  if (on_going_) {
    error_callback("RequestPermissions - error",
                   "A request for permissions is already running");
    return;
  }

  int status, result;
  request_results_.clear();
  std::vector<const char*> permissions_to_request;
  for (auto permission : permissions) {
    result = DeterminePermissionStatus(permission, &status);
    if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      error_callback(PPMErrorToString(result),
                     "An error occurred when call ppm_check_permission()");
      return;
    }

    if (status == PERMISSION_STATUS_GRANTED) {
      if (request_results_.find(permission) == request_results_.end()) {
        LOG_DEBUG("Request permission %d result: PERMISSION_STATUS_GRANTED",
                  permission);
        request_results_[permission] = PERMISSION_STATUS_GRANTED;
      }
      continue;
    }

    ConvertToPrivileges(permission, permissions_to_request);
  }

  // no permission is needed to requested
  if (permissions_to_request.size() == 0) {
    success_callback(request_results_);
    return;
  }

  on_going_ = true;
  request_success_callback_ = success_callback;
  request_error_callback_ = error_callback;
  result = ppm_request_permissions(permissions_to_request.data(),
                                   permissions_to_request.size(),
                                   OnRequestPermissionsResponse, this);
  if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
    error_callback(PPMErrorToString(result),
                   "An error occurred when call ppm_request_permissions()");
    on_going_ = false;
  }
}

int PermissionManager::ConvertToPermission(const std::string& privilege) {
  if (privilege == PRIVILEGE_CALENDAR_READ ||
      privilege == PRIVILEGE_CALENDAR_WRITE) {
    return PERMISSION_GROUP_CALENDAR;
  } else if (privilege == PRIVILEGE_CAMERA) {
    return PERMISSION_GROUP_CAMERA;
  } else if (privilege == PRIVILEGE_CONTACT_READ ||
             privilege == PRIVILEGE_CONTACT_WRITE) {
    return PERMISSION_GROUP_CONTACTS;
  } else if (privilege == PRIVILEGE_LOCATION ||
             privilege == PRIVILEGE_LOCATION_COARSE) {
    return PERMISSION_GROUP_LOCATION;
  } else if (privilege == PRIVILEGE_RECORDER) {
    return PERMISSION_GROUP_MICROPHONE;
  } else if (privilege == PRIVILEGE_CALL) {
    return PERMISSION_GROUP_PHONE;
  } else if (privilege == PRIVILEGE_SENSORS) {
    return PERMISSION_GROUP_SENSORS;
  } else if (privilege == PRIVILEGE_MESSAGE_READ ||
             privilege == PRIVILEGE_MESSAGE_WRITE) {
    return PERMISSION_GROUP_SMS;
  } else if (privilege == PRIVILEGE_EXTERNAL_STORAGE) {
    return PERMISSION_GROUP_STORAGE;
  } else if (privilege == PRIVILEGE_MEDIA_STORAGE) {
    return PERMISSION_GROUP_MEDIA_LIBRARY;
  } else {
    return PERMISSION_GROUP_UNKNOWN;
  }
}

void PermissionManager::ConvertToPrivileges(
    int permission, std::vector<const char*>& privileges) {
  switch (permission) {
    case PERMISSION_GROUP_CALENDAR:
      privileges.push_back(PRIVILEGE_CALENDAR_READ);
      privileges.push_back(PRIVILEGE_CALENDAR_WRITE);
      break;
    case PERMISSION_GROUP_CAMERA:
      privileges.push_back(PRIVILEGE_CAMERA);
      break;
    case PERMISSION_GROUP_CONTACTS:
      privileges.push_back(PRIVILEGE_CONTACT_READ);
      privileges.push_back(PRIVILEGE_CONTACT_WRITE);
      break;
    case PERMISSION_GROUP_LOCATION:
    case PERMISSION_GROUP_LOCATION_ALWAYS:
    case PERMISSION_GROUP_LOCATION_WHEN_IN_USE:
      privileges.push_back(PRIVILEGE_LOCATION);
      privileges.push_back(PRIVILEGE_LOCATION_COARSE);
      break;
    case PERMISSION_GROUP_MICROPHONE:
      privileges.push_back(PRIVILEGE_RECORDER);
      break;
    case PERMISSION_GROUP_PHONE:
      privileges.push_back(PRIVILEGE_CALL);
      break;
    case PERMISSION_GROUP_SENSORS:
      privileges.push_back(PRIVILEGE_SENSORS);
      break;
    case PERMISSION_GROUP_SMS:
      privileges.push_back(PRIVILEGE_MESSAGE_READ);
      privileges.push_back(PRIVILEGE_MESSAGE_WRITE);
      break;
    case PERMISSION_GROUP_STORAGE:
      privileges.push_back(PRIVILEGE_EXTERNAL_STORAGE);
      break;
    case PERMISSION_GROUP_MEDIA_LIBRARY:
      privileges.push_back(PRIVILEGE_MEDIA_STORAGE);
      break;
    default:
      break;
  }
}

int PermissionManager::DeterminePermissionStatus(int permission, int* status) {
  std::vector<const char*> privileges;
  ConvertToPrivileges(permission, privileges);

  if (privileges.size() == 0) {
    LOG_DEBUG("No tizen specific privileges needed for permission %d",
              permission);
    *status = PERMISSION_STATUS_GRANTED;
    return PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE;
  }

  int result;
  ppm_check_result_e check_result;
  for (auto iter : privileges) {
    result = ppm_check_permission(iter, &check_result);
    if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      LOG_ERROR("ppm_check_permission (%s) error: %s", iter,
                PPMErrorToString(result).c_str());
      return result;
    } else {
      LOG_DEBUG("ppm_check_permission (%s) result: %s", iter,
                CheckResultToString(check_result).c_str());
      switch (check_result) {
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
          *status = PERMISSION_STATUS_DENIED;
          return result;
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
        default:
          *status = PERMISSION_STATUS_GRANTED;
          break;
      }
    }
  }
  return result;
}

void PermissionManager::OnRequestPermissionsResponse(
    ppm_call_cause_e cause, const ppm_request_result_e* results,
    const char** privileges, size_t privileges_count, void* user_data) {
  if (!user_data) {
    LOG_ERROR("Invalid user data");
    return;
  }

  PermissionManager* permission_manager = (PermissionManager*)user_data;
  if (cause != PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ANSWER) {
    permission_manager->request_error_callback_(
        "PrivacyPrivilegeManager - Request callback error",
        "ppm_request_permissions callback was called because of an error");
    permission_manager->on_going_ = false;
    return;
  }

  for (int i = 0; i < privileges_count; i++) {
    LOG_DEBUG("ppm_request_permissions (%s) result: %s", privileges[i],
              RequestResultToString(results[i]).c_str());

    int permission = permission_manager->ConvertToPermission(privileges[i]);
    if (permission == PERMISSION_GROUP_UNKNOWN) {
      continue;
    }

    if (permission_manager->request_results_.count(permission) == 0) {
      switch (results[i]) {
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
          permission_manager->request_results_[permission] =
              PERMISSION_STATUS_GRANTED;
          break;
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
          permission_manager->request_results_[permission] =
              PERMISSION_STATUS_DENIED;
          break;
        case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
          permission_manager->request_results_[permission] =
              PERMISSION_STATUS_NEVER_ASK_AGAIN;
          break;
      }
    }
    LOG_DEBUG("permission %d status: %d", permission,
              permission_manager->request_results_[permission]);
  }

  auto location =
      permission_manager->request_results_.find(PERMISSION_GROUP_LOCATION);
  if (location != permission_manager->request_results_.end()) {
    permission_manager->request_results_[PERMISSION_GROUP_LOCATION_ALWAYS] =
        location->second;
    permission_manager
        ->request_results_[PERMISSION_GROUP_LOCATION_WHEN_IN_USE] =
        location->second;
  }

  permission_manager->request_success_callback_(
      permission_manager->request_results_);
  permission_manager->on_going_ = false;
}
