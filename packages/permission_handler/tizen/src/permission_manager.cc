#include "permission_manager.h"

#include <Ecore.h>

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
    error_callback(get_error_message(result),
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
      error_callback(get_error_message(result),
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

  struct Param {
    PermissionManager* self{nullptr};
    bool is_done{false};
    size_t remaining_request{0};
  };
  Param p;
  p.self = this;
  p.remaining_request = permissions_to_request.size();

  for (size_t i = 0; i < permissions_to_request.size(); i++) {
    const char* permission = permissions_to_request[i];
    p.is_done = false;
    result = ppm_request_permission(
        permission,
        [](ppm_call_cause_e cause, ppm_request_result_e result,
           const char* privilege, void* data) {
          Param* param = (Param*)data;
          PermissionManager* self = param->self;

          if (cause != PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ANSWER) {
            // abandon a request
            LOG_ERROR("Privilege[%s] request failed with an error", privilege);
            param->is_done = true;
            return;
          }

          int permission = self->ConvertToPermission(privilege);

          if (self->request_results_.count(permission) == 0) {
            switch (result) {
              case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
                self->request_results_[permission] = PERMISSION_STATUS_GRANTED;
                break;
              case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
                self->request_results_[permission] = PERMISSION_STATUS_DENIED;
                break;
              case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
                self->request_results_[permission] =
                    PERMISSION_STATUS_NEVER_ASK_AGAIN;
                break;
            }
          }
          LOG_DEBUG("permission %d status: %d", permission,
                    self->request_results_[permission]);
          auto location =
              self->request_results_.find(PERMISSION_GROUP_LOCATION);
          if (location != self->request_results_.end()) {
            self->request_results_[PERMISSION_GROUP_LOCATION_ALWAYS] =
                location->second;
            self->request_results_[PERMISSION_GROUP_LOCATION_WHEN_IN_USE] =
                location->second;
          }

          param->remaining_request--;
          param->is_done = true;
        },
        &p);

    if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      LOG_ERROR("Failed to call ppm_request_permission with [%s]", permission);
      continue;
    }

    // Wait until ppm_request_permission is done;
    while (!p.is_done) {
      ecore_main_loop_iterate();
    }
  }

  if (p.remaining_request) {
    error_callback(get_error_message(result),
                   "some error occurred when call ppm_request_permission");
  } else {
    success_callback(request_results_);
  }
  on_going_ = false;
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
                get_error_message(result));
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
