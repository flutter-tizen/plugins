// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_FLUTTER_SECURE_STORAGE_H_
#define FLUTTER_PLUGIN_FLUTTER_SECURE_STORAGE_H_

#include <flutter/encodable_value.h>

#include <optional>
#include <string>
#include <vector>

class FlutterSecureStorage {
 public:
  FlutterSecureStorage();
  ~FlutterSecureStorage();

  void Write(const std::string &key, const std::string &value);

  std::optional<std::string> Read(const std::string &key);

  flutter::EncodableMap ReadAll();

  void Delete(const std::string &key);

  void DeleteAll();

  bool ContainsKey(const std::string &key);

 private:
  std::vector<std::string> GetKeys(bool is_data = true);
};

#endif  // FLUTTER_PLUGIN_FLUTTER_SECURE_STORAGE_H_
