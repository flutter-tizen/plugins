// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_FLUTTER_SECURE_STORAGE_H_
#define FLUTTER_PLUGIN_FLUTTER_SECURE_STORAGE_H_

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class SecureStorage {
 public:
  SecureStorage();
  ~SecureStorage();

  void Write(const std::string &key, const std::string &value);

  std::optional<std::string> Read(const std::string &key);

  std::map<std::string, std::string> ReadAll();

  void Delete(const std::string &key);

  void DeleteAll();

  bool ContainsKey(const std::string &key);

 private:
  enum class AliasType {
    kKey,
    kData,
  };

  void CreateAesKeyOnce();

  std::string Encrypt(const std::string &value);

  std::string Decrypt(const std::string &value);

  std::vector<unsigned char> GenerateRandomVector();

  std::vector<std::string> GetAliasList(AliasType type);
};

#endif  // FLUTTER_PLUGIN_FLUTTER_SECURE_STORAGE_H_
