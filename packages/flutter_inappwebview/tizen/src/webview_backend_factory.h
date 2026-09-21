// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_BACKEND_FACTORY_H_
#define FLUTTER_PLUGIN_WEBVIEW_BACKEND_FACTORY_H_

#include <memory>

#include "webview_backend.h"

class WebViewBackendFactory {
 public:
  static std::unique_ptr<WebViewBackend> Create(
      WebViewBackend::Delegate* delegate);

  static void InitializeEngine();

  static void ShutdownEngine();
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_BACKEND_FACTORY_H_
