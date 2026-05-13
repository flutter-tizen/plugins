// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_runner_tizen.h"

TaskRunnerTizen::TaskRunnerTizen() = default;

TaskRunnerTizen::~TaskRunnerTizen() = default;

void TaskRunnerTizen::EnqueueTask(TaskClosure task) {
  std::lock_guard<std::mutex> lock(tasks_mutex_);
  tasks_.push(std::move(task));
  ecore_main_loop_thread_safe_call_async(RunTask, this);
}

void TaskRunnerTizen::RunTask(void* data) {
  TaskRunnerTizen* runner = static_cast<TaskRunnerTizen*>(data);
  std::lock_guard<std::mutex> lock(runner->tasks_mutex_);
  while (!runner->tasks_.empty()) {
    TaskClosure task = std::move(runner->tasks_.front());
    runner->tasks_.pop();
    task();
  }
}
