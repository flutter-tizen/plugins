// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "secure_storage.h"

#include <ckmc/ckmc-manager.h>
#include <ckmc/ckmc-type.h>

#include <cassert>

namespace {

// This value must match `_kIvSizeBytes` in `secure_storage.dart`.
constexpr size_t kIvSizeBytes = 12;

constexpr char kAesKey[] = "AesKey";

const ckmc_policy_s kDefaultKeyPolicy = {
    .password = nullptr,
    .extractable = false,
};

const ckmc_policy_s kDefaultDataPolicy = {
    .password = nullptr,
    .extractable = true,
};

enum class NameType {
  kKey,
  kData,
};

std::vector<std::string> GetNames(NameType name_type) {
  ckmc_alias_list_s *ckmc_alias_list = nullptr;
  switch (name_type) {
    case NameType::kKey:
      ckmc_get_key_alias_list(&ckmc_alias_list);
      break;
    case NameType::kData:
      ckmc_get_data_alias_list(&ckmc_alias_list);
      break;
    default:
      assert(false);
  }

  std::vector<std::string> names;
  ckmc_alias_list_s *current = ckmc_alias_list;
  while (current != nullptr) {
    std::string name = current->alias;
    // The ckmc module returns all aliases prefixed with application id.
    // For example, if 'some_alias' is saved for 'some_app',
    // the returned alias would be 'some_app some_alias'.
    // So we need to remove prefix from the returned alias.
    names.push_back(name.substr(name.find(' ') + 1));
    current = current->next;
  }

  ckmc_alias_list_all_free(ckmc_alias_list);
  return names;
}
}  // namespace

SecureStorage::SecureStorage() {
  ckmc_generate_new_params(CKMC_ALGO_AES_GCM, &params_);
  std::vector<std::string> names = GetNames(NameType::kKey);
  for (const auto &name : names) {
    if (name == kAesKey) {
      return;
    }
  }
  ckmc_create_key_aes(256, kAesKey, kDefaultKeyPolicy);
}

SecureStorage::~SecureStorage() { ckmc_param_list_free(params_); }

void SecureStorage::Destroy() {
  std::vector<std::string> names = GetNames(NameType::kData);
  for (const auto &name : names) {
    RemoveData(name);
  }
  ckmc_remove_alias(kAesKey);
}

void SecureStorage::SaveData(
    const std::string &name, const std::vector<uint8_t> &data,
    const std::vector<uint8_t> &initialization_vector) {
  std::vector<uint8_t> encrypted = EncryptData(data, initialization_vector);

  ckmc_raw_buffer_s *buffer = nullptr;
  ckmc_buffer_new(encrypted.data(), encrypted.size(), &buffer);
  int ret = ckmc_save_data(name.c_str(), *buffer, kDefaultDataPolicy);
  if (ret == CKMC_ERROR_DB_ALIAS_EXISTS) {
    RemoveData(name.c_str());
    ckmc_save_data(name.c_str(), *buffer, kDefaultDataPolicy);
  }
  ckmc_buffer_free(buffer);
}

std::optional<std::vector<uint8_t>> SecureStorage::GetData(
    const std::string &name) {
  ckmc_raw_buffer_s *buffer = nullptr;

  int ret = ckmc_get_data(name.c_str(), nullptr, &buffer);
  if (ret != CKMC_ERROR_NONE) {
    return std::nullopt;
  }
  std::vector<uint8_t> data(buffer->data, buffer->data + buffer->size);
  ckmc_buffer_free(buffer);
  return DecryptData(data);
}

void SecureStorage::RemoveData(const std::string &name) {
  ckmc_remove_alias(name.c_str());
}

std::vector<uint8_t> SecureStorage::EncryptData(
    std::vector<uint8_t> data, std::vector<uint8_t> initialization_vector) {
  ckmc_raw_buffer_s *buffer = nullptr;
  ckmc_buffer_new(initialization_vector.data(), initialization_vector.size(),
                  &buffer);
  ckmc_param_list_set_buffer(params_, CKMC_PARAM_ED_IV, buffer);
  ckmc_buffer_free(buffer);

  ckmc_buffer_new(data.data(), data.size(), &buffer);
  ckmc_raw_buffer_s *encrypted_buffer = nullptr;
  ckmc_encrypt_data(params_, kAesKey, nullptr, *buffer, &encrypted_buffer);
  ckmc_buffer_free(buffer);

  std::vector<uint8_t> encrypted(
      encrypted_buffer->data, encrypted_buffer->data + encrypted_buffer->size);
  ckmc_buffer_free(encrypted_buffer);
  encrypted.insert(encrypted.end(), initialization_vector.begin(),
                   initialization_vector.end());
  return encrypted;
}

std::vector<uint8_t> SecureStorage::DecryptData(std::vector<uint8_t> data) {
  std::vector<uint8_t> initialization_vector(data.end() - kIvSizeBytes,
                                             data.end());
  data.erase(data.end() - kIvSizeBytes, data.end());

  ckmc_raw_buffer_s *buffer = nullptr;
  ckmc_buffer_new(initialization_vector.data(), initialization_vector.size(),
                  &buffer);
  ckmc_param_list_set_buffer(params_, CKMC_PARAM_ED_IV, buffer);
  ckmc_buffer_free(buffer);

  ckmc_buffer_new(data.data(), data.size(), &buffer);
  ckmc_raw_buffer_s *decrypted_buffer = nullptr;
  ckmc_decrypt_data(params_, kAesKey, nullptr, *buffer, &decrypted_buffer);
  ckmc_buffer_free(buffer);

  std::vector<uint8_t> decrypted(
      decrypted_buffer->data, decrypted_buffer->data + decrypted_buffer->size);
  ckmc_buffer_free(decrypted_buffer);

  return decrypted;
}
