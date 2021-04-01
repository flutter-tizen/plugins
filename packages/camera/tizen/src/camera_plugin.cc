// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "camera_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_tizen_texture_registrar.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "camera_device.h"
#include "log.h"
#include "permission_manager.h"

#define CAMERA_CHANNEL_NAME "plugins.flutter.io/camera"
#define IMAGE_STREAM_CHANNEL_NAME "plugins.flutter.io/camera/imageStream"

template <typename T>
bool GetValueFromEncodableMap(flutter::EncodableMap &map, std::string key,
                              T &out) {
  flutter::EncodableValue value = map[flutter::EncodableValue(key)];
  if (!value.IsNull()) {
    out = std::get<T>(value);
    return true;
  }
  return false;
}

class CameraPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(
      flutter::PluginRegistrar *registrar,
      FlutterTextureRegistrar *texture_registrar) {
    auto camera_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), CAMERA_CHANNEL_NAME,
            &flutter::StandardMethodCodec::GetInstance());

    auto camera_plugin =
        std::make_unique<CameraPlugin>(registrar, texture_registrar);

    camera_channel->SetMethodCallHandler(
        [plugin_pointer = camera_plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(camera_plugin));
  }

  CameraPlugin(flutter::PluginRegistrar *registrar,
               FlutterTextureRegistrar *texture_registrar)
      : registrar_(registrar), texture_registrar_(texture_registrar) {}

  virtual ~CameraPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    std::string method_name = method_call.method_name();

    LOG_DEBUG("method_name[%s]", method_name.data());
    if (method_name == "availableCameras") {
      flutter::EncodableValue availableCameras =
          CameraDevice::GetAvailableCameras();
      result->Success(availableCameras);
    } else if (method_name == "create") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        std::string camera_name;
        std::string resolution_preset;
        bool enable_audio = false;
        if (!GetValueFromEncodableMap(arguments, "cameraName", camera_name) ||
            !GetValueFromEncodableMap(arguments, "resolutionPreset",
                                      resolution_preset) ||
            !GetValueFromEncodableMap(arguments, "enableAudio", enable_audio)) {
          result->Error(
              "InvalidArguments",
              "Please check cameraName, resolutionPreset, enableAudio.");
          return;
        }

        PermissionManager pmm;
        auto p_result = result.release();
        // Request a camera permssion
        pmm.RequestPermssion(
            Permission::kCamera,
            [this, p_result, &camera_name, &resolution_preset,
             &enable_audio]() {
              flutter::EncodableValue reply = InitializeCameraDevice(
                  camera_name, resolution_preset, enable_audio);
              p_result->Success(reply);
              delete p_result;
            },
            [p_result](const std::string &code, const std::string &message) {
              p_result->Error(code, message);
              LOG_DEBUG("failure");
              delete p_result;
            });
      }
    } else if (method_name == "initialize") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        std::string image_format_group;
        GetValueFromEncodableMap(arguments, "imageFormatGroup",
                                 image_format_group);
        camera_->Open(image_format_group);
        result->Success();
      }
    } else if (method_name == "takePicture") {
      result->NotImplemented();
    } else if (method_name == "prepareForVideoRecording") {
      result->NotImplemented();
    } else if (method_name == "startVideoRecording") {
      result->NotImplemented();
    } else if (method_name == "stopVideoRecording") {
      result->NotImplemented();
    } else if (method_name == "pauseVideoRecording") {
      result->NotImplemented();
    } else if (method_name == "resumeVideoRecording") {
      result->NotImplemented();
    } else if (method_name == "startImageStream") {
      result->NotImplemented();
    } else if (method_name == "stopImageStream") {
      result->NotImplemented();
    } else if (method_name == "dispose") {
      if (camera_) {
        camera_->Dispose();
        result->Success();
      }
    } else {
      result->NotImplemented();
    }
  }

  flutter::EncodableValue InitializeCameraDevice(
      const std::string &camera_name,
      const std::string & /*TODO :resolution_preset*/,
      bool /*TODO : enable_audio_value*/) {
    CameraDeviceType type;
    if (camera_name == "camera1") {
      type = CameraDeviceType::Rear;
    } else {
      type = CameraDeviceType::Front;
    }
    camera_ =
        std::make_unique<CameraDevice>(registrar_, texture_registrar_, type);
    flutter::EncodableMap ret;
    ret[flutter::EncodableValue("cameraId")] =
        flutter::EncodableValue((int64_t)camera_->GetTextureId());
    return flutter::EncodableValue(ret);
  }

  flutter::PluginRegistrar *registrar_{nullptr};
  FlutterTextureRegistrar *texture_registrar_{nullptr};
  std::unique_ptr<CameraDevice> camera_;
};

void CameraPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  CameraPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar),
      FlutterPluginRegistrarGetTexture(registrar));
}
