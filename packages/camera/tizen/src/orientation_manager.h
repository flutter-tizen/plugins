// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_ORIENTATION_EVENT_LISTENER_H_
#define FLUTTER_PLUGIN_ORIENTATION_EVENT_LISTENER_H_

#include <app.h>

#include <string>

class DeviceMethodChannel;

enum class OrientationType {
  kPortraitUp = APP_DEVICE_ORIENTATION_0,
  kLandscapeLeft = APP_DEVICE_ORIENTATION_90,
  kPortraitDown = APP_DEVICE_ORIENTATION_180,
  kLandscapeRight = APP_DEVICE_ORIENTATION_270,
};

bool StringToOrientationType(std::string orientation,
                             OrientationType& orientation_type);
bool OrientationTypeToString(OrientationType orientation_type,
                             std::string& orientation);
class OrientationManager {
 public:
  OrientationManager(DeviceMethodChannel* device_method_channel,
                     OrientationType lens_orientation,
                     bool is_front_lens_facing);
  ~OrientationManager();

  OrientationType ConvertOrientation(OrientationType orientation_event_type,
                                     bool to_target = true);
  OrientationType GetDeviceOrientationType();
  OrientationType GetTargetOrientationType() { return target_orientation_; }
  void SendOrientation(OrientationType orientation);
  void Start();
  void Stop();

 private:
  app_event_handler_h event_handler_{nullptr};
  DeviceMethodChannel* device_method_channel_{nullptr};
  OrientationType lens_orientation_{OrientationType::kPortraitUp};
  bool is_front_lens_facing_{false};
  OrientationType last_device_orientation_{OrientationType::kPortraitUp};
  OrientationType target_orientation_{OrientationType::kPortraitUp};
};
#endif
