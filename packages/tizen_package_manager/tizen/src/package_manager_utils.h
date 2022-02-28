// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PACKAGE_MANAGER_UTILS_H_
#define FLUTTER_PLUGIN_PACKAGE_MANAGER_UTILS_H_

#include <app_common.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <package_manager.h>

#include <map>
#include <string>

#include "log.h"

namespace package_manager_utils {
bool ExtractValueFromMap(const flutter::EncodableValue &arguments,
                         const char *key, std::string &out_value);
int GetPackageData(package_info_h package_info, flutter::EncodableMap &value);

std::string StorageTypeToString(package_info_installed_storage_type_e value);
std::string PacakgeEventTypeToString(package_manager_event_type_e type);
std::string PacakgeEventStateToString(package_manager_event_state_e state);
}  // namespace package_manager_utils

#endif
