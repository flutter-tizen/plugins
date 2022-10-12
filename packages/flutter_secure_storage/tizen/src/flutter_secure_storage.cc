// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_secure_storage.h"

#include <ckmc/ckmc-manager.h>
#include <ckmc/ckmc-type.h>

#include <algorithm>

#include "log.h"
#include "secure_storage_util.h"

FlutterSecureStorage::FlutterSecureStorage() {
  cipher_ = std::make_unique<Cipher>("FlutterSecureStorageAesKey", 12);
}

FlutterSecureStorage::~FlutterSecureStorage() {}

void FlutterSecureStorage::Write(const std::string &key,
                                 const std::string &value) {
  std::string encrypt = cipher_->Encrypt(value);

  ckmc_raw_buffer_s write_buffer;
  write_buffer.data = const_cast<unsigned char *>(
      reinterpret_cast<const unsigned char *>(encrypt.c_str()));
  write_buffer.size = encrypt.size();

  ckmc_policy_s policy = {
      .password = nullptr,
      .extractable = true,
  };

  ckmc_save_data(key.c_str(), write_buffer, policy);
}

std::optional<std::string> FlutterSecureStorage::Read(const std::string &key) {
  ckmc_raw_buffer_s *read_buffer;
  int ret = ckmc_get_data(key.c_str(), nullptr, &read_buffer);
  if (ret != CKMC_ERROR_NONE) {
    return std::nullopt;
  }

  std::string read_string(read_buffer->data,
                          read_buffer->data + read_buffer->size);
  ckmc_buffer_free(read_buffer);

  std::string decrypt = cipher_->Decrypt(read_string);
  return decrypt;
}

flutter::EncodableMap FlutterSecureStorage::ReadAll() {
  std::vector<std::string> keys =
      SecureStorageUtil::GetAliasList(AliasType::kData);
  flutter::EncodableMap key_value_pairs;

  for (std::string key : keys) {
    std::optional<std::string> value = Read(key);
    key_value_pairs[flutter::EncodableValue(key)] =
        flutter::EncodableValue(value.value());
  }

  return key_value_pairs;
}

void FlutterSecureStorage::Delete(const std::string &key) {
  ckmc_remove_alias(key.c_str());
}

void FlutterSecureStorage::DeleteAll() {
  std::vector<std::string> keys =
      SecureStorageUtil::GetAliasList(AliasType::kData);
  for (std::string key : keys) {
    Delete(key);
  }
}

bool FlutterSecureStorage::ContainsKey(const std::string &key) {
  std::vector<std::string> keys =
      SecureStorageUtil::GetAliasList(AliasType::kData);
  return std::find(keys.begin(), keys.end(), key) != keys.end();
}
