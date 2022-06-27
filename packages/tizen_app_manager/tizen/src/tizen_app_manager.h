// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_APP_MANAGER_H_
#define FLUTTER_PLUGIN_TIZEN_APP_MANAGER_H_

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>

#include "tizen_app_info.h"

typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;

class TizenAppManagerPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr = std::unique_ptr<FlMethodResult>;
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

  TizenAppManagerPlugin();

  virtual ~TizenAppManagerPlugin();

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        MethodResultPtr result);
  void RegisterObserver(std::unique_ptr<FlEventSink> &&events);
  void UnregisterObserver();
  void GetCurrentId(MethodResultPtr result);
  void GetApplicationInfo(const flutter::EncodableValue &arguments,
                          MethodResultPtr result);
  void GetInstalledApplicationsInfo(MethodResultPtr result);
  void ApplicationIsRunning(const flutter::EncodableValue &arguments,
                            MethodResultPtr result);
  void SetupChannels(flutter::PluginRegistrar *registrar);
  bool ExtractValueFromMap(const flutter::EncodableValue &arguments,
                           const char *key, std::string &out_value);

  std::unique_ptr<FlEventSink> launch_events_;
  std::unique_ptr<FlEventSink> terminate_events_;
  flutter::EncodableList applications_;
  bool has_registered_observer_;
  int registered_cnt_;
};

#endif
