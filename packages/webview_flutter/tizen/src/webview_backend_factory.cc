// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_backend_factory.h"

#include <cstdio>
#include <cstdlib>

#include "ewk_webview_backend.h"
#include "log.h"
#include "wv_webview_backend.h"

namespace {

enum class BackendKind { kEwk, kEwkWrapper, kWvStandalone };

BackendKind DefaultBackendForPlatform() {
  int major = 0, minor = 0;
  if (const char* value = std::getenv("TIZEN_API_VERSION")) {
    std::sscanf(value, "%d.%d", &major, &minor);
  }
  if (major >= 11) {
    return BackendKind::kWvStandalone;
  }
  if (major == 10 && minor >= 1) {
    return BackendKind::kEwkWrapper;
  }
  return BackendKind::kEwk;
}

BackendKind DetectBackend() {
  BackendKind selected = DefaultBackendForPlatform();
  if (selected == BackendKind::kWvStandalone) {
    LOG_INFO("WebView backend: WV (standalone mode).");
  } else if (selected == BackendKind::kEwkWrapper) {
    LOG_INFO("WebView backend: EWK wrapper mode (WV API).");
  }
  return selected;
}

const BackendKind kSelectedBackend = DetectBackend();

bool g_wv_engine_initialized = false;

}  // namespace

std::unique_ptr<WebViewBackend> WebViewBackendFactory::Create(
    WebViewBackend::Delegate* delegate) {
  BackendKind kind = kSelectedBackend;
  if (kind != BackendKind::kEwk) {
    if (!g_wv_engine_initialized) {
      LOG_ERROR("WV engine is not initialized; cannot create WebView.");
      return nullptr;
    }
    return std::make_unique<WvWebViewBackend>(delegate);
  }
  return std::make_unique<EwkWebViewBackend>(delegate);
}

void WebViewBackendFactory::InitializeEngine() {
  BackendKind kind = kSelectedBackend;
  if (kind != BackendKind::kEwk) {
    if (!WvWebViewBackend::GlobalInitialize(kind ==
                                            BackendKind::kWvStandalone)) {
      LOG_ERROR("wv_init() failed; engine not started.");
      return;
    }
    g_wv_engine_initialized = true;
    return;
  }
  EwkWebViewBackend::GlobalInitialize();
}

void WebViewBackendFactory::ShutdownEngine() {
  if (kSelectedBackend != BackendKind::kEwk) {
    if (g_wv_engine_initialized) {
      WvWebViewBackend::GlobalShutdown();
      g_wv_engine_initialized = false;
    }
    return;
  }
  EwkWebViewBackend::GlobalShutdown();
}
