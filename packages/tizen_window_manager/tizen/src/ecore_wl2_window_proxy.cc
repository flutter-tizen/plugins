// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ecore_wl2_window_proxy.h"

#include <dlfcn.h>

#include "log.h"

typedef void (*FuncEcoreWl2WindowGeometryGet)(void *window, int *x, int *y,
                                              int *width, int *height);
typedef void (*FuncEcoreWl2WindowActivate)(void *window);
typedef void (*FuncEcoreWl2WindowRaise)(void *window);
typedef void (*FuncEcoreWl2WindowLower)(void *window);

EcoreWl2WindowProxy::EcoreWl2WindowProxy() {
  ecore_wl2_window_handle_ = dlopen("libecore_wl2.so.1", RTLD_LAZY);
  if (ecore_wl2_window_handle_ == nullptr) {
    LOG_ERROR("Failed to open ecore wl2.");
  }
}

EcoreWl2WindowProxy::~EcoreWl2WindowProxy() {
  if (ecore_wl2_window_handle_) {
    dlclose(ecore_wl2_window_handle_);
    ecore_wl2_window_handle_ = nullptr;
  }
}

void EcoreWl2WindowProxy::ecore_wl2_window_geometry_get(void *window, int *x,
                                                        int *y, int *width,
                                                        int *height) {
  if (!ecore_wl2_window_handle_) {
    LOG_ERROR("ecore_wl2_window_handle_ not valid");
    return;
  }

  FuncEcoreWl2WindowGeometryGet ecore_wl2_window_geometry_get =
      reinterpret_cast<FuncEcoreWl2WindowGeometryGet>(
          dlsym(ecore_wl2_window_handle_, "ecore_wl2_window_geometry_get"));
  if (!ecore_wl2_window_geometry_get) {
    LOG_ERROR("Fail to find ecore_wl2_window_geometry_get.");
    return;
  }
  ecore_wl2_window_geometry_get(window, x, y, width, height);
}

void EcoreWl2WindowProxy::ecore_wl2_window_activate(void *window) {
  if (!ecore_wl2_window_handle_) {
    LOG_ERROR("ecore_wl2_window_handle_ not valid");
    return;
  }

  FuncEcoreWl2WindowActivate ecore_wl2_window_activate =
      reinterpret_cast<FuncEcoreWl2WindowActivate>(
          dlsym(ecore_wl2_window_handle_, "ecore_wl2_window_activate"));
  if (!ecore_wl2_window_activate) {
    LOG_ERROR("Fail to find ecore_wl2_window_activate.");
    return;
  }
  ecore_wl2_window_activate(window);
}

void EcoreWl2WindowProxy::ecore_wl2_window_lower(void *window) {
  if (!ecore_wl2_window_handle_) {
    LOG_ERROR("ecore_wl2_window_handle_ not valid");
    return;
  }

  FuncEcoreWl2WindowLower ecore_wl2_window_lower =
      reinterpret_cast<FuncEcoreWl2WindowLower>(
          dlsym(ecore_wl2_window_handle_, "ecore_wl2_window_lower"));
  if (!ecore_wl2_window_lower) {
    LOG_ERROR("Fail to find ecore_wl2_window_lower.");
    return;
  }
  ecore_wl2_window_lower(window);
}
