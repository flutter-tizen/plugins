// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_dispatcher.h"

#include <Ecore.h>

MessageDispatcher::MessageDispatcher() { ecore_init(); }
MessageDispatcher::~MessageDispatcher() { ecore_shutdown(); }

void MessageDispatcher::dispatchTaskOnMainThread(std::function<void()> fn) {
  struct Param {
    std::function<void()> fn;
  };

  Param* p = new Param;
  p->fn = fn;

  ecore_main_loop_thread_safe_call_sync(
      [](void* data) -> void* {
        Param* p = (Param*)data;
        p->fn();
        delete p;
        return nullptr;
      },
      p);
}
