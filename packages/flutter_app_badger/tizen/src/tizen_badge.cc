// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_badge.h"

#include "log.h"

bool TizenBadge::Initialize() {
  char *app_id = nullptr;
  int ret = app_get_id(&app_id);
  if (ret != APP_ERROR_NONE) {
    LOG_ERROR("app_get_id() failed with error %s.", get_error_message(ret));
    last_error_ = ret;
    return false;
  }
  app_id_ = app_id;
  free(app_id);

  ret = badge_add(app_id_.c_str());
  if (ret == BADGE_ERROR_NOT_SUPPORTED) {
    LOG_INFO("Badge is not supported.");
    is_supported_ = false;
    last_error_ = ret;
  } else if (ret != BADGE_ERROR_NONE && ret != BADGE_ERROR_ALREADY_EXIST) {
    LOG_ERROR("badge_add() failed with error %s.", get_error_message(ret));
    last_error_ = ret;
    return false;
  } else {
    is_supported_ = true;
  }

  is_added_ = true;
  return true;
}

bool TizenBadge::UpdateBadgeCount(int32_t count) {
  if (!is_supported_) {
    LOG_ERROR("Badge is not supported.");
    last_error_ = BADGE_ERROR_NOT_SUPPORTED;
    return false;
  }
  int ret = BADGE_ERROR_NONE;
  if (!is_added_) {
    ret = badge_add(app_id_.c_str());
    if (ret != BADGE_ERROR_NONE && ret != BADGE_ERROR_ALREADY_EXIST) {
      LOG_ERROR("badge_add() failed with error %s.", get_error_message(ret));
      last_error_ = ret;
      return false;
    }
    is_added_ = true;
  }

  ret = badge_set_count(app_id_.c_str(), count);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_count() failed with error %s.",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  ret = badge_set_display(app_id_.c_str(), 1);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_display() failed with error %s.",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }
  return true;
}

bool TizenBadge::RemoveBadge() {
  if (!is_supported_) {
    LOG_ERROR("Badge is not supported.");
    last_error_ = BADGE_ERROR_NOT_SUPPORTED;
    return false;
  }

  int ret = badge_set_display(app_id_.c_str(), 0);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_display() failed with error %s.",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  ret = badge_set_count(app_id_.c_str(), 0);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_count() failed with error %s.",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  ret = badge_remove(app_id_.c_str());
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_remove() failed with error %s.", get_error_message(ret));
    last_error_ = ret;
    return false;
  }
  is_added_ = false;
  return true;
}
