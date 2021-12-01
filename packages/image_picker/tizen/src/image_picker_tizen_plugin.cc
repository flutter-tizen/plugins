// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_picker_tizen_plugin.h"

#include <app_control.h>
#include <assert.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#ifndef TV_PROFILE
#include <privacy_privilege_manager.h>
#endif

#include <memory>
#include <string>

#include "image_resize.h"
#include "log.h"

enum class ImageSource {
  // Opens up the device camera, letting the user to take a new picture.
  kCamera,
  // Opens the user's photo gallery.
  kGallery,
};

class ImagePickerTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/image_picker",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<ImagePickerTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  ImagePickerTizenPlugin() {}

  virtual ~ImagePickerTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();
    const auto &arguments = *method_call.arguments();

    if (result_) {
      SendResultWithError("Already active", "Cancelled by a second request.");
      return;
    }
    result_ = std::move(result);
    multi_image_ = method_name == "pickMultiImage";

    ImageSource source = ImageSource::kGallery;
    if (method_name == "pickImage" || method_name == "pickMultiImage") {
      double width = 0.0, height = 0.0;
      int32_t quality = 0;
      if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
        flutter::EncodableMap values =
            std::get<flutter::EncodableMap>(arguments);
        auto s = values[flutter::EncodableValue("source")];
        if (std::holds_alternative<int32_t>(s)) {
          source = (ImageSource)std::get<int32_t>(s);
        }
        auto w = values[flutter::EncodableValue("maxWidth")];
        if (std::holds_alternative<double>(w)) {
          width = std::get<double>(w);
        }
        auto h = values[flutter::EncodableValue("maxHeight")];
        if (std::holds_alternative<double>(h)) {
          height = std::get<double>(h);
        }
        auto q = values[flutter::EncodableValue("imageQuality")];
        if (std::holds_alternative<int32_t>(q)) {
          quality = std::get<int32_t>(q);
        }
        image_resize_.SetSize((unsigned int)width, (unsigned int)height,
                              quality);
      } else {
        SendResultWithError("Invalid arguments");
        return;
      }

      if (source == ImageSource::kCamera) {
        // TODO: we need to check this feature after webcam is prepared
        SendResultWithError("Not supported on this device");
      } else if (source == ImageSource::kGallery) {
        SetContentMimeType("image");
        CheckPermissionAndPickContent();
      } else {
        SendResultWithError("Invalid image source");
      }
    } else if (method_name == "pickVideo") {
      if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
        flutter::EncodableMap values =
            std::get<flutter::EncodableMap>(arguments);
        auto s = values[flutter::EncodableValue("source")];
        if (std::holds_alternative<int32_t>(s)) {
          source = (ImageSource)std::get<int32_t>(s);
        }
      } else {
        SendResultWithError("Invalid arguments");
        return;
      }

      if (source == ImageSource::kCamera) {
        // TODO: we need to check this feature after webcam is prepared
        SendResultWithError("Not supported on this device");
      } else if (source == ImageSource::kGallery) {
        SetContentMimeType("video");
        CheckPermissionAndPickContent();
      } else {
        SendResultWithError("Invalid video source");
      }
    } else {
      result_->NotImplemented();
      result_ = nullptr;
    }
  }

  void CheckPermissionAndPickContent() {
#ifndef TV_PROFILE
    const char *privilege = "http://tizen.org/privilege/mediastorage";

    ppm_check_result_e permission;
    int ret = ppm_check_permission(privilege, &permission);
    if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      LOG_ERROR("ppm_check_permission fail! [%d]", ret);
    } else {
      switch (permission) {
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
          LOG_INFO("ppm_check_permission success!");
          PickContent();
          return;
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
          ret = ppm_request_permission(
              privilege,
              [](ppm_call_cause_e cause, ppm_request_result_e result,
                 const char *privilege, void *data) -> void {
                auto *plugin = (ImagePickerTizenPlugin *)data;
                assert(plugin);

                if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
                  LOG_ERROR("ppm_request_permission error! [%d]", result);
                  plugin->SendResultWithError("Permission denied");
                  return;
                }

                plugin->PickContent();
              },
              this);
          if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
            LOG_ERROR("ppm_request_permission fail! [%d]", ret);
            break;
          }
          return;
        default:
          LOG_ERROR("ppm_check_permission deny!");
          break;
      }
    }
    SendResultWithError("Permission denied");
