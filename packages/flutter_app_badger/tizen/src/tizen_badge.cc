// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_badge.h"

#include "log.h"

TizenBadge::~TizenBadge() {
  int ret = badge_remove(app_id_);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_remove() failed with error %d.", ret);
  }
}

bool TizenBadge::Initialize() {
  int ret = app_get_id(&app_id_);
  if (ret != APP_ERROR_NONE) {
    LOG_ERROR("app_get_id() failed with error %d.", ret);
    return false;
  }

  ret = badge_add(app_id_);
  if (ret == BADGE_ERROR_NOT_SUPPORTED) {
    LOG_ERROR("badge is not supported.");
    is_supported_ = false;
  } else if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_add() failed with error %d.", ret);
    return false;
  }
  return true;
}

bool TizenBadge::UpdateBadgeCount(int count) {
  if (!is_supported_) {
    LOG_ERROR("badge is not supported.");
    return false;
  }

  int ret = badge_set_display(app_id_, 1);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_display() failed with error %d.", ret);
    return false;
  }

  ret = badge_set_count(app_id_, count);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_count() failed with error %d.", ret);
    return false;
  }
  return true;
}

bool TizenBadge::RemoveBadge() {
  if (!is_supported_) {
    LOG_ERROR("badge is not supported.");
    return false;
  }

  int ret = badge_set_count(app_id_, 0);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_count() failed with error %d.", ret);
    return false;
  }

  ret = badge_set_display(app_id_, 0);
  if (ret != BADGE_ERROR_NONE) {
    LOG_ERROR("badge_set_display() failed with error %d.", ret);
    return false;
  }
  return true;
}
