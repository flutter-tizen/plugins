#include "permission_manager.h"

#include <Ecore.h>

#include "log.h"
#include "type.h"

namespace {

constexpr char kPrivilegeCalendarRead[] =
    "http://tizen.org/privilege/calendar.read";
constexpr char kPrivilegeCalendarWrite[] =
    "http://tizen.org/privilege/calendar.write";
constexpr char kPrivilegeCamera[] = "http://tizen.org/privilege/camera";
constexpr char kPrivilegeContactRead[] =
    "http://tizen.org/privilege/contact.read";
constexpr char kPrivilegeContactWrite[] =
    "http://tizen.org/privilege/contact.write";
constexpr char kPrivilegeLocation[] = "http://tizen.org/privilege/location";
constexpr char kPrivilegeLocationCoarse[] =
    "http://tizen.org/privilege/location.coarse";
constexpr char kPrivilegeRecorder[] = "http://tizen.org/privilege/recorder";
constexpr char kPrivilegeCall[] = "http://tizen.org/privilege/call";
constexpr char kPrivilegeSensors[] = "http://tizen.org/privilege/healthinfo";
constexpr char kPrivilegeMessageRead[] =
    "http://tizen.org/privilege/message.read";
constexpr char kPrivilegeMessageWrite[] =
    "http://tizen.org/privilege/message.write";
constexpr char kPrivilegeExternalStorage[] =
    "http://tizen.org/privilege/externalstorage";
constexpr char kPrivilegeMediaStorage[] =
    "http://tizen.org/privilege/mediastorage";

struct Param {
  PermissionManager* manager{nullptr};
  bool is_done{false};
  size_t remaining_request{0};
};

std::string CheckResultToString(int result) {
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

bool ConvertToPermission(const std::string& privilege,
                         PermissionGroup* permission) {
  if (privilege == kPrivilegeCalendarRead ||
      privilege == kPrivilegeCalendarWrite) {
    *permission = PermissionGroup::kCalendar;
  } else if (privilege == kPrivilegeCamera) {
    *permission = PermissionGroup::kCamera;
  } else if (privilege == kPrivilegeContactRead ||
             privilege == kPrivilegeContactWrite) {
    *permission = PermissionGroup::kContacts;
  } else if (privilege == kPrivilegeLocation ||
             privilege == kPrivilegeLocationCoarse) {
    *permission = PermissionGroup::kLocation;
  } else if (privilege == kPrivilegeRecorder) {
    *permission = PermissionGroup::kMicrophone;
  } else if (privilege == kPrivilegeCall) {
    *permission = PermissionGroup::kPhone;
  } else if (privilege == kPrivilegeSensors) {
    *permission = PermissionGroup::kSensors;
  } else if (privilege == kPrivilegeMessageRead ||
             privilege == kPrivilegeMessageWrite) {
    *permission = PermissionGroup::kSMS;
  } else if (privilege == kPrivilegeExternalStorage) {
    *permission = PermissionGroup::kStorage;
  } else if (privilege == kPrivilegeMediaStorage) {
    *permission = PermissionGroup::kMediaLibrary;
  } else {
    LOG_WARN("Unknown Privilege!");
    return false;
  }
  return true;
}

bool ConvertToPrivileges(PermissionGroup permission,
                         std::vector<const char*>* privileges) {
  switch (permission) {
    case PermissionGroup::kCalendar:
      privileges->push_back(kPrivilegeCalendarRead);
      break;
    case PermissionGroup::kCamera:
      privileges->push_back(kPrivilegeCamera);
      break;
    case PermissionGroup::kContacts:
      privileges->push_back(kPrivilegeContactRead);
      break;
    case PermissionGroup::kLocation:
    case PermissionGroup::kLocationAlways:
    case PermissionGroup::kLocationWhenInUse:
      privileges->push_back(kPrivilegeLocation);
      break;
    case PermissionGroup::kMicrophone:
      privileges->push_back(kPrivilegeRecorder);
      break;
    case PermissionGroup::kPhone:
      privileges->push_back(kPrivilegeCall);
      break;
    case PermissionGroup::kSensors:
      privileges->push_back(kPrivilegeSensors);
      break;
    case PermissionGroup::kSMS:
      privileges->push_back(kPrivilegeMessageRead);
      break;
    case PermissionGroup::kStorage:
      privileges->push_back(kPrivilegeExternalStorage);
      break;
    case PermissionGroup::kMediaLibrary:
      privileges->push_back(kPrivilegeMediaStorage);
      break;
    default:
      LOG_WARN("Unknown Permission!");
      return false;
  }
  return true;
}

int DeterminePermissionStatus(PermissionGroup permission,
                              PermissionStatus* status) {
  std::vector<const char*> privileges;
  ConvertToPrivileges(permission, &privileges);

  if (privileges.size() == 0) {
    LOG_DEBUG("No tizen specific privileges needed for permission %d",
              permission);
    *status = PermissionStatus::kGranted;
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
          *status = PermissionStatus::kDenied;
          return result;
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
        default:
          *status = PermissionStatus::kGranted;
          break;
      }
    }
  }
  return result;
}

