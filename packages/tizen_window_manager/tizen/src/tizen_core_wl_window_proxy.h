// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_CORE_WL_WINDOW_PROXY_H_
#define FLUTTER_PLUGIN_TIZEN_CORE_WL_WINDOW_PROXY_H_

#include "window_proxy_interface.h"

class TizenCoreWlWindowProxy : public WindowProxyInterface {
 public:
  TizenCoreWlWindowProxy();
  ~TizenCoreWlWindowProxy() override;

  bool IsValid() const { return handle_ != nullptr; }

  void GetGeometry(void *window, int *x, int *y, int *width,
                   int *height) override;
  void Activate(void *window) override;
  void Lower(void *window) override;

 private:
  typedef int (*FuncGetGeometry)(void *window, int *x, int *y, int *width,
                                 int *height);
  typedef int (*FuncActivate)(void *window);
  typedef int (*FuncLower)(void *window);

  void *handle_ = nullptr;
  FuncGetGeometry get_geometry_ = nullptr;
  FuncActivate activate_ = nullptr;
  FuncLower lower_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_TIZEN_CORE_WL_WINDOW_PROXY_H_
