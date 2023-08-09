// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
#define FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_

#include <job_scheduler.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "job.h"
#include "options.h"

constexpr const char* kTaskInfoPreferencePrefix = "WmTaskInfo_";
constexpr const char* kTaskInfoPreferenceSizePrefix = "WmTaskInfoSize_";

class JobScheduler {
 public:
  JobScheduler(JobScheduler const&) = delete;
  JobScheduler& operator=(JobScheduler const&) = delete;

  static JobScheduler& instance() {
    static JobScheduler instance;
    return instance;
  }

  int SetJobConstraints(job_info_h job_handle, const Constraints& constraints);

  void RegisterJob(const JobInfo& job_info,
                   job_service_callback_s* callback = nullptr);

  void CancelByUniqueName(const std::string& name);

  void CancelAll();

  job_service_h SetCallback(const char* job_name,
                            job_service_callback_s* callback);

  std::vector<std::string> GetAllJobs();

  std::optional<JobInfo> LoadJobInfo(const std::string& job_name);

 private:
  std::unordered_map<std::string, job_service_h> job_service_handles_;

  JobScheduler();
  ~JobScheduler() = default;

  void SaveJobInfo(const JobInfo& job_info);

  std::string GetJobInfoKey(const std::string& job_name);
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_JOB_SCHEDULER_WRAPPER_H_
