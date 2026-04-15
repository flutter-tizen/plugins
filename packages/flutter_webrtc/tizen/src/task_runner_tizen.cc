// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_runner_tizen.h"

TaskRunnerTizen::TaskRunnerTizen() = default;

TaskRunnerTizen::~TaskRunnerTizen() = default;

void TaskRunnerTizen::EnqueueTask(TaskClosure task) {
  TaskClosure* task_ptr = new TaskClosure(std::move(task));
  ecore_main_loop_thread_safe_call_async(RunTask, task_ptr);
}

void TaskRunnerTizen::RunTask(void* data) {
  TaskClosure* task_ptr = static_cast<TaskClosure*>(data);
  if (task_ptr) {
    (*task_ptr)();
    delete task_ptr;
  }
}
