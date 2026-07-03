// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_core_wl_window_proxy.h"

#include <dlfcn.h>

#include "log.h"

TizenCoreWlWindowProxy::TizenCoreWlWindowProxy() {
  handle_ = dlopen("libtizen-core-wl.so.0", RTLD_LAZY);
  if (handle_ == nullptr) {
    LOG_ERROR("Failed to open tizen-core-wl.");
    return;
  }

  get_geometry_ = reinterpret_cast<FuncGetGeometry>(
      dlsym(handle_, "tizen_core_wl_window_get_geometry"));
  activate_ = reinterpret_cast<FuncActivate>(
      dlsym(handle_, "tizen_core_wl_window_activate"));
  lower_ =
      reinterpret_cast<FuncLower>(dlsym(handle_, "tizen_core_wl_window_lower"));

  if (!get_geometry_ || !activate_ || !lower_) {
    LOG_ERROR("Failed to resolve tizen-core-wl symbols.");
    dlclose(handle_);
    handle_ = nullptr;
  }
}

TizenCoreWlWindowProxy::~TizenCoreWlWindowProxy() {
  if (handle_) {
    dlclose(handle_);
    handle_ = nullptr;
  }
}

void TizenCoreWlWindowProxy::GetGeometry(void *window, int *x, int *y,
                                         int *width, int *height) {
  if (!handle_) {
    LOG_ERROR("tizen-core-wl handle not valid");
    return;
  }
  get_geometry_(window, x, y, width, height);
}

void TizenCoreWlWindowProxy::Activate(void *window) {
  if (!handle_) {
    LOG_ERROR("tizen-core-wl handle not valid");
    return;
  }
  activate_(window);
}

void TizenCoreWlWindowProxy::Lower(void *window) {
  if (!handle_) {
    LOG_ERROR("tizen-core-wl handle not valid");
    return;
  }
  lower_(window);
}