void OnRequestPermissionResponse(ppm_call_cause_e cause,
                                 ppm_request_result_e result,
                                 const char* privilege, void* data) {
  Param* param = (Param*)data;
  PermissionManager* manager = param->manager;
  PermissionGroup permission;

  if (cause != PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ANSWER ||
      !ConvertToPermission(privilege, &permission)) {
    // abandon a request
    LOG_ERROR("Privilege[%s] request failed with an error", privilege);
    param->is_done = true;
    return;
  }

  if (manager->RequestResults().count(permission) == 0) {
    switch (result) {
      case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
        manager->RequestResults()[permission] = PermissionStatus::kGranted;
        break;
      case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
        manager->RequestResults()[permission] = PermissionStatus::kDenied;
        break;
      case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
        manager->RequestResults()[permission] =
            PermissionStatus::kNeverAskAgain;
        break;
    }
  }
  LOG_DEBUG("permission %d status: %d", permission,
            manager->RequestResults()[permission]);
  auto location = manager->RequestResults().find(PermissionGroup::kLocation);
  if (location != manager->RequestResults().end()) {
    manager->RequestResults()[PermissionGroup::kLocationAlways] =
        location->second;
    manager->RequestResults()[PermissionGroup::kLocationWhenInUse] =
        location->second;
  }

  param->remaining_request--;
  param->is_done = true;
}
}  // namespace

PermissionManager::PermissionManager() : on_going_(false) {}

PermissionManager::~PermissionManager() {}

void PermissionManager::CheckPermissionStatus(
    PermissionGroup permission, OnPermissionChecked success_callback,
    OnPermissionError error_callback) {
  LOG_DEBUG("Check permission %d status", permission);

  PermissionStatus status;
  int result = DeterminePermissionStatus(permission, &status);
  if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
    error_callback(get_error_message(result),
                   "An error occurred when call ppm_check_permission()");
  } else {
    success_callback(status);
  }
}

void PermissionManager::RequestPermissions(
    std::vector<PermissionGroup> permissions,
    OnPermissionRequested success_callback, OnPermissionError error_callback) {
  if (on_going_) {
    error_callback("RequestPermissions - error",
                   "A request for permissions is already running");
    return;
  }
  int result;
  PermissionStatus status;
  request_results_.clear();
  std::vector<const char*> permissions_to_request;
  for (auto permission : permissions) {
    result = DeterminePermissionStatus(permission, &status);
    if (result != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      error_callback(get_error_message(result),
                     "An error occurred when call ppm_check_permission()");
      return;
    }

    if (status == PermissionStatus::kGranted) {
      if (request_results_.find(permission) == request_results_.end()) {
        LOG_DEBUG("Request permission %d result: kGranted", permission);
        request_results_[permission] = PermissionStatus::kGranted;
      }
      continue;
    }

    ConvertToPrivileges(permission, &permissions_to_request);
  }

  // no permission is needed to requested
  if (permissions_to_request.size() == 0) {
    success_callback(request_results_);
    return;
  }

  on_going_ = true;

  Param p;
  p.manager = this;
  p.remaining_request = permissions_to_request.size();

  for (size_t i = 0; i < permissions_to_request.size(); i++) {
    const char* permission = permissions_to_request[i];
    p.is_done = false;
    result =
        ppm_request_permission(permission, OnRequestPermissionResponse, &p);

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
