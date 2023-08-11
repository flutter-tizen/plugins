// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "job_scheduler_wrapper.h"

#include <app_preference.h>

#include <utility>
#include <vector>

#include "log.h"

JobScheduler::JobScheduler() { job_scheduler_init(); }

int JobScheduler::SetJobConstraints(job_info_h job_info,
                                    const Constraints& constraints) {
  int ret = JOB_ERROR_NONE;
  if (constraints.battery_not_low) {
    ret = job_info_set_requires_battery_not_low(job_info, true);
    if (ret != JOB_ERROR_NONE) {
      LOG_ERROR("Failed to set job info battery_not_low. Error message: %s",
                get_error_message(ret));
      return ret;
    }
  }

  if (constraints.charging) {
    ret = job_info_set_requires_charging(job_info, true);
    if (ret != JOB_ERROR_NONE) {
      LOG_ERROR("Failed to set job info charging. Error message: %s",
                get_error_message(ret));
      return ret;
    }
  }

  switch (constraints.network_type) {
    case NetworkType::kConnected:
    case NetworkType::kUnmetered:
      ret = job_info_set_requires_wifi_connection(job_info, true);
      if (ret != JOB_ERROR_NONE) {
        LOG_ERROR("Failed to set job info wifi_connection. Error message: %s",
                  get_error_message(ret));
        return ret;
      }
      break;
  }
  return ret;
}

void JobScheduler::RegisterJob(const JobInfo& job_info,
                               job_service_callback_s* callback) {
  if (!callback) {
    LOG_ERROR("callback in RegisterJob should not be nullptr.");
    return;
  }

  job_info_h job_handle;
  int ret = job_info_create(&job_handle);
  if (ret != JOB_ERROR_NONE) {
    LOG_ERROR("Failed to create job info. Error message: %s",
              get_error_message(ret));
    return;
  }

  ret = SetJobConstraints(job_handle, job_info.constraints);
  if (ret != JOB_ERROR_NONE) {
    job_info_destroy(job_handle);
    return;
  }

  if (job_info.is_periodic) {
    ret = job_info_set_periodic(job_handle, job_info.frequency_seconds / 60);
    if (ret != JOB_ERROR_NONE) {
      LOG_ERROR("Failed to set job info periodic. Error message: %s",
                get_error_message(ret));
      job_info_destroy(job_handle);
      return;
    }
    ret = job_info_set_persistent(job_handle, true);
    if (ret != JOB_ERROR_NONE) {
      LOG_ERROR("Failed to set job info persistent. Error message: %s",
                get_error_message(ret));
      job_info_destroy(job_handle);
      return;
    }
  } else {
    ret = job_info_set_once(job_handle, true);
    if (ret != JOB_ERROR_NONE) {
      LOG_ERROR("Failed to set job info once. Error message: %s",
                get_error_message(ret));
      job_info_destroy(job_handle);
      return;
    }
    ret = job_info_set_persistent(job_handle, false);
    if (ret != JOB_ERROR_NONE) {
      LOG_ERROR("Failed to set job non-persistent. Error message: %s",
                get_error_message(ret));
      job_info_destroy(job_handle);
      return;
    }
  }

  ret = job_scheduler_schedule(job_handle, job_info.unique_name.c_str());
  if (ret != JOB_ERROR_NONE) {
    if (ret == JOB_ERROR_ALREADY_EXIST) {
      switch (job_info.existing_work_policy) {
        case ExistingWorkPolicy::kReplace:
        case ExistingWorkPolicy::kUpdate:
          CancelByUniqueName(job_info.unique_name);
          ret =
              job_scheduler_schedule(job_handle, job_info.unique_name.c_str());
          if (ret != JOB_ERROR_NONE) {
            LOG_ERROR("Failed to schedule job. Error message: %s",
                      get_error_message(ret));
          } else {
            SaveJobInfo(job_info);
            SetCallback(job_info.unique_name.c_str(), callback);
          }
          break;
        default:
          LOG_INFO("Job already exists but ignored. Job name: %s",
                   job_info.unique_name.c_str());
      }
    } else {
      LOG_ERROR("Failed to schedule job. Error message: %s",
                get_error_message(ret));
    }
  } else {
    SaveJobInfo(job_info);
    SetCallback(job_info.unique_name.c_str(), callback);
  }

  job_info_destroy(job_handle);
}

