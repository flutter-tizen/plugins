// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orientation_manager.h"

#include <flutter/encodable_value.h>

#include <string>

#include "device_method_channel.h"
#include "log.h"

std::string EventTypeToString(OrientationType type) {
  if (type == OrientationType::kPortraitUp) {
    return "portraitUp";
  } else if (type == OrientationType::kLandscapeLeft) {
    return "landscapeLeft";
  } else if (type == OrientationType::kPortraitDown) {
    return "portraitDown";
  } else if (type == OrientationType::kLandscapeRight) {
    return "landscapeRight";
  }
  LOG_WARN("Unknown event type!");
  return "unknown";
}

OrientationManager::OrientationManager(
    DeviceMethodChannel* device_method_channel,
    OrientationType lens_orientation, bool is_front_lens_facing)
    : device_method_channel_(device_method_channel),
      lens_orientation_(lens_orientation),
      is_front_lens_facing_(is_front_lens_facing) {
  LOG_DEBUG("is_front_lens_facing_[%s]",
            is_front_lens_facing_ ? "true" : "false");
}

OrientationManager::~OrientationManager() {}

OrientationType OrientationManager::ConvertTargetOrientation(
    OrientationType orientation_event_type) {
  int degree = (int)orientation_event_type;
  if (is_front_lens_facing_) {
    degree = 180 - degree;
  }
  int target = (degree + (int)lens_orientation_ + 360) % 360;
  return (OrientationType)target;
}

void OrientationManager::SendOrientation(OrientationType orientation) {
  std::string orientation_str = EventTypeToString((OrientationType)orientation);

  LOG_DEBUG("Send Orientation [%d] : %s", orientation, orientation_str.data());

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
            LOG_ERROR("app_event_get_device_orientation fail - error : %s",
                      get_error_message(error));
            return;
          }

          OrientationManager* self = (OrientationManager*)data;
          if (self->last_device_orientation_ == device_orientation) {
            // ignore
            return;
          }
          self->last_device_orientation_ = device_orientation;
          auto target_orientation = self->ConvertTargetOrientation(
              (OrientationType)device_orientation);
          self->SendOrientation((OrientationType)target_orientation);
        },
        this);

    LOG_ERROR_IF(error != APP_ERROR_NONE,
                 "ui_app_add_event_handler fail - error : %s",
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
                 "ui_app_remove_event_handler fail - error : %s",
                 get_error_message(error));
    event_handler_ = nullptr;
  } else {
    LOG_WARN("OrientationManager already stopped!");
  }
}
