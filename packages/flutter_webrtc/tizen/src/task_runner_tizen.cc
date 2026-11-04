// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "task_runner_tizen.h"

TaskRunnerTizen::TaskRunnerTizen() = default;

TaskRunnerTizen::~TaskRunnerTizen() = default;

void TaskRunnerTizen::EnqueueTask(TaskClosure task) {
  std::lock_guard<std::mutex> lock(tasks_mutex_);
  tasks_.push(std::move(task));
  g_idle_add_full(G_PRIORITY_DEFAULT, RunTask, this, nullptr);
}

gboolean TaskRunnerTizen::RunTask(gpointer data) {
  TaskRunnerTizen* runner = static_cast<TaskRunnerTizen*>(data);
  std::queue<TaskClosure> tasks;
  {
    std::lock_guard<std::mutex> lock(runner->tasks_mutex_);
    tasks.swap(runner->tasks_);
  }
  while (!tasks.empty()) {
    tasks.front()();
    tasks.pop();
  }
  return G_SOURCE_REMOVE;
}
