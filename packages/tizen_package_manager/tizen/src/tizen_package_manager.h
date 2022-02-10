// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PACKAGE_MANAGER_H_
#define FLUTTER_PLUGIN_PACKAGE_MANAGER_H_
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>

#include "package_manager_utils.h"

class TizenPackageManagerPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr =
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>;
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

  TizenPackageManagerPlugin();

  virtual ~TizenPackageManagerPlugin();

  flutter::EncodableList m_packages;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> m_install_events;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>
      m_uninstall_events;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> m_update_events;
  bool m_registered_event_cb;
  int m_registered_cnt;

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      MethodResultPtr result);
  void RegisterObserver(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events);
  void UnregisterObserver();
  void GetPackageInfo(const flutter::EncodableValue &arguments,
                      MethodResultPtr result);
  void GetAllPackagesInfo(MethodResultPtr result);
  void Install(const flutter::EncodableValue &arguments,
               MethodResultPtr result);
  void Uninstall(const flutter::EncodableValue &arguments,
                 MethodResultPtr result);
  void SetupChannels(flutter::PluginRegistrar *registrar);

  package_manager_h m_package_manager_h;
};

#endif
