// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_TIZEN_PLUGINS_WRAPPER_TIZEN_LOG_H_
#define FLUTTER_TIZEN_PLUGINS_WRAPPER_TIZEN_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

int ftpw_tizen_log_dlog_print(int priority, const char *tag,
                              const char *message);

#ifdef __cplusplus
}
#endif

#endif  // FLUTTER_TIZEN_PLUGINS_WRAPPER_TIZEN_LOG_H_
