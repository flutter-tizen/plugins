// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_secure_storage.h"

#include <app_common.h>
#include <ckmc/ckmc-manager.h>
#include <ckmc/ckmc-type.h>

#include <algorithm>

#include "log.h"

FlutterSecureStorage::FlutterSecureStorage() {}

FlutterSecureStorage::~FlutterSecureStorage() {}

void FlutterSecureStorage::Write(const std::string &key,
                                 const std::string &value) {
  ckmc_raw_buffer_s write_buffer;
  write_buffer.data = (unsigned char *)value.c_str();
  write_buffer.size = value.size();

  ckmc_policy_s policy = {
      .password = nullptr,
      .extractable = true,
  };

  int ret = ckmc_save_data(key.c_str(), write_buffer, policy);
  if (ret != CKMC_ERROR_NONE) {
    LOG_ERROR("Failed to write: key[%s] value[%s]", key.c_str(), value.c_str());
  }
}

std::optional<std::string> FlutterSecureStorage::Read(const std::string &key) {
  ckmc_raw_buffer_s *read_buffer;
  int ret = ckmc_get_data(key.c_str(), nullptr, &read_buffer);
  if (ret != CKMC_ERROR_NONE) {
    return std::nullopt;
  }

  std::string read_string((char *)read_buffer->data, read_buffer->size);
  ckmc_buffer_free(read_buffer);
  return read_string;
}

flutter::EncodableMap FlutterSecureStorage::ReadAll() {
  std::vector<std::string> keys = GetKeys();
  flutter::EncodableMap key_value_pairs;

  for (std::string key : keys) {
    std::optional<std::string> value = Read(key);
    if (value.has_value()) {
      key_value_pairs[flutter::EncodableValue(key)] =
          flutter::EncodableValue(value.value());
    }
  }

  return key_value_pairs;
}

void FlutterSecureStorage::Delete(const std::string &key) {
  ckmc_remove_alias(key.c_str());
}

void FlutterSecureStorage::DeleteAll() {
  std::vector<std::string> keys = GetKeys();
  for (std::string key : keys) {
    Delete(key);
  }
}

bool FlutterSecureStorage::ContainsKey(const std::string &key) {
  std::vector<std::string> keys = GetKeys();
  return std::find(keys.begin(), keys.end(), key) != keys.end();
}

std::vector<std::string> FlutterSecureStorage::GetKeys(bool is_data) {
  ckmc_alias_list_s *ckmc_alias_list = nullptr;
  if (is_data) {
    ckmc_get_data_alias_list(&ckmc_alias_list);
  } else {
    ckmc_get_key_alias_list(&ckmc_alias_list);
  }

  char *app_id = nullptr;
  app_get_id(&app_id);
  std::string id = app_id;
  free(app_id);

  std::vector<std::string> names;
  ckmc_alias_list_s *current = ckmc_alias_list;

  while (current != nullptr) {
    std::string name = current->alias;
    names.push_back(name.substr(name.find(id) + id.length() + 1));
    current = current->next;
  }

  ckmc_alias_list_all_free(ckmc_alias_list);

  return names;
}