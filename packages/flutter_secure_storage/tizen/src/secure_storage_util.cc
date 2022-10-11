// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "secure_storage_util.h"

#include <app_common.h>
#include <ckmc/ckmc-manager.h>

std::vector<std::string> SecureStorageUtil::GetAliasList(AliasType type) {
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