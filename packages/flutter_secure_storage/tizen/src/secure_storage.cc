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
  std::string encrypt = Encrypt(value);

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

std::optional<std::string> SecureStorage::Read(const std::string &key) {
  ckmc_raw_buffer_s *read_buffer;
  int ret = ckmc_get_data(key.c_str(), nullptr, &read_buffer);
  if (ret != CKMC_ERROR_NONE) {
    return std::nullopt;
  }

  std::string read_string(read_buffer->data,
                          read_buffer->data + read_buffer->size);
  ckmc_buffer_free(read_buffer);

  std::string decrypt = Decrypt(read_string);
  return decrypt;
}

std::map<std::string, std::string> SecureStorage::ReadAll() {
  std::vector<std::string> keys = GetAliasList(AliasType::kData);

  std::map<std::string, std::string> key_value_pairs;
  for (std::string key : keys) {
    std::optional<std::string> value = Read(key);
    key_value_pairs[key] = value.value();
  }

  return key_value_pairs;
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

std::string SecureStorage::Encrypt(const std::string &value) {
  ckmc_raw_buffer_s plaintext;
  plaintext.data = const_cast<unsigned char *>(
      reinterpret_cast<const unsigned char *>(value.c_str()));
  plaintext.size = value.length();

  ckmc_param_list_h params = nullptr;
  ckmc_generate_new_params(CKMC_ALGO_AES_GCM, &params);

  std::vector<unsigned char> iv = GenerateRandomVector();

  ckmc_raw_buffer_s iv_buffer;
  iv_buffer.data = iv.data();
  iv_buffer.size = iv.size();
  ckmc_param_list_set_buffer(params, CKMC_PARAM_ED_IV, &iv_buffer);

  ckmc_raw_buffer_s *encrypted = nullptr;
  ckmc_encrypt_data(params, kSecureStorageAesKey, nullptr, plaintext,
                    &encrypted);

  std::string encrypted_value(encrypted->data,
                              encrypted->data + encrypted->size);

  ckmc_param_list_free(params);
  ckmc_buffer_free(encrypted);

  std::string iv_str(iv.data(), iv.data() + iv.size());
  std::string combind = iv_str + encrypted_value;

  return combind;
}

std::string SecureStorage::Decrypt(const std::string &value) {
  std::string encrypted_value = value.substr(kInitializationVectorSize);
  ckmc_raw_buffer_s encrypted;
  encrypted.data = const_cast<unsigned char *>(
      reinterpret_cast<const unsigned char *>(encrypted_value.c_str()));
  encrypted.size = encrypted_value.length();

  ckmc_param_list_h params = nullptr;
  ckmc_generate_new_params(CKMC_ALGO_AES_GCM, &params);

  std::string iv = value.substr(0, kInitializationVectorSize);

  ckmc_raw_buffer_s iv_buffer;
  iv_buffer.data = const_cast<unsigned char *>(
      reinterpret_cast<const unsigned char *>(iv.c_str()));
  iv_buffer.size = iv.size();
  ckmc_param_list_set_buffer(params, CKMC_PARAM_ED_IV, &iv_buffer);

  ckmc_raw_buffer_s *decrypted = nullptr;
  ckmc_decrypt_data(params, kSecureStorageAesKey, nullptr, encrypted,
                    &decrypted);

  std::string decrypted_value(decrypted->data,
                              decrypted->data + decrypted->size);

  ckmc_param_list_free(params);
  ckmc_buffer_free(decrypted);

  return decrypted_value;
}

std::vector<unsigned char> SecureStorage::GenerateRandomVector() {
  static std::mt19937 mt{std::random_device{}()};
  static std::uniform_int_distribution<> distrib(0, 255);

  std::vector<unsigned char> vector;
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
