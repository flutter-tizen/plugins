// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_APPLICATION_UTILS_H_
#define FLUTTER_PLUGIN_APPLICATION_UTILS_H_

#include <app_common.h>
#include <app_manager.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <string>

#include "log.h"

namespace application_utils {
bool ExtractValueFromMap(const flutter::EncodableValue &arguments,
                         const char *key, std::string &out_value);
bool AppMetaDataCB(const char *key, const char *value, void *user_data);
int GetAppData(app_info_h app_info, flutter::EncodableMap &value);
}  // namespace application_utils

#endif
