// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orientation_event_listener.h"

#include <flutter/encodable_value.h>

#include <string>

#include "device_method_channel.h"
#include "log.h"

std::string EventTypeToString(OrientationEventType type) {
  if (type == OrientationEventType::PortraitUp) {
    return "portraitUp";
  } else if (type == OrientationEventType::LandscapeLeft) {
    return "landscapeLeft";
  } else if (type == OrientationEventType::PortraitDown) {
    return "portraitDown";
  } else if (type == OrientationEventType::LandscapeRight) {
    return "landscapeRight";
  }
  LOG_WARN("Unknown event type!");
  return "unknown";
}

OrientationEventListner::OrientationEventListner(
    DeviceMethodChannel* device_method_channel)
    : device_method_channel_(device_method_channel) {}

OrientationEventListner::~OrientationEventListner() {}

void OrientationEventListner::Start() {
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

          OrientationEventListner* self = (OrientationEventListner*)data;
          std::string orientation =
              EventTypeToString((OrientationEventType)device_orientation);

          LOG_DEBUG("orientation changed[%d] : %s", device_orientation,
                    orientation.data());

          flutter::EncodableMap map;
          map[flutter::EncodableValue("orientation")] =
              flutter::EncodableValue(orientation);

          auto value = std::make_unique<flutter::EncodableValue>(map);
          self->device_method_channel_->Send(
              DeviceEventType::OrientationChanged, std::move(value));
        },
        this);

    LOG_ERROR_IF(error != APP_ERROR_NONE,
                 "ui_app_add_event_handler fail - error : %s",
                 get_error_message(error));
  } else {
    LOG_WARN("OrientationEventListner already started!");
  }
}

void OrientationEventListner::Stop() {
  LOG_DEBUG("enter");
  if (event_handler_) {
    int error = ui_app_remove_event_handler(event_handler_);
    LOG_ERROR_IF(error != APP_ERROR_NONE,
                 "ui_app_remove_event_handler fail - error : %s",
                 get_error_message(error));
    event_handler_ = nullptr;
  } else {
    LOG_WARN("OrientationEventListner already stopped!");
  }
}
