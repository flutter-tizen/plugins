// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_TIZEN_PLUGINS_WRAPPER_WAKELOCK_PLUS_H_
#define FLUTTER_TIZEN_PLUGINS_WRAPPER_WAKELOCK_PLUS_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int ftpw_wakelock_plus_screensaver_reset_timeout(void);
int ftpw_wakelock_plus_screensaver_override_reset(bool onoff);

#ifdef __cplusplus
}
#endif

#endif
