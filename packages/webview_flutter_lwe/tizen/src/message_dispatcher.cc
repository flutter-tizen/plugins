// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "message_dispatcher.h"

#include <Ecore.h>

MessageDispatcher::MessageDispatcher() { ecore_init(); }
MessageDispatcher::~MessageDispatcher() { ecore_shutdown(); }

void MessageDispatcher::dispatchTaskOnMainThread(std::function<void()>&& fn) {
  struct Param {
    std::function<void()> fn;
  };
  Param* p = new Param({std::move(fn)});

  ecore_main_loop_thread_safe_call_async(
      [](void* data) -> void {
        ecore_timer_add(
            0.0,
            [](void* data) -> Eina_Bool {
              auto* p = static_cast<Param*>(data);
              p->fn();
              delete p;
              return ECORE_CALLBACK_CANCEL;
            },
            data);
      },
      p);
}
