// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "camera_event_channel.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include "log.h"

#define CAMERA_CHANNEL_NAME_BASE "flutter.io/cameraPlugin/cameraEvents"

std::string EventTypeToString(EventType type) {
  if (type == EventType::Error) {
    return "error";
  } else if (type == EventType::CameraClosing) {
    return "cameraClosing";
  }
}

CameraEventChannel::CameraEventChannel(flutter::PluginRegistrar *registrar,
                                       long event_channel_Id) {
  std::string channel_name = CAMERA_CHANNEL_NAME_BASE;
  channel_name += std::to_string(event_channel_Id);

  event_channel_ =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          registrar->messenger(), channel_name.data(),
          &flutter::StandardMethodCodec::GetInstance());

  auto event_channel_handler =
      std::make_unique<flutter::StreamHandlerFunctions<>>(
          [this](const flutter::EncodableValue *arguments,
                 std::unique_ptr<flutter::EventSink<>> &&events)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_DEBUG("OnListen");
            event_sink_ = std::move(events);
            return nullptr;
          },
          [this](const flutter::EncodableValue *arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            LOG_DEBUG("OnCancel");
            event_sink_ = nullptr;
            return nullptr;
          });
  event_channel_->SetStreamHandler(std::move(event_channel_handler));
}

void CameraEventChannel::Send(EventType event_type, std::string message) {
  if (event_sink_ == nullptr) {
    LOG_WARN("Failed to send");
    return;
  }

  flutter::EncodableMap event;
  event[flutter::EncodableValue("eventType")] = EventTypeToString(event_type);

  if (event_type == EventType::Error && !message.empty()) {
    event[flutter::EncodableValue("errorDescription")] =
        flutter::EncodableValue(message);
  }
  event_sink_->Success(flutter::EncodableValue(event));
}