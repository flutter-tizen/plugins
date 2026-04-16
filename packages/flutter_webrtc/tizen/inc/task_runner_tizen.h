// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef PACKAGES_FLUTTER_WEBRTC_TASK_RUNNER_TIZEN_H_
#define PACKAGES_FLUTTER_WEBRTC_TASK_RUNNER_TIZEN_H_

#include <mutex>
#include <queue>

#include <Ecore.h>

#include "task_runner.h"

class TaskRunnerTizen : public TaskRunner {
 public:
  TaskRunnerTizen();
  ~TaskRunnerTizen() override;

  void EnqueueTask(TaskClosure task) override;

 private:
  static void RunTask(void* data);
  std::mutex tasks_mutex_;
  std::queue<TaskClosure> tasks_;
};

#endif  // PACKAGES_FLUTTER_WEBRTC_TASK_RUNNER_TIZEN_H_