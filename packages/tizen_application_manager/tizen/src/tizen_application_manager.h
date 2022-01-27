// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_APPLICATION_MANAGER_H_
#define FLUTTER_PLUGIN_TIZEN_APPLICATION_MANAGER_H_

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>

#include "application_utils.h"

class TizenApplicationManagerPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr =
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>;
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

  TizenApplicationManagerPlugin();

  virtual ~TizenApplicationManagerPlugin();

  flutter::EncodableList m_applications;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> m_events;
  bool m_registered_event_cb;

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      MethodResultPtr result);
  void RegisterObserver(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events);
  void UnregisterObserver();
  void GetCurrentId(MethodResultPtr result);
  void GetApplicationInfo(const flutter::EncodableValue &arguments,
                          MethodResultPtr result);
  void GetInstalledApplicationsInfo(MethodResultPtr result);
  void ApplicationIsRunning(const flutter::EncodableValue &arguments,
                            MethodResultPtr result);
  void SetupChannels(flutter::PluginRegistrar *registrar);
};

#endif
