// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_dispatcher.h"

#include <glib.h>

MessageDispatcher::MessageDispatcher() {}
MessageDispatcher::~MessageDispatcher() {}

void MessageDispatcher::dispatchTaskOnMainThread(std::function<void()>&& fn) {
  struct Param {
    std::function<void()> fn;
  };
  Param* p = new Param({std::move(fn)});

  g_idle_add_full(
      G_PRIORITY_DEFAULT,
      [](gpointer data) -> gboolean {
        auto* p = static_cast<Param*>(data);
        p->fn();
        return G_SOURCE_REMOVE;
      },
      p, [](gpointer data) { delete static_cast<Param*>(data); });
}
