// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_window_manager.h"

#include <flutter/encodable_value.h>
#include <flutter/standard_method_codec.h>

#include <memory>

#include "log.h"
#include "window_proxy.h"

TizenWindowManager::TizenWindowManager(void* handle)
    : window_handle_(handle), proxy_(std::make_unique<WindowProxy>()) {}

TizenWindowManager::~TizenWindowManager() {}

void TizenWindowManager::Activate() {
  if (!window_handle_) {
    LOG_ERROR("Window handle is null");
  }

  proxy_->Activate(window_handle_);
}

void TizenWindowManager::Lower() {
  if (!window_handle_) {
    LOG_ERROR("Window handle is null");
  }

  proxy_->Lower(window_handle_);
}

flutter::EncodableMap TizenWindowManager::GetGeometry() {
  flutter::EncodableMap geometry;

  if (!window_handle_) {
    LOG_ERROR("Window handle is null");
    geometry[flutter::EncodableValue("x")] = flutter::EncodableValue(0);
    geometry[flutter::EncodableValue("y")] = flutter::EncodableValue(0);
    geometry[flutter::EncodableValue("width")] = flutter::EncodableValue(0);
    geometry[flutter::EncodableValue("height")] = flutter::EncodableValue(0);
    return geometry;
  }

  int x, y, width, height;
  proxy_->GetGeometry(window_handle_, &x, &y, &width, &height);

  geometry[flutter::EncodableValue("x")] = flutter::EncodableValue(x);
  geometry[flutter::EncodableValue("y")] = flutter::EncodableValue(y);
  geometry[flutter::EncodableValue("width")] = flutter::EncodableValue(width);
  geometry[flutter::EncodableValue("height")] = flutter::EncodableValue(height);

  return geometry;
}
