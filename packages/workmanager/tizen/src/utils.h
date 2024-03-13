// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WORKMANAGER_UTILS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_UTILS_H_

#include <flutter/encodable_value.h>

#include <optional>

template <typename T>
bool GetValueFromEncodableMap(const flutter::EncodableMap *map, const char *key,
                              T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

template <typename T>
std::optional<T> GetOrNullFromEncodableMap(const flutter::EncodableMap *map,
                                           const char *key) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      return *value;
    }
  }
  return std::nullopt;
}

#endif  // FLUTTER_PLUGIN_WORKMANAGER_UTILS_H_
