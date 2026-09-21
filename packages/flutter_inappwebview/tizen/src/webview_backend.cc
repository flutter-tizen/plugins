// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview_backend.h"

#include <glib.h>

#include <algorithm>
#include <atomic>
#include <mutex>
#include <vector>

#include "buffer_pool.h"
#include "log.h"

namespace {

struct PendingTeardown {
  std::shared_ptr<BufferPool> pool;
  std::function<void()> destroy;
  std::atomic<bool> completed{false};
};

std::mutex g_mutex;
std::vector<std::shared_ptr<PendingTeardown>> g_pending_teardowns;

void CompletePendingTeardown(const std::shared_ptr<PendingTeardown>& pending) {
  bool expected = false;
  if (pending->completed.compare_exchange_strong(expected, true) &&
      pending->destroy) {
    pending->destroy();
  }
  std::lock_guard<std::mutex> lock(g_mutex);
  auto it = std::find(g_pending_teardowns.begin(), g_pending_teardowns.end(),
                      pending);
  if (it != g_pending_teardowns.end()) {
    g_pending_teardowns.erase(it);
  }
}

}  // namespace

std::function<void()> WebViewBackend::RegisterPendingTeardown(
    std::shared_ptr<BufferPool> pool, std::function<void()> destroy) {
  auto pending = std::make_shared<PendingTeardown>();
  pending->pool = std::move(pool);
  pending->destroy = std::move(destroy);
  {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_pending_teardowns.push_back(pending);
  }
  return [pending]() { CompletePendingTeardown(pending); };
}

void WebViewBackend::FlushPendingTeardowns() {
  constexpr gint64 kDeadlineUsec = 2 * G_USEC_PER_SEC;
  const gint64 deadline = g_get_monotonic_time() + kDeadlineUsec;
  for (;;) {
    bool deadline_passed = g_get_monotonic_time() >= deadline;
    std::vector<std::shared_ptr<PendingTeardown>> snapshot;
    {
      std::lock_guard<std::mutex> lock(g_mutex);
      if (g_pending_teardowns.empty()) {
        return;
      }
      if (deadline_passed) {
        snapshot = g_pending_teardowns;
      }
    }
    if (deadline_passed) {
      LOG_WARN("Forcing %zu pending teardown(s) past deadline",
               snapshot.size());
      for (auto& pending : snapshot) {
        CompletePendingTeardown(pending);
      }
      continue;
    }
    if (!g_main_context_iteration(g_main_context_default(), FALSE)) {
      g_usleep(1000);
    }
  }
}
