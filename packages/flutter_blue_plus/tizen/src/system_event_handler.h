// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_BLUE_PLUS_TIZEN_SYSTEM_EVENT_HANDLER_H
#define FLUTTER_BLUE_PLUS_TIZEN_SYSTEM_EVENT_HANDLER_H

#include <app_event.h>
#include <bundle.h>

#include <any>
#include <cassert>
#include <functional>
#include <map>
#include <string>
#include <variant>

#include "log.h"

namespace flutter_blue_plus_tizen {

class SystemEventHandler {
 public:
  using MapType = std::map<std::string, std::any>;

  using SystemEventCallback = std::function<void(MapType)>;

  SystemEventHandler(std::string event_name);

  SystemEventHandler(std::string event_name, SystemEventCallback callback);

  void SetCallback(SystemEventCallback);

  ~SystemEventHandler();

 private:
  event_handler_h handle_ = nullptr;
  SystemEventCallback callback_;
  std::string event_name_;
};

}  // namespace flutter_blue_plus_tizen
#endif  // FLUTTER_BLUE_PLUS_TIZEN_SYSTEM_EVENT_HANDLER_H
