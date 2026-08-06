// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "window_proxy.h"

#include <system_info.h>

#include <cstdlib>
#include <memory>
#include <utility>

#include "ecore_wl2_window_proxy.h"
#include "log.h"
#include "tizen_core_wl_window_proxy.h"

namespace {

constexpr int kTizenCoreWlMinMajorVersion = 11;

int GetPlatformMajorVersion() {
  char *version = nullptr;
  int ret = system_info_get_platform_string(
      "http://tizen.org/feature/platform.version", &version);
  if (ret != SYSTEM_INFO_ERROR_NONE || version == nullptr) {
    LOG_ERROR("Failed to get platform version. error: %d", ret);
    return 0;
  }

  int major = std::atoi(version);
  free(version);
  return major;
}

}  // namespace

WindowProxy::WindowProxy() {
  if (GetPlatformMajorVersion() >= kTizenCoreWlMinMajorVersion) {
    auto proxy = std::make_unique<TizenCoreWlWindowProxy>();
    if (proxy->IsValid()) {
      LOG_INFO("Using tizen-core-wl backend.");
      impl_ = std::move(proxy);
      return;
    }
    LOG_INFO("tizen-core-wl unavailable, falling back to ecore_wl2 backend.");
  }
  impl_ = std::make_unique<EcoreWl2WindowProxy>();
}

void WindowProxy::GetGeometry(void *window, int *x, int *y, int *width,
                              int *height) {
  impl_->GetGeometry(window, x, y, width, height);
}

void WindowProxy::Activate(void *window) { impl_->Activate(window); }

void WindowProxy::Lower(void *window) { impl_->Lower(window); }
