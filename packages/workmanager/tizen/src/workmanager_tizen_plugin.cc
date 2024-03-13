// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "workmanager_tizen_plugin.h"

#include <app.h>
#include <app_event.h>
#include <app_manager.h>
#include <app_preference.h>
#include <bundle.h>
#include <device/battery.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <net_connection.h>
#include <system_info.h>
#include <unistd.h>

#include <string>

#include "extractor.h"
#include "job.h"
#include "job_scheduler_wrapper.h"
#include "log.h"
#include "utils.h"

namespace {

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;

const char *kForegroundChannelName =
    "be.tramckrijte.workmanager/foreground_channel_work_manager";
const char *kBackgroundChannelName =
    "be.tramckrijte.workmanager/background_channel_work_manager";

const char *kBgChannelInputData = "be.tramckrijte.workmanager.INPUT_DATA";
const char *kBgChannelDartTask = "be.tramckrijte.workmanager.DART_TASK";
const char *kDispatcherHandle = "WMANAGER_TIZEN_DISPATCHER_HANDLE_KEY";

const char *kNotInitializedErrMsg =
    "You have not properly initialized the Flutter WorkManager Package. "
    "You should ensure you have called the 'initialize' function first! "
    "Example: \n"
    "\n"
    "`Workmanager().initialize(\n"
    "  callbackDispatcher,\n"
    " )`"
    "\n"
    "\n"
    "The `callbackDispatcher` is a top level function. See example in "
    "repository.";

const int32_t kMinBackOffPeriodic = 15 * 60 * 1000;
const int32_t kMinBackOffOneOff = 10 * 1000;

const char *kEventName = "pass_taskinfo_event";

const char *kInvalidArg = "Invalid argument";
const char *kOperationFailed = "Operation failed/cancelled";

bool CheckAppIsRunning(const char *app_id) {
  app_context_h context;
  int ret = app_manager_get_app_context(app_id, &context);
  if (ret == APP_MANAGER_ERROR_NO_SUCH_APP) {
    return false;
  }

  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("%s", get_error_message(ret));
    return false;
  }

  app_state_e state;
  app_context_get_app_state(context, &state);

  switch (state) {
    case APP_STATE_FOREGROUND:
    case APP_STATE_BACKGROUND:
    case APP_STATE_SERVICE:
      return true;
  }
  return false;
}

void SendLaunchRequest(const char *app_id) {
  app_control_h control;
  int ret = app_control_create(&control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("%s", get_error_message(ret));
  }

  ret = app_control_set_app_id(control, app_id);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("%s", get_error_message(ret));
  }

  ret = app_control_send_launch_request(control, NULL, NULL);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("%s", get_error_message(ret));
  }

  ret = app_control_destroy(control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("%s", get_error_message(ret));
  }
}

bool CheckIsServiceApp() {
  char *app_id;
  int ret = app_manager_get_app_id(getpid(), &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get app id. Error message: %s",
              get_error_message(ret));
    return false;
  }

  app_info_h app_info;
  app_info_app_component_type_e app_type;
  app_info_create(app_id, &app_info);
  app_info_get_app_component_type(app_info, &app_type);
  free(app_id);

  return app_type == APP_INFO_APP_COMPONENT_TYPE_SERVICE_APP;
}

class WorkmanagerTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<WorkmanagerTizenPlugin>();

    is_service_app_ = CheckIsServiceApp();

    if (is_service_app_) {
      background_channel_ = std::make_unique<FlMethodChannel>(
          registrar->messenger(), kBackgroundChannelName,
          &flutter::StandardMethodCodec::GetInstance());

      background_channel_.value()->SetMethodCallHandler(
          [plugin_pointer = plugin.get()](const auto &call, auto result) {
            plugin_pointer->HandleBackgroundMethodCall(call, std::move(result));
          });

      event_handler_h handler;
      std::string service_app_id = GetAppId().value();
      std::string event_id =
          "event." + service_app_id.substr(0, service_app_id.size() - 8) + "." +
          kEventName;
      event_add_event_handler(event_id.c_str(), TaskInfoCallback, nullptr,
                              &handler);

      JobScheduler &scheduler = JobScheduler::instance();
      std::vector<std::string> job_names = scheduler.GetAllJobs();
      job_service_callback_s callback = {StartJobCallback, StopJobCallback};
      for (const std::string &name : job_names) {
        std::optional<JobInfo> info = scheduler.LoadJobInfo(name);
        if (!info.has_value()) {
          continue;
        }
        scheduler.CancelByUniqueName(name);
        scheduler.RegisterJob(info.value(), &callback);
      }
    }

    auto foreground_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), kForegroundChannelName,
        &flutter::StandardMethodCodec::GetInstance());

    foreground_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  WorkmanagerTizenPlugin() {}

  virtual ~WorkmanagerTizenPlugin() {}

 private:
  static std::optional<std::unique_ptr<FlMethodChannel>> background_channel_;
  static bool is_service_app_;
  static job_service_callback_s callback_;
  static bool is_background_initialized_;  // Only in service app

  void HandleMethodCall(const FlMethodCall &call,
                        std::unique_ptr<FlMethodResult> result) {
    const std::string method_name = call.method_name();
    const flutter::EncodableValue &arguments = *call.arguments();

    std::string app_id = GetAppId().value();
    std::string event_id = "event." + app_id + "." + kEventName;

    const std::string service_app_id = GetAppId().value() + "_service";

    if (method_name == kCancelAllTasks) {
      bundle *bund = bundle_create();
      if (!bund) {
        LOG_ERROR("Failed create bundle.");
        result->Error(kOperationFailed, "Failed Creating bundle.");
      }
      bundle_add_str(bund, kMethodNameKey, method_name.c_str());

      if (is_service_app_) {
        TaskInfoCallback(nullptr, bund, nullptr);
      } else {
        int ret = event_publish_app_event(event_id.c_str(), bund);

        if (ret != EVENT_ERROR_NONE) {
          LOG_ERROR("Failed publich app event. Error message: %s",
                    get_error_message(ret));

          result->Error(kOperationFailed, "Failed publish app event.");
          bundle_free(bund);
          return;
        }
      }
      bundle_free(bund);

      result->Success();
      return;
    }

    if (!std::holds_alternative<flutter::EncodableMap>(arguments)) {
      result->Error(kInvalidArg, "No argument provided.");
      return;
    }

    flutter::EncodableMap map = std::get<flutter::EncodableMap>(arguments);

    if (method_name == kInitialize) {
      bool is_debug_mode =
          std::get<bool>(map[flutter::EncodableValue(kIsInDebugMode)]);

      int64_t handle =
          std::get<int64_t>(map[flutter::EncodableValue(kCallbackHandle)]);

      preference_set_int(kDispatcherHandle, handle);

      if (!is_service_app_ && !CheckAppIsRunning(service_app_id.c_str())) {
        SendLaunchRequest(service_app_id.c_str());
      }

      result->Success();
    } else if (method_name == kRegisterOneOffTask ||
               method_name == kRegisterPeriodicTask) {
      const bool is_periodic = method_name == kRegisterPeriodicTask;

      bool is_debug_mode =
          std::get<bool>(map[flutter::EncodableValue(kIsInDebugMode)]);
      std::string unique_name =
          std::get<std::string>(map[flutter::EncodableValue(kUniqueName)]);
      std::string task_name =
          std::get<std::string>(map[flutter::EncodableValue(kNameValue)]);
      ExistingWorkPolicy existing_work_policy =
          ExtractExistingWorkPolicyFromMap(map);
      int32_t initial_delay_seconds =
          GetOrNullFromEncodableMap<int32_t>(&map, kInitialDelaySeconds)
              .value_or(0);
      int32_t frequency_seconds =
          GetOrNullFromEncodableMap<int32_t>(&map, kFrequencySeconds)
              .value_or(0);

      Constraints constraints_config = ExtractConstraintConfigFromMap(map);
      std::string payload =
          GetOrNullFromEncodableMap<std::string>(&map, kPayload).value_or("");
      JobInfo job_info(is_debug_mode, unique_name, task_name,
                       existing_work_policy, initial_delay_seconds,
                       constraints_config, frequency_seconds, payload,
                       is_periodic);
      bundle *bund = bundle_create();

      if (!bund) {
        LOG_ERROR("Failed create bundle.");
        result->Error(kOperationFailed, "Failed Creating bundle.");
      }

      bundle_add_str(bund, kMethodNameKey, method_name.c_str());

      AddJobInfoToBundle(bund, job_info);

      if (is_service_app_) {
        TaskInfoCallback(nullptr, bund, nullptr);
      } else {
        int ret = event_publish_app_event(event_id.c_str(), bund);
        if (ret != EVENT_ERROR_NONE) {
          LOG_ERROR("Failed publish event. Error message: %s",
                    get_error_message(ret));
          result->Error(kOperationFailed, "Error occured.");

          bundle_free(bund);
          return;
        }
      }
      bundle_free(bund);

      result->Success();
    } else if (method_name == kCancelTaskByUniqueName) {
      std::optional<std::string> name =
          GetOrNullFromEncodableMap<std::string>(&map, kCancelTaskUniqueName);
      if (!name.has_value()) {
        result->Error(kInvalidArg, "No name provided.");
        return;
      }

      bundle *bund = bundle_create();
      if (!bund) {
        LOG_ERROR("Failed create bundle.");
        result->Error(kOperationFailed, "Failed Creating bundle.");
      }

      bundle_add_str(bund, kMethodNameKey, method_name.c_str());
      bundle_add_str(bund, kCancelTaskByUniqueName, name.value().c_str());

      if (is_service_app_) {
        TaskInfoCallback(nullptr, bund, nullptr);
      } else {
        int ret = event_publish_app_event(event_id.c_str(), bund);

        if (ret != BUNDLE_ERROR_NONE) {
          LOG_ERROR("Failed publish event. : %s", get_error_message(ret));
          result->Error(kOperationFailed, "Failed Publish event.");

          bundle_free(bund);
          return;
        }
      }
      bundle_free(bund);

      result->Success();
    } else {
      result->NotImplemented();
    }
  }

  void HandleBackgroundMethodCall(const FlMethodCall &call,
                                  std::unique_ptr<FlMethodResult> result) {
    if (call.method_name() == kBackgroundChannelInitialized) {
      is_background_initialized_ = true;
    }

    result->Success();
  }

  static void RunBackgroundCallback(const std::string &job_id,
                                    const std::string &payload) {
    if (!background_channel_.has_value()) {
      LOG_ERROR("Background channel is not initialized.");
      return;
    }

    flutter::EncodableMap arg = {
        {flutter::EncodableValue(kBgChannelDartTask),
         flutter::EncodableValue(job_id)},
    };

    if (payload.size() > 0) {
      arg[flutter::EncodableValue(kBgChannelInputData)] =
          flutter::EncodableValue(payload);
    }

    background_channel_.value()->InvokeMethod(
        kOnResultSendMethod, std::make_unique<flutter::EncodableValue>(arg));
  }

  static void StartJobCallback(job_info_h job_info, void *user_data) {
    char *job_id = nullptr;
    job_info_get_job_id(job_info, &job_id);
    JobScheduler &scheduler = JobScheduler::instance();
    JobInfo task_info = scheduler.LoadJobInfo(job_id).value();

    RunBackgroundCallback(task_info.task_name, task_info.payload);
  }

  static void StopJobCallback(job_info_h job_info, void *user_data) {
    // Empty
  }

  static std::optional<std::string> GetAppId() {
    char *app_id;
    int ret = app_manager_get_app_id(getpid(), &app_id);
    if (ret == APP_MANAGER_ERROR_NONE) {
      return std::string(app_id);
    }
    return std::nullopt;
  }

  static void TaskInfoCallback(const char *event_name, bundle *event_data,
                               void *user_data) {
    char *method_name = nullptr;

    bundle_get_str(event_data, kMethodNameKey, &method_name);

    std::string method_name_str(method_name);
    JobScheduler &job_scheduler = JobScheduler::instance();

    if (method_name_str == kRegisterOneOffTask ||
        method_name_str == kRegisterPeriodicTask) {
      JobInfo job_info = GetJobInfoFromBundle(event_data);

      if (job_info.is_periodic) {
        job_service_callback_s callback = {StartJobCallback, StopJobCallback};
        job_scheduler.RegisterJob(job_info, &callback);
      } else {
        if (job_info.constraints.battery_not_low) {
          device_battery_level_e level;
          device_battery_get_level_status(&level);

          switch (level) {
            case DEVICE_BATTERY_LEVEL_LOW:
            case DEVICE_BATTERY_LEVEL_CRITICAL:
            case DEVICE_BATTERY_LEVEL_EMPTY:
              return;
          }
        }

        if (job_info.constraints.charging) {
          bool charging = false;
          device_battery_is_charging(&charging);
          if (!charging) {
            return;
          }
        }

        switch (job_info.constraints.network_type) {
          case NetworkType::kUnmetered:
          case NetworkType::kConnected:
            connection_h conn = nullptr;
            connection_wifi_state_e wifi_state;
            connection_create(&conn);
            connection_get_wifi_state(conn, &wifi_state);
            connection_destroy(conn);

            if (wifi_state != CONNECTION_WIFI_STATE_CONNECTED) {
              return;
            }
        }

        RunBackgroundCallback(job_info.task_name, job_info.payload);
      }

    } else if (method_name_str == kCancelTaskByUniqueName) {
      char *unique_name;
      int ret = bundle_get_str(event_data, kUniqueName, &unique_name);
      if (ret != BUNDLE_ERROR_NONE) {
        LOG_ERROR("Failed get unique_name from bundle.");
        return;
      }

      job_scheduler.CancelByUniqueName(unique_name);
    } else if (method_name_str == kCancelAllTasks) {
      job_scheduler.CancelAll();
    }
  }
};

std::optional<std::unique_ptr<FlMethodChannel>>
    WorkmanagerTizenPlugin::background_channel_ = std::nullopt;
bool WorkmanagerTizenPlugin::is_service_app_ = false;
bool WorkmanagerTizenPlugin::is_background_initialized_ = false;

}  // namespace

void WorkmanagerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WorkmanagerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
