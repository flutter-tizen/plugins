// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CAMERA_EVENT_CHANNEL_H_
#define FLUTTER_PLUGIN_CAMERA_EVENT_CHANNEL_H_

#include <flutter/encodable_value.h>
#include <flutter/event_sink.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>

enum class CameraEventType {
  Error,
  CameraClosing,
  Initialized,
};

class CameraMethodChannel {
 public:
  CameraMethodChannel(flutter::PluginRegistrar* registrar,
                      long event_channel_Id);
  void Send(CameraEventType eventType,
            std::unique_ptr<flutter::EncodableValue>&& args);

 private:
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
};

#endif
