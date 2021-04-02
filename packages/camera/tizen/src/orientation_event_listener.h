// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_ORIENTATION_EVENT_LISTENER_H_
#define FLUTTER_PLUGIN_ORIENTATION_EVENT_LISTENER_H_

#include <app.h>

class DeviceMethodChannel;

enum class OrientationEventType {
  PortraitUp = APP_DEVICE_ORIENTATION_0,
  LandscapeLeft = APP_DEVICE_ORIENTATION_90,
  PortraitDown = APP_DEVICE_ORIENTATION_180,
  LandscapeRight = APP_DEVICE_ORIENTATION_270,
};

class OrientationEventListner {
 public:
  OrientationEventListner(DeviceMethodChannel* device_method_channel);
  ~OrientationEventListner();
  void Start();
  void Stop();

 private:
  app_event_handler_h event_handler_{nullptr};
  DeviceMethodChannel* device_method_channel_{nullptr};
};
#endif
