// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_dispatcher.h"

#include <Ecore.h>

MessageDispatcher::MessageDispatcher() { ecore_init(); }
MessageDispatcher::~MessageDispatcher() { ecore_shutdown(); }

void MessageDispatcher::dispatchTaskOnMainThread(std::function<void()>&& fn) {
  ecore_main_loop_thread_safe_call_sync(
      [](void* data) -> void* {
        auto fn = static_cast<std::function<void()>*>(data);
        if (fn) (*fn)();
        return nullptr;
      },
      &fn);
}
