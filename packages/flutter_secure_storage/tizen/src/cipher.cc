// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cipher.h"

#include <ckmc/ckmc-manager.h>
#include <ckmc/ckmc-type.h>

#include <ctime>
#include <random>

#include "secure_storage_util.h"

constexpr size_t kInitializationVectorSize = 12;

Cipher::Cipher(std::string aes_key_name, size_t initialization_vector_size)
    : aes_key_name_(aes_key_name),
      initialization_vector_size_(initialization_vector_size) {
  CreateAesKeyOnce();
}

Cipher::~Cipher() {}

void Cipher::CreateAesKeyOnce() {
  std::vector<std::string> names =
      SecureStorageUtil::GetAliasList(AliasType::kKey);
  for (const auto &name : names) {
    if (name == aes_key_name_) {
      return;
    }
  }
  ckmc_policy_s policy = {
      .password = nullptr,
      .extractable = false,
  };
  ckmc_create_key_aes(256, aes_key_name_.c_str(), policy);
}

std::string Cipher::Encrypt(const std::string &value) {
  ckmc_raw_buffer_s plaintext;
  plaintext.data = (unsigned char *)value.c_str();
  plaintext.size = value.length();

  ckmc_param_list_h params = nullptr;
  ckmc_generate_new_params(CKMC_ALGO_AES_GCM, &params);

  std::vector<unsigned char> iv = GenerateRandomVector();

  ckmc_raw_buffer_s iv_buffer;
  iv_buffer.data = iv.data();
  iv_buffer.size = iv.size();
  ckmc_param_list_set_buffer(params, CKMC_PARAM_ED_IV, &iv_buffer);

  ckmc_raw_buffer_s *encrypted = nullptr;
  ckmc_encrypt_data(params, aes_key_name_.c_str(), nullptr, plaintext,
                    &encrypted);

  std::string encrypted_value((char *)encrypted->data, encrypted->size);

  ckmc_param_list_free(params);
  ckmc_buffer_free(encrypted);

  std::string iv_str((char *)iv.data(), iv.size());
  std::string combind = iv_str + encrypted_value;

  return combind;
}

std::string Cipher::Decrypt(const std::string &value) {
  std::string encrypted_value = value.substr(initialization_vector_size_);
  ckmc_raw_buffer_s encrypted;
  encrypted.data = (unsigned char *)encrypted_value.c_str();
  encrypted.size = encrypted_value.length();

  ckmc_param_list_h params = nullptr;
  ckmc_generate_new_params(CKMC_ALGO_AES_GCM, &params);

  std::string iv = value.substr(0, initialization_vector_size_);

  ckmc_raw_buffer_s iv_buffer;
  iv_buffer.data = (unsigned char *)iv.c_str();
  iv_buffer.size = iv.size();
  ckmc_param_list_set_buffer(params, CKMC_PARAM_ED_IV, &iv_buffer);

  ckmc_raw_buffer_s *decrypted = nullptr;
  ckmc_decrypt_data(params, aes_key_name_.c_str(), nullptr, encrypted,
                    &decrypted);

  std::string decrypted_value((char *)decrypted->data, decrypted->size);

  ckmc_param_list_free(params);
  ckmc_buffer_free(decrypted);

  return decrypted_value;
}

std::vector<unsigned char> Cipher::GenerateRandomVector() {
  static std::mt19937 mt{std::random_device{}()};
  static std::uniform_int_distribution<> distrib(0, 255);

  std::vector<unsigned char> vector;
  for (size_t i = 0; i < initialization_vector_size_; i++) {
    vector.emplace_back(distrib(mt));
  }

  return vector;
}