// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "camera_method_channel.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <string>

#include "log.h"

#define CAMERA_CHANNEL_NAME_BASE "flutter.io/cameraPlugin/camera"

std::string EventTypeToString(CameraEventType type) {
  if (type == CameraEventType::kError) {
    return "error";
  } else if (type == CameraEventType::kCameraClosing) {
    return "cameraClosing";
  } else if (type == CameraEventType::kInitialized) {
    return "initialized";
  }
  LOG_WARN("Unknown event type!");
  return "unknown";
}

CameraMethodChannel::CameraMethodChannel(flutter::PluginRegistrar* registrar,
                                         long event_channel_Id) {
  std::string channel_name = CAMERA_CHANNEL_NAME_BASE;
  channel_name += std::to_string(event_channel_Id);
  channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      registrar->messenger(), channel_name.c_str(),
      &flutter::StandardMethodCodec::GetInstance());
}

void CameraMethodChannel::Send(
    CameraEventType event_type,
    std::unique_ptr<flutter::EncodableValue>&& args) {
  if (!channel_) {
    return;
  }
  channel_->InvokeMethod(EventTypeToString(event_type), std::move(args));
}