void JobScheduler::CancelByUniqueName(const std::string& name) {
  int ret = job_scheduler_cancel(name.c_str());
  if (ret != JOB_ERROR_NONE) {
    LOG_ERROR("Failed to cancel job with name %s. Error message: %s",
              name.c_str(), get_error_message(ret));
    return;
  }
  preference_remove(GetJobInfoKey(name).c_str());

  job_scheduler_service_remove(job_service_handles_[name]);
  job_service_handles_.erase(name);
}

void JobScheduler::CancelAll() {
  std::vector<std::string> job_names = GetAllJobs();

  job_scheduler_cancel_all();

  for (const std::string& name : job_names) {
    preference_remove(GetJobInfoKey(name).c_str());
  }

  for (const std::pair<std::string, job_service_h>& items :
       job_service_handles_) {
    job_scheduler_service_remove(items.second);
  }
  job_service_handles_.clear();
}

job_service_h JobScheduler::SetCallback(const char* job_name,
                                        job_service_callback_s* callback) {
  job_service_h service = nullptr;
  int ret = job_scheduler_service_add(job_name, callback, nullptr, &service);
  if (ret != JOB_ERROR_NONE) {
    LOG_ERROR("Failed to add service to job. Error message: %s",
              get_error_message(ret));
    return nullptr;
  }

  if (job_service_handles_.count(job_name)) {
    job_scheduler_service_remove(job_service_handles_[job_name]);
  }

  job_service_handles_[job_name] = service;
  return service;
}

std::vector<std::string> JobScheduler::GetAllJobs() {
  std::vector<std::string> jobs;

  int code = job_scheduler_foreach_job(
      [](job_info_h job_info, void* user_data) {
        char* job_id = nullptr;
        auto vec = static_cast<std::vector<std::string>*>(user_data);

        job_info_get_job_id(job_info, &job_id);
        vec->emplace_back(job_id);
        return true;
      },
      &jobs);

  return jobs;
}

void JobScheduler::SaveJobInfo(const JobInfo& job_info) {
  const std::string jobinfo_key = GetJobInfoKey(job_info.unique_name);
  const std::string jobinfo_size_key =
      kTaskInfoPreferenceSizePrefix + job_info.unique_name;
  bundle_raw* encoded_bund;
  int32_t size = 0;

  bundle* bund = bundle_create();
  AddJobInfoToBundle(bund, job_info);
  bundle_encode(bund, &encoded_bund, &size);

  preference_set_string(jobinfo_key.c_str(),
                        reinterpret_cast<char*>(encoded_bund));
  preference_set_int(jobinfo_size_key.c_str(), size);
  free(encoded_bund);
  bundle_free(bund);
}

std::optional<JobInfo> JobScheduler::LoadJobInfo(const std::string& job_name) {
  const std::string jobinfo_key = GetJobInfoKey(job_name);
  char* base64;
  int32_t size;

  preference_get_string(jobinfo_key.c_str(), &base64);
  preference_get_int((kTaskInfoPreferenceSizePrefix + job_name).c_str(), &size);

  bundle* bund = bundle_decode(reinterpret_cast<bundle_raw*>(base64), size);

  if (!bund) {
    LOG_ERROR("Failed load JobInfo %s.", job_name.c_str());
    return std::nullopt;
  }

  JobInfo job_info = GetJobInfoFromBundle(bund);
  bundle_free(bund);
  return job_info;
}

std::string JobScheduler::GetJobInfoKey(const std::string& job_name) {
  return kTaskInfoPreferencePrefix + job_name;
}
