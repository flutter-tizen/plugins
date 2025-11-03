// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_ECORE_WL2_WINDOW_PROXY_H_
#define FLUTTER_PLUGIN_ECORE_WL2_WINDOW_PROXY_H_

class EcoreWl2WindowProxy {
 public:
  EcoreWl2WindowProxy();
  ~EcoreWl2WindowProxy();
  void ecore_wl2_window_geometry_get(void *window, int *x, int *y, int *width,
                                     int *height);

  void ecore_wl2_window_activate(void *window);
  void ecore_wl2_window_lower(void *window);

 private:
  void *ecore_wl2_window_handle_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_ECORE_WL2_WINDOW_PROXY_H_
