// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_picker_tizen_plugin.h"

#include <app_control.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <cassert>
#include <memory>
#include <string>
#include <variant>

#include "image_resize.h"
#include "permission_manager.h"

namespace {

enum class ImageSource {
  // Opens up the device camera, letting the user to take a new picture.
  kCamera,
  // Opens the user's photo gallery.
  kGallery,
};

template <typename T>
static bool GetValueFromEncodableMap(const flutter::EncodableMap *map,
                                     const char *key, T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

class ImagePickerTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/image_picker_tizen",
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

    if (result_) {
      SendErrorResult("Already active", "Cancelled by a second request.");
      return;
    }
    result_ = std::move(result);

    if (method_name == "pickImage" || method_name == "pickMultiImage") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);

      int source_index = static_cast<int>(ImageSource::kGallery);
      double max_width = 0.0, max_height = 0.0;
      int32_t quality = 0;
      GetValueFromEncodableMap(arguments, "source", source_index);
      GetValueFromEncodableMap(arguments, "maxWidth", max_width);
      GetValueFromEncodableMap(arguments, "maxHeight", max_height);
      GetValueFromEncodableMap(arguments, "imageQuality", quality);

      image_resize_.SetSize(static_cast<uint32_t>(max_width),
                            static_cast<uint32_t>(max_height), quality);

      ImageSource source = ImageSource(source_index);
      if (source == ImageSource::kCamera) {
        // TODO: we need to check this feature after webcam is prepared
        SendErrorResult("Not supported", "Not supported on this device.");
      } else if (source == ImageSource::kGallery) {
        multi_image_ = method_name == "pickMultiImage";
        PickContent("image/*");
      } else {
        SendErrorResult("Invalid arguments", "Invalid image source.");
      }
    } else if (method_name == "pickVideo") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);

      int source_index = static_cast<int>(ImageSource::kGallery);
      GetValueFromEncodableMap(arguments, "source", source_index);

      ImageSource source = ImageSource(source_index);
      if (source == ImageSource::kCamera) {
        // TODO: we need to check this feature after webcam is prepared
        SendErrorResult("Not supported", "Not supported on this device.");
      } else if (source == ImageSource::kGallery) {
        multi_image_ = false;
        PickContent("video/*");
      } else {
        SendErrorResult("Invalid arguments", "Invalid video source.");
      }
    } else if (method_name == "pickMedia") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);

      int source_index = static_cast<int>(ImageSource::kGallery);
      double max_width = 0.0, max_height = 0.0;
      int32_t quality = 0;
      bool multiple = false;
      GetValueFromEncodableMap(arguments, "source", source_index);
      GetValueFromEncodableMap(arguments, "maxWidth", max_width);
      GetValueFromEncodableMap(arguments, "maxHeight", max_height);
      GetValueFromEncodableMap(arguments, "imageQuality", quality);
      GetValueFromEncodableMap(arguments, "allowMultiple", multiple);

      image_resize_.SetSize(static_cast<uint32_t>(max_width),
                            static_cast<uint32_t>(max_height), quality);

      ImageSource source = ImageSource(source_index);
      if (source == ImageSource::kCamera) {
        // TODO: we need to check this feature after webcam is prepared
        SendErrorResult("Not supported", "Not supported on this device.");
      } else if (source == ImageSource::kGallery) {
        multi_image_ = multiple;
        PickContent("image/*, video/*");
      } else {
        SendErrorResult("Invalid arguments", "Invalid image source.");
      }
    } else {
      result_->NotImplemented();
      result_ = nullptr;
    }
  }

  bool CheckPermission() {
    PermissionManager manager;
    PermissionResult result =
        manager.RequestPermission("http://tizen.org/privilege/mediastorage");

    if (result == PermissionResult::kDenyForever ||
        result == PermissionResult::kDenyOnce) {
      SendErrorResult("Permission denied", "Permission denied by user.");
      return false;
    } else if (result == PermissionResult::kError) {
      SendErrorResult("Operation failed", "Failed to request permission.");
      return false;
    }
    return true;
  }

  void PickContent(std::string mime_type) {
    if (!CheckPermission()) {
      return;
    }

    app_control_h handle = nullptr;
#define RETURN_IF_ERROR(ret)                                      \
  if (ret != APP_CONTROL_ERROR_NONE) {                            \
    SendErrorResult(std::to_string(ret), get_error_message(ret)); \
    if (handle) {                                                 \
      app_control_destroy(handle);                                \
    }                                                             \
    return;                                                       \
  }
    int ret = app_control_create(&handle);
    RETURN_IF_ERROR(ret);

    ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_PICK);
    RETURN_IF_ERROR(ret);

    ret = app_control_add_extra_data(handle, APP_CONTROL_DATA_SELECTION_MODE,
                                     multi_image_ ? "multiple" : "single");
    RETURN_IF_ERROR(ret);

    ret = app_control_set_mime(handle, mime_type.c_str());
    RETURN_IF_ERROR(ret);

    ret = app_control_send_launch_request(handle, ReplyCallback, this);
    RETURN_IF_ERROR(ret);
#undef RETURN_IF_ERROR

    app_control_destroy(handle);
  }

  static void ReplyCallback(app_control_h request, app_control_h reply,
                            app_control_result_e result, void *user_data) {
    auto *self = static_cast<ImagePickerTizenPlugin *>(user_data);
    assert(self->result_);

    if (result != APP_CONTROL_RESULT_SUCCEEDED) {
      self->SendErrorResult("Operation failed", "Received an error response.");
      return;
    }

    char **values = nullptr;
    int count = 0;
    int ret = app_control_get_extra_data_array(reply, APP_CONTROL_DATA_SELECTED,
                                               &values, &count);
    if (ret != APP_CONTROL_ERROR_NONE) {
      self->SendErrorResult(std::to_string(ret), get_error_message(ret));
      return;
    }

    if (count == 0) {
      self->SendErrorResult("Operation cancelled", "No file selected.");
    } else if (self->multi_image_) {
      flutter::EncodableList paths;
      for (int i = 0; i < count; i++) {
        std::string source_path = values[i];
        std::string dest_path;
        if (self->image_resize_.Resize(source_path, &dest_path)) {
          paths.push_back(flutter::EncodableValue(dest_path));
        } else {
          paths.push_back(flutter::EncodableValue(source_path));
        }
        free(values[i]);
      }
      self->SendResult(flutter::EncodableValue(paths));
    } else {
      std::string source_path = values[0];
      std::string dest_path;
      if (self->image_resize_.Resize(source_path, &dest_path)) {
        self->SendResult(flutter::EncodableValue(dest_path));
      } else {
        self->SendResult(flutter::EncodableValue(source_path));
      }
      free(values[0]);
    }

    if (values) {
      free(values);
    }
  }

  void SendResult(const flutter::EncodableValue &result) {
    if (!result_) {
      return;
    }
    result_->Success(result);
    result_ = nullptr;
  }

  void SendErrorResult(const std::string &error_code,
                       const std::string &error_message) {
    if (!result_) {
      return;
    }
    result_->Error(error_code, error_message);
    result_ = nullptr;
  }

  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result_;
  ImageResize image_resize_;
  bool multi_image_ = false;
};

}  // namespace

void ImagePickerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ImagePickerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
