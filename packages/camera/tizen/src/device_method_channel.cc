// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_method_channel.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <string>

#include "log.h"

#define DEVICE_CHANNEL_NAME "flutter.io/cameraPlugin/device"

std::string EventTypeToString(DeviceEventType type) {
  if (type == DeviceEventType::kOrientationChanged) {
    return "orientation_changed";
  }
  LOG_WARN("Unknown event type!");
  return "unknown";
}

DeviceMethodChannel::DeviceMethodChannel(flutter::PluginRegistrar* registrar) {
  std::string channel_name = DEVICE_CHANNEL_NAME;
  channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar->messenger(), channel_name.c_str(),
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
