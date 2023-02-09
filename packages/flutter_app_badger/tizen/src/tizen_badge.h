// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_BADGE_H_
#define FLUTTER_PLUGIN_TIZEN_BADGE_H_

#include <app.h>
#include <badge.h>

#include <string>

class TizenBadge {
 public:
  ~TizenBadge() = default;

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

  bool IsSupported() { return is_supported_; };

  bool Initialize();

  bool UpdateBadgeCount(int32_t count);

  bool RemoveBadge();

 private:
  std::string app_id_;
  bool is_added_ = false;
  bool is_supported_ = false;
  int last_error_ = TIZEN_ERROR_NONE;
};

#endif  // FLUTTER_PLUGIN_TIZEN_BADGE_H_