#else
    PickContent();
#endif
  }

  void PickContent() {
    if (mime_type_.empty()) {
      return;
    }

    app_control_h handle = nullptr;
#define RET_IF_ERROR(ret)                                             \
  if (ret != APP_CONTROL_ERROR_NONE) {                                \
    SendResultWithError(std::to_string(ret), get_error_message(ret)); \
    if (handle) {                                                     \
      app_control_destroy(handle);                                    \
    }                                                                 \
    return;                                                           \
  }
    int ret = app_control_create(&handle);
    RET_IF_ERROR(ret);

    ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_PICK);
    RET_IF_ERROR(ret);

    ret = app_control_add_extra_data(handle, APP_CONTROL_DATA_SELECTION_MODE,
                                     multi_image_ ? "multiple" : "single");
    RET_IF_ERROR(ret);

    ret = app_control_set_mime(handle, mime_type_.c_str());
    RET_IF_ERROR(ret);

    ret = app_control_send_launch_request(handle, PickImageReplyCallback, this);
    RET_IF_ERROR(ret);
#undef RET_IF_ERROR

    app_control_destroy(handle);
  }

  static void PickImageReplyCallback(app_control_h request, app_control_h reply,
                                     app_control_result_e result,
                                     void *user_data) {
    LOG_INFO("PickImageReplyCallback called: %d", (int)result);

    auto *plugin = (ImagePickerTizenPlugin *)user_data;
    assert(plugin != nullptr);
    assert(plugin->result_ != nullptr);

    if (result != APP_CONTROL_RESULT_SUCCEEDED) {
      plugin->SendResultWithError("Operation failed");
      return;
    }

    char **values = nullptr;
    int count = 0;
    int ret = app_control_get_extra_data_array(reply, APP_CONTROL_DATA_SELECTED,
                                               &values, &count);
    if (ret != APP_CONTROL_ERROR_NONE) {
      plugin->SendResultWithError(std::to_string(ret), get_error_message(ret));
      return;
    }

    if (count == 0) {
      plugin->SendResultWithError("No file selected");
    } else if (plugin->multi_image_) {
      flutter::EncodableList paths;
      for (int i = 0; i < count; i++) {
        std::string source_path = values[i];
        std::string dest_path;
        if (plugin->image_resize_.Resize(source_path, dest_path)) {
          paths.push_back(flutter::EncodableValue(dest_path));
        } else {
          paths.push_back(flutter::EncodableValue(source_path));
        }
        free(values[i]);
      }
      plugin->SendResultWithSuccess(flutter::EncodableValue(paths));
    } else {
      std::string source_path = values[0];
      std::string dest_path;
      if (plugin->image_resize_.Resize(source_path, dest_path)) {
        plugin->SendResultWithSuccess(flutter::EncodableValue(dest_path));
      } else {
        plugin->SendResultWithSuccess(flutter::EncodableValue(source_path));
      }
      free(values[0]);
    }

    if (values) {
      free(values);
    }
  }

  void SendResultWithSuccess(const flutter::EncodableValue &result) {
    if (result_ == nullptr) {
      return;
    }
    result_->Success(result);
    result_ = nullptr;
  }

  void SendResultWithError(const std::string &error_code,
                           const std::string &error_message = "") {
    if (result_ == nullptr) {
      return;
    }
    result_->Error(error_code, error_message);
    result_ = nullptr;
  }

  void SetContentMimeType(const std::string &mime_type) {
    mime_type_ = mime_type + "/*";
  }

  ImageResize image_resize_;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result_;
  std::string mime_type_;
  bool multi_image_ = false;
};

void ImagePickerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ImagePickerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
