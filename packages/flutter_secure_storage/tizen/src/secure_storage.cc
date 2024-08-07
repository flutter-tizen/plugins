// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "secure_storage.h"

#include <app_common.h>
#include <ckmc/ckmc-manager.h>
#include <ckmc/ckmc-type.h>

#include <algorithm>
#include <ctime>
#include <random>

constexpr char kSecureStorageAesKey[] = "SecureStorageAesKey";
constexpr size_t kInitializationVectorSize = 12;

SecureStorage::SecureStorage() { CreateAesKeyOnce(); }

SecureStorage::~SecureStorage() {}

void SecureStorage::Write(const std::string &key, const std::string &value) {
  std::vector<uint8_t> encrypted = Encrypt(value);

  ckmc_raw_buffer_s write_buffer;
  write_buffer.data = encrypted.data();
  write_buffer.size = encrypted.size();

  ckmc_policy_s policy = {
      .password = nullptr,
      .extractable = true,
  };

  ckmc_save_data(key.c_str(), write_buffer, policy);
}

std::optional<std::string> SecureStorage::Read(const std::string &key) {
  ckmc_raw_buffer_s *read_buffer;
  int ret = ckmc_get_data(key.c_str(), nullptr, &read_buffer);
  if (ret != CKMC_ERROR_NONE) {
    return std::nullopt;
  }

  std::vector<uint8_t> encrypted(read_buffer->data,
                                 read_buffer->data + read_buffer->size);
  ckmc_buffer_free(read_buffer);

  std::string decrypted = Decrypt(encrypted);
  return decrypted;
}

std::map<std::string, std::string> SecureStorage::ReadAll() {
  std::vector<std::string> keys = GetAliasList(AliasType::kData);

  std::map<std::string, std::string> map;
  for (const std::string &key : keys) {
    std::optional<std::string> value = Read(key);
    map[key] = value.value();
  }

  return map;
}

void SecureStorage::Delete(const std::string &key) {
  ckmc_remove_alias(key.c_str());
}

void SecureStorage::DeleteAll() {
  std::vector<std::string> keys = GetAliasList(AliasType::kData);
  for (std::string key : keys) {
    Delete(key);
  }
}

bool SecureStorage::ContainsKey(const std::string &key) {
  std::vector<std::string> keys = GetAliasList(AliasType::kData);
  return std::find(keys.begin(), keys.end(), key) != keys.end();
}

void SecureStorage::CreateAesKeyOnce() {
  std::vector<std::string> names = GetAliasList(AliasType::kKey);
  for (const auto &name : names) {
    if (name == kSecureStorageAesKey) {
      return;
    }
  }
  ckmc_policy_s policy = {
      .password = nullptr,
      .extractable = false,
  };
  ckmc_create_key_aes(256, kSecureStorageAesKey, policy);
}

std::vector<uint8_t> SecureStorage::Encrypt(const std::string &value) {
  if (value.empty()) {       
    return std::vector<uint8_t>{0x00};
  }

  ckmc_raw_buffer_s plain_buffer;
  plain_buffer.data =
      const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(value.c_str()));
  plain_buffer.size = value.length();

  ckmc_param_list_h params = nullptr;
  ckmc_generate_new_params(CKMC_ALGO_AES_GCM, &params);

  std::vector<uint8_t> iv = GenerateRandomVector();
  ckmc_raw_buffer_s iv_buffer;
  iv_buffer.data = iv.data();
  iv_buffer.size = iv.size();
  ckmc_param_list_set_buffer(params, CKMC_PARAM_ED_IV, &iv_buffer);

  ckmc_raw_buffer_s *encrypted_buffer = nullptr;
  ckmc_encrypt_data(params, kSecureStorageAesKey, nullptr, plain_buffer,
                    &encrypted_buffer);

  std::vector<uint8_t> encrypted(iv);
  encrypted.insert(encrypted.end(), encrypted_buffer->data,
                   encrypted_buffer->data + encrypted_buffer->size);

  ckmc_param_list_free(params);
  ckmc_buffer_free(encrypted_buffer);

  return encrypted;
}

std::string SecureStorage::Decrypt(const std::vector<uint8_t> &value) {
  if (value.size() <= kInitializationVectorSize) {
    return "";
  }
  
  std::vector<uint8_t> iv(value.begin(),
                          value.begin() + kInitializationVectorSize);
  ckmc_raw_buffer_s iv_buffer;
  iv_buffer.data = iv.data();
  iv_buffer.size = iv.size();

  ckmc_param_list_h params = nullptr;
  ckmc_generate_new_params(CKMC_ALGO_AES_GCM, &params);

  ckmc_param_list_set_buffer(params, CKMC_PARAM_ED_IV, &iv_buffer);

  std::vector<uint8_t> encrypted_value(
      value.begin() + kInitializationVectorSize, value.end());
  ckmc_raw_buffer_s encrypted_buffer;
  encrypted_buffer.data = encrypted_value.data();
  encrypted_buffer.size = encrypted_value.size();

  ckmc_raw_buffer_s *decrypted_buffer = nullptr;
  ckmc_decrypt_data(params, kSecureStorageAesKey, nullptr, encrypted_buffer,
                    &decrypted_buffer);

  std::string decrypted(decrypted_buffer->data,
                        decrypted_buffer->data + decrypted_buffer->size);

  ckmc_param_list_free(params);
  ckmc_buffer_free(decrypted_buffer);

  return decrypted;
}

std::vector<uint8_t> SecureStorage::GenerateRandomVector() {
  static std::mt19937 mt(std::random_device{}());
  static std::uniform_int_distribution<uint8_t> distrib;

  std::vector<uint8_t> vector;
  for (size_t i = 0; i < kInitializationVectorSize; i++) {
    vector.emplace_back(distrib(mt));
  }

  return vector;
}

std::vector<std::string> SecureStorage::GetAliasList(AliasType type) {
  ckmc_alias_list_s *ckmc_alias_list = nullptr;
  if (type == AliasType::kKey) {
    ckmc_get_key_alias_list(&ckmc_alias_list);
  } else if (type == AliasType::kData) {
    ckmc_get_data_alias_list(&ckmc_alias_list);
  }

  char *app_id = nullptr;
  app_get_id(&app_id);
  std::string id = app_id;
  free(app_id);

  std::vector<std::string> names;
  ckmc_alias_list_s *current = ckmc_alias_list;

  while (current != nullptr) {
    std::string name = current->alias;
    // Remove prefix of alias.
    names.push_back(name.substr(name.find(id) + id.length() + 1));
    current = current->next;
  }

  ckmc_alias_list_all_free(ckmc_alias_list);

  return names;
}
