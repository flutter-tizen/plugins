// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WINDOW_PROXY_INTERFACE_H_
#define FLUTTER_PLUGIN_WINDOW_PROXY_INTERFACE_H_

class WindowProxyInterface {
 public:
  virtual ~WindowProxyInterface() = default;

  virtual void GetGeometry(void *window, int *x, int *y, int *width,
                           int *height) = 0;
  virtual void Activate(void *window) = 0;
  virtual void Lower(void *window) = 0;
};

#endif  // FLUTTER_PLUGIN_WINDOW_PROXY_INTERFACE_H_
