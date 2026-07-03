// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WINDOW_PROXY_H_
#define FLUTTER_PLUGIN_WINDOW_PROXY_H_

#include <memory>

#include "window_proxy_interface.h"

class WindowProxy : public WindowProxyInterface {
 public:
  WindowProxy();
  ~WindowProxy() override = default;

  void GetGeometry(void *window, int *x, int *y, int *width,
                   int *height) override;
  void Activate(void *window) override;
  void Lower(void *window) override;

 private:
  std::unique_ptr<WindowProxyInterface> impl_;
};

#endif  // FLUTTER_PLUGIN_WINDOW_PROXY_H_
