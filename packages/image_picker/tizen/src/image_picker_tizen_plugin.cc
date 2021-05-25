// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_picker_tizen_plugin.h"

#include <app_control.h>
#include <assert.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#ifndef TV_PROFILE
#include <privacy_privilege_manager.h>
#endif

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "image_resize.h"
#include "log.h"

enum class ImageSource {
  // Opens up the device camera, letting the user to take a new picture.
  CAMERA,
  // Opens the user's photo gallery.
  GALLERY,
};

#define RET_IF_ERROR(ret)                                             \
  if (ret != APP_CONTROL_ERROR_NONE) {                                \
    SendResultWithError(std::to_string(ret), get_error_message(ret)); \
    if (handle) app_control_destroy(handle);                          \
    return;                                                           \
  }

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

  ImagePickerTizenPlugin() : mime_type_("") {}

  virtual ~ImagePickerTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("method : %s", method_call.method_name().data());

    if (result_) {
      SendResultWithError("already_active", "Cancelled by a second request");
      return;
    }

    result_ = std::move(result);
    const auto &arguments = *method_call.arguments();

    ImageSource source = ImageSource::GALLERY;
    if (method_call.method_name().compare("pickImage") == 0) {
      double width = 0.0, height = 0.0;
      int quality = 0;
      if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
        flutter::EncodableMap values =
            std::get<flutter::EncodableMap>(arguments);
        auto s = values[flutter::EncodableValue("source")];
        if (std::holds_alternative<int>(s)) {
          source = (ImageSource)std::get<int>(s);
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
        if (std::holds_alternative<int>(q)) {
          quality = std::get<int>(q);
        }
        image_resize_.SetSize((unsigned int)width, (unsigned int)height,
                              quality);

      } else {
        SendResultWithError("Invalid arguments");
        return;
      }

      if (source == ImageSource::CAMERA) {
        // TODO: we need to check this feature after webcam is prepared
        SendResultWithError("Not supported on this device");
      } else if (source == ImageSource::GALLERY) {
        CheckPermissionAndPickImage("image");
      } else {
        SendResultWithError("Invalid image source");
      }

    } else if (method_call.method_name().compare("pickVideo") == 0) {
      if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
        flutter::EncodableMap values =
            std::get<flutter::EncodableMap>(arguments);
        auto s = values[flutter::EncodableValue("source")];
        if (std::holds_alternative<int>(s)) {
          source = (ImageSource)std::get<int>(s);
        }
      } else {
        SendResultWithError("Invalid arguments");
        return;
      }

      if (source == ImageSource::CAMERA) {
        // TODO: we need to check this feature after webcam is prepared
        SendResultWithError("Not supported on this device");
      } else if (source == ImageSource::GALLERY) {
        CheckPermissionAndPickImage("video");
      } else {
        SendResultWithError("Invalid video source");
      }
    } else {
      result_->NotImplemented();
      result_ = nullptr;
    }
  }

  void CheckPermissionAndPickImage(const std::string &mimeType) {
#ifndef TV_PROFILE
    const char *privilege = "http://tizen.org/privilege/mediastorage";

    ppm_check_result_e permission;
    int ret = ppm_check_permission(privilege, &permission);
    if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
      LOG_ERROR("ppm_check_permission fail! [%d]", ret);
    } else {
      switch (permission) {
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
          LOG_INFO("ppm_check_permission success! [%d]", (int)permission);
          SetContentMimeType(mimeType);
          PickContent();
          return;
        case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
          ret = ppm_request_permission(privilege, AppRequestResponseCb, this);
          if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE) {
            LOG_ERROR("ppm_request_permission fail! [%d]", ret);
            break;
          }
          return;
        default:
          LOG_ERROR("ppm_check_permission deny! [%d]", (int)permission);
          break;
      }
    }
    SendResultWithError("Invalid permission");
#else
    SetContentMimeType(mimeType);
    PickContent();
#endif
  }

  void PickContent() {
    if (mime_type_.size() == 0) return;

    app_control_h handle = NULL;
    int ret = app_control_create(&handle);
    RET_IF_ERROR(ret);

    ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_PICK);
    RET_IF_ERROR(ret);

    ret = app_control_set_mime(handle, mime_type_.c_str());
    mime_type_ = "";
    RET_IF_ERROR(ret);

    ret = app_control_send_launch_request(handle, PickImageReplyCb, this);
    RET_IF_ERROR(ret);

    app_control_destroy(handle);
  }

#ifndef TV_PROFILE
  static void AppRequestResponseCb(ppm_call_cause_e cause,
                                   ppm_request_result_e result,
                                   const char *privilege, void *data) {
    ImagePickerTizenPlugin *plugin = (ImagePickerTizenPlugin *)data;
    assert(plugin);

    if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR) {
      LOG_ERROR("app_request_response_cb failed! [%d]", result);
      plugin->SendResultWithError("Invalid permission");
      return;
    }

    plugin->PickContent();
  }
#endif

  static void PickImageReplyCb(app_control_h request, app_control_h reply,
                               app_control_result_e result, void *user_data) {
    LOG_INFO("PickImageReplyCb called: %d", (int)result);

    ImagePickerTizenPlugin *plugin = (ImagePickerTizenPlugin *)user_data;
    assert(plugin != nullptr);
    assert(plugin->result_ != nullptr);

    if (result != APP_CONTROL_RESULT_SUCCEEDED) {
      plugin->SendResultWithError("Not Found Images");
      return;
    }

    char **value = NULL;
    int count = 0;
    int ret = app_control_get_extra_data_array(reply, APP_CONTROL_DATA_SELECTED,
                                               &value, &count);
    if (ret != APP_CONTROL_ERROR_NONE) {
      plugin->SendResultWithError(std::to_string(ret), get_error_message(ret));
      return;
    }

    if (count == 1) {
      LOG_INFO("image path: %s", value[0]);
      std::string src_path = value[0];
      std::string dst_path;
      if (plugin->image_resize_.Resize(src_path, dst_path)) {
        plugin->SendResultWithSuccess(dst_path);
      } else {
        plugin->SendResultWithSuccess(src_path);
      }
      free(value[0]);
    } else {
      plugin->SendResultWithError("Not Found Images");
    }

    if (value) {
      free(value);
    }
  }

  void SendResultWithSuccess(std::string imagePath) {
    if (result_ == nullptr) {
      return;
    }
    result_->Success(flutter::EncodableValue(imagePath));
    result_ = nullptr;
  }

  void SendResultWithError(std::string errorCode,
                           std::string errorMessage = "") {
    if (result_ == nullptr) {
      return;
    }
    result_->Error(errorCode, errorMessage);
    result_ = nullptr;
  }

  void SetContentMimeType(const std::string &mimeType) {
    mime_type_ = mimeType + "/*";
  }

  ImageResize image_resize_;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result_;
  std::string mime_type_;
};

void ImagePickerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ImagePickerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
