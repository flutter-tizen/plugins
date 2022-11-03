// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_BADGE_H_
#define FLUTTER_PLUGIN_TIZEN_BADGE_H_

#include <app.h>
#include <badge.h>

class TizenBadge {
 public:
  TizenBadge() = default;
  ~TizenBadge();

  bool IsSupported() { return is_supported_; };

  bool Initialize();

  bool UpdateBadgeCount(int count);

  bool RemoveBadge();

 private:
  char* app_id_;
  bool is_supported_ = true;
};

#endif  // FLUTTER_PLUGIN_TIZEN_BADGE_H_
