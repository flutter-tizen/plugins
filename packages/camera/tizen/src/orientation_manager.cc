// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orientation_manager.h"

#include <flutter/encodable_value.h>

#include <string>

#include "device_method_channel.h"
#include "log.h"

bool OrientationTypeToString(OrientationType orientation_type,
                             std::string& orientation) {
  switch (orientation_type) {
    case OrientationType::kPortraitUp:
      orientation = "portraitUp";
      return true;
    case OrientationType::kLandscapeLeft:
      orientation = "landscapeLeft";
      return true;
    case OrientationType::kPortraitDown:
      orientation = "portraitDown";
      return true;
    case OrientationType::kLandscapeRight:
      orientation = "landscapeRight";
      return true;
    default:
      LOG_WARN("Unknown OrientationType!");
      return false;
  }
}

bool StringToOrientationType(std::string orientation,
                             OrientationType& orientation_type) {
  if (orientation == "portraitUp") {
    orientation_type = OrientationType::kPortraitUp;
    return true;
  } else if (orientation == "landscapeLeft") {
    orientation_type = OrientationType::kLandscapeLeft;
    return true;
  } else if (orientation == "portraitDown") {
    orientation_type = OrientationType::kPortraitDown;
    return true;
  } else if (orientation == "landscapeRight") {
    orientation_type = OrientationType::kLandscapeRight;
    return true;
  }
  LOG_WARN("Unknown OrientationType!");
  return false;
}

OrientationManager::OrientationManager(
    DeviceMethodChannel* device_method_channel,
    OrientationType lens_orientation, bool is_front_lens_facing)
    : device_method_channel_(device_method_channel),
      lens_orientation_(lens_orientation),
      is_front_lens_facing_(is_front_lens_facing) {
  LOG_DEBUG("is_front_lens_facing_[%s]",
            is_front_lens_facing_ ? "true" : "false");

  // Send initial orientation
  last_device_orientation_ = GetDeviceOrientationType();
  target_orientation_ = ConvertOrientation(last_device_orientation_);
  SendOrientation(target_orientation_);
}

OrientationManager::~OrientationManager() {}

OrientationType OrientationManager::ConvertOrientation(
    OrientationType orientation_event_type, bool to_target /* = true */) {
  int degree = static_cast<int>(orientation_event_type);
  if (is_front_lens_facing_) {
    degree = 180 + degree;
  }
  int target =
      (degree + (static_cast<int>(lens_orientation_) * (to_target ? 1 : -1))) %
      360;
  return (OrientationType)target;
}

OrientationType OrientationManager::GetDeviceOrientationType() {
  app_device_orientation_e orientation = app_get_device_orientation();
  return (OrientationType)orientation;
}

void OrientationManager::SendOrientation(OrientationType orientation) {
  std::string orientation_str;
  if (!OrientationTypeToString(orientation, orientation_str)) {
    LOG_WARN("Unknown orientation type, failed to send orientation!");
    return;
  }

  LOG_DEBUG("Send Orientation [%d] : %s", orientation, orientation_str.c_str());

  flutter::EncodableMap map;
  map[flutter::EncodableValue("orientation")] =
      flutter::EncodableValue(orientation_str);

  auto value = std::make_unique<flutter::EncodableValue>(map);
  device_method_channel_->Send(DeviceEventType::kOrientationChanged,
                               std::move(value));
}

void OrientationManager::Start() {
  LOG_DEBUG("enter");
  if (event_handler_ == nullptr) {
    int error = ui_app_add_event_handler(
        &event_handler_, APP_EVENT_DEVICE_ORIENTATION_CHANGED,
        [](app_event_info_h event_info, void* data) {
          LOG_DEBUG("enter");
          app_device_orientation_e device_orientation;
          int error =
              app_event_get_device_orientation(event_info, &device_orientation);
          if (error != APP_ERROR_NONE) {
            LOG_ERROR("app_event_get_device_orientation fail - error[%d]: %s",
                      error, get_error_message(error));
            return;
          }

          OrientationManager* self = (OrientationManager*)data;
          OrientationType orientation =
              static_cast<OrientationType>(device_orientation);
          if (self->last_device_orientation_ == orientation) {
            // ignore
            return;
          }
          self->last_device_orientation_ = orientation;
          self->target_orientation_ = self->ConvertOrientation(orientation);
          self->SendOrientation(self->target_orientation_);
        },
        this);

    LOG_ERROR_IF(error != APP_ERROR_NONE,
                 "ui_app_add_event_handler fail - error[%d]: %s", error,
                 get_error_message(error));
  } else {
    LOG_WARN("OrientationManager already started!");
  }
}

void OrientationManager::Stop() {
  LOG_DEBUG("enter");
  if (event_handler_) {
    int error = ui_app_remove_event_handler(event_handler_);
    LOG_ERROR_IF(error != APP_ERROR_NONE,
                 "ui_app_remove_event_handler fail - error[%d]: %s", error,
                 get_error_message(error));
    event_handler_ = nullptr;
  } else {
    LOG_WARN("OrientationManager already stopped!");
  }
}
