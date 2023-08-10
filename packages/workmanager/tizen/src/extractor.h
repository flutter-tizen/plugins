// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_
#define FLUTTER_PLUGIN_WORKMANAGER_EXTRACTOR_H_

#include <flutter/encodable_value.h>

#include <string>

#include "options.h"

constexpr const char* kInitialDelaySeconds = "initialDelaySeconds";
constexpr const char* kExistingWorkpolicy = "existingWorkPolicy";

constexpr const char* kNetworkType = "networkType";
constexpr const char* kBatteryNotLow = "requiresBatteryNotLow";
constexpr const char* kCharging = "requiresCharging";
constexpr const char* kDeviceidle = "requiresDeviceIdle";
constexpr const char* kStorageNotLow = "requiresStorageNotLow";

// NetworkType
constexpr const char* kConnected = "connected";
constexpr const char* kMetered = "metered";
constexpr const char* kNotRequired = "not_required";
constexpr const char* kNotRoaming = "not_roaming";
constexpr const char* kUnmetered = "unmetered";
constexpr const char* kTemporarilyUnmetered = "temporarily_unmetered";

// ExistingWorkPolicy
constexpr const char* kReplace = "replace";
constexpr const char* kKeep = "keep";
constexpr const char* kAppend = "append";
constexpr const char* kUpdate = "update";

ExistingWorkPolicy StringToExistingWorkPolicy(const std::string& str);

NetworkType StringToNetworkType(const std::string& str);

ExistingWorkPolicy ExtractExistingWorkPolicyFromMap(
    const flutter::EncodableMap& map);

NetworkType ExtractNetworkTypeFromMap(const flutter::EncodableMap& args);

Constraints ExtractConstraintConfigFromMap(const flutter::EncodableMap& map);

#endif
