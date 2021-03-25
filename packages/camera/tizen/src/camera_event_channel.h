// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CAMERA_EVENT_CHANNEL_H_
#define FLUTTER_PLUGIN_CAMERA_EVENT_CHANNEL_H_

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/plugin_registrar.h>

enum class EventType {
  Error,
  CameraClosing,
};

class CameraEventChannel {
 public:
  CameraEventChannel(flutter::PluginRegistrar *registrar,
                     long event_channel_Id);
  void Send(EventType eventType, std::string message);

 private:
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
};

#endif
