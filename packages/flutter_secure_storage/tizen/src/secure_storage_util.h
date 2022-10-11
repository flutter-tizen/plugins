// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_SECURE_STORAGE_UTIL_H_
#define FLUTTER_PLUGIN_SECURE_STORAGE_UTIL_H_

#include <string>
#include <vector>

enum class AliasType {
  kKey,
  kData,
};

class SecureStorageUtil {
 public:
  static std::vector<std::string> GetAliasList(AliasType type);
};

#endif  // FLUTTER_PLUGIN_SECURE_STORAGE_UTIL_H_
