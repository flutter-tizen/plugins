// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extractor.h"

#include <flutter/encodable_value.h>
#include <flutter/method_call.h>

#include <string>

#include "options.h"
#include "utils.h"

ExistingWorkPolicy StringToExistingWorkPolicy(const std::string &str) {
  if (str == kReplace) {
    return ExistingWorkPolicy::kReplace;
  } else if (str == kAppend) {
    return ExistingWorkPolicy::kAppend;
  } else if (str == kUpdate) {
    return ExistingWorkPolicy::kUpdate;
  }
  return ExistingWorkPolicy::kKeep;
}

NetworkType StringToNetworkType(const std::string &str) {
  if (str == kConnected) {
    return NetworkType::kConnected;
  } else if (str == kMetered) {
    return NetworkType::kMetered;
  } else if (str == kNotRoaming) {
    return NetworkType::kNotRoaming;
  } else if (str == kUnmetered) {
    return NetworkType::kUnmetered;
  } else if (str == kTemporarilyUnmetered) {
    return NetworkType::kTemporarilyUnmetered;
  }
  return NetworkType::kNotRequired;
}

ExistingWorkPolicy ExtractExistingWorkPolicyFromMap(
    const flutter::EncodableMap &map) {
  std::string value;
  GetValueFromEncodableMap<std::string>(&map, kExistingWorkpolicy, value);
  return StringToExistingWorkPolicy(value);
}

NetworkType ExtractNetworkTypeFromMap(const flutter::EncodableMap &args) {
  std::optional<std::string> value =
      GetOrNullFromEncodableMap<std::string>(&args, kNetworkType);
  if (!value.has_value()) {
    return NetworkType::kNotRequired;
  }

  return StringToNetworkType(value.value());
}

Constraints ExtractConstraintConfigFromMap(const flutter::EncodableMap &map) {
  NetworkType network_type = ExtractNetworkTypeFromMap(map);
  bool battery_not_low =
      GetOrNullFromEncodableMap<bool>(&map, kBatteryNotLow).value_or(false);
  bool charging =
      GetOrNullFromEncodableMap<bool>(&map, kCharging).value_or(false);
  bool device_idle =
      GetOrNullFromEncodableMap<bool>(&map, kDeviceidle).value_or(false);
  bool storage_not_low =
      GetOrNullFromEncodableMap<bool>(&map, kStorageNotLow).value_or(false);

  return Constraints(network_type, battery_not_low, charging, storage_not_low);
}
