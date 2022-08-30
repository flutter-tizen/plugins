// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "system_event_handler.h"

#include <stdexcept>
#include <utility>

#include "log.h"
#include "tizen_error.h"

namespace flutter_blue_plus_tizen {

SystemEventHandler::SystemEventHandler(std::string event_name)
    : event_name_(std::move(event_name)) {
  auto ret = event_add_event_handler(
      event_name_.c_str(),
      [](const char* event_name, bundle* event_data, void* user_data) {
        auto& state_handler = *static_cast<SystemEventHandler*>(user_data);

        MapType map;

        struct Scope {
          MapType& map;
          bundle* event_data;
        } scope{map, event_data};

        bundle_foreach(
            event_data,
            [](const char* key, const int type_raw, const bundle_keyval_t* kv,
               void* user_data) {
              bundle_type type = static_cast<bundle_type>(type_raw);

              auto& scope = *static_cast<Scope*>(user_data);

              if (type == bundle_type::BUNDLE_TYPE_STR) {
                char* string;
                auto ret = bundle_get_str(scope.event_data, key, &string);
                LOG_ERROR("bundle_get_str: %s", get_error_message(ret));

                scope.map[key] = std::string(string);
              } else {
                /*
                 * TODO(JRazek): may implement other types(string array, byte
                 * array etc) in the future
                 * https://docs.tizen.org/application/native/api/mobile/6.5/group__CORE__LIB__BUNDLE__MODULE.html#gab4ba87b3aebc54170c0ac760e921a851
                 */
                throw std::logic_error("feature unimplemented!");
              }
            },
            &scope);

        assert(event_name == state_handler.event_name_);

        if (state_handler.callback_) state_handler.callback_(std::move(map));
      },
      this, &handle_);
  LOG_ERROR("event_add_event_handler: %s", get_error_message(ret));
}

SystemEventHandler::SystemEventHandler(std::string event_name,
                                       SystemEventCallback callback)
    : SystemEventHandler(std::move(event_name)) {
  SetCallback(std::move(callback));
}

void SystemEventHandler::SetCallback(SystemEventCallback callback) {
  callback_ = std::move(callback);
}

SystemEventHandler::~SystemEventHandler() {
  if (handle_) {
    auto ret = event_remove_event_handler(handle_);
    LOG_ERROR("event_add_event_handler %s", get_error_message(ret));
  }
}

}  // namespace flutter_blue_plus_tizen
