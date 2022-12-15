// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_method_channel.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <string>

#include "log.h"

std::string EventTypeToString(DeviceEventType type) {
  if (type == DeviceEventType::kOrientationChanged) {
    return "orientation_changed";
  }
  LOG_WARN("Unknown event type!");
  return "unknown";
}

DeviceMethodChannel::DeviceMethodChannel(flutter::PluginRegistrar* registrar) {
  channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar->messenger(), "plugins.flutter.io/camera_tizen/fromPlatform",
      &flutter::StandardMethodCodec::GetInstance());
}

void DeviceMethodChannel::Send(
    DeviceEventType event_type,
    std::unique_ptr<flutter::EncodableValue>&& args) {
  if (!channel_) {
    return;
  }
  channel_->InvokeMethod(EventTypeToString(event_type), std::move(args));
}
