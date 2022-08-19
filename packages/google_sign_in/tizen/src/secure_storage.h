// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_SECURE_STORAGE_H_
#define FLUTTER_PLUGIN_SECURE_STORAGE_H_

#include <ckmc/ckmc-type.h>

#include <optional>
#include <string>
#include <vector>

class SecureStorage {
 public:
  SecureStorage();

  ~SecureStorage();

  void Destroy();

  void SaveData(const std::string& name, const std::vector<uint8_t>& data,
                const std::vector<uint8_t>& initialization_vector);

  std::optional<std::vector<uint8_t>> GetData(const std::string& name);

  void RemoveData(const std::string& name);

 private:
  ckmc_param_list_h params_;

  std::vector<uint8_t> EncryptData(std::vector<uint8_t> data,
                                   std::vector<uint8_t> initialization_vector);

  std::vector<uint8_t> DecryptData(std::vector<uint8_t> data);
};

#endif  // FLUTTER_PLUGIN_SECURE_STORAGE_H_
