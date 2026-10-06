// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_backend_factory.h"

#include <system_info.h>

#include <cstdio>
#include <cstdlib>

#include "ewk_webview_backend.h"
#include "log.h"
#include "wv_webview_backend.h"

namespace {

enum class BackendKind { kEwk, kEwkWrapper, kWvStandalone };

BackendKind DefaultBackendForPlatform() {
  int major = 0, minor = 0;
  char* value = nullptr;
  int ret = system_info_get_platform_string(
      "http://tizen.org/feature/platform.version", &value);
  if (ret == SYSTEM_INFO_ERROR_NONE && value) {
    std::sscanf(value, "%d.%d", &major, &minor);
  }
  if (value) {
    free(value);
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

enum class WvEngineState { kNotStarted, kInitialized, kFailed };

WvEngineState g_wv_engine_state = WvEngineState::kNotStarted;

}  // namespace

std::unique_ptr<WebViewBackend> WebViewBackendFactory::Create(
    WebViewBackend::Delegate* delegate) {
  BackendKind kind = kSelectedBackend;
  if (kind != BackendKind::kEwk) {
    return std::make_unique<WvWebViewBackend>(delegate);
  }
  return std::make_unique<EwkWebViewBackend>(delegate);
}

void WebViewBackendFactory::InitializeEngine() {
  if (kSelectedBackend == BackendKind::kEwk) {
    EwkWebViewBackend::GlobalInitialize();
  }
}

bool WebViewBackendFactory::EnsureEngineInitialized(bool engine_policy) {
  if (kSelectedBackend == BackendKind::kEwk ||
      g_wv_engine_state == WvEngineState::kInitialized) {
    return true;
  }
  // Do not retry wv_init() on a possibly half-initialized engine.
  if (g_wv_engine_state == WvEngineState::kFailed) {
    return false;
  }
  if (!WvWebViewBackend::GlobalInitialize(
          kSelectedBackend == BackendKind::kWvStandalone, engine_policy)) {
    LOG_ERROR("wv_init() failed; engine not started.");
    g_wv_engine_state = WvEngineState::kFailed;
    return false;
  }
  g_wv_engine_state = WvEngineState::kInitialized;
  return true;
}

void WebViewBackendFactory::ShutdownEngine() {
  if (kSelectedBackend != BackendKind::kEwk) {
    if (g_wv_engine_state == WvEngineState::kInitialized) {
      WvWebViewBackend::GlobalShutdown();
      g_wv_engine_state = WvEngineState::kNotStarted;
    }
    return;
  }
  EwkWebViewBackend::GlobalShutdown();
}
