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

        auto p_result = result.release();
        PermissionManager::OnFailure on_failure =
            [p_result](const std::string &code, const std::string &message) {
              LOG_DEBUG("failure");
              p_result->Error(code, message);
              delete p_result;
            };

        auto plugin = this;
        // Request a camera permssion
        pmm_.RequestPermssion(
            Permission::kCamera,
            [plugin, p_result, camera_name, resolution_preset, enable_audio,
             on_failure]() {
              // Request a recorder permssion as asynchronous callchain
              plugin->pmm_.RequestPermssion(
                  Permission::kRecorder,
                  [plugin, p_result, camera_name, resolution_preset,
                   enable_audio]() {
                    LOG_DEBUG("All RequestPermssion success!");
                    flutter::EncodableValue reply =
                        plugin->InitializeCameraDevice(
                            camera_name, resolution_preset, enable_audio);
                    p_result->Success(reply);
                    delete p_result;
                  },
                  on_failure);
              LOG_DEBUG("Request a recorder permssion");
            },
            on_failure);
      }
    } else if (method_name == "initialize") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        std::string image_format_group;
        if (GetValueFromEncodableMap(arguments, "imageFormatGroup",
                                     image_format_group)) {
          camera_->Open(image_format_group);
          result->Success();
          return;
        }
      }
      result->Error("InvalidArguments", "Please check 'imageFormatGroup'");
    } else if (method_name == "takePicture") {
      camera_->TakePicture(std::move(result));
    } else if (method_name == "prepareForVideoRecording") {
      result->NotImplemented();
    } else if (method_name == "startVideoRecording") {
      camera_->StartVideoRecording(std::move(result));
    } else if (method_name == "stopVideoRecording") {
      camera_->StopVideoRecording(std::move(result));
    } else if (method_name == "pauseVideoRecording") {
      camera_->PauseVideoRecording(std::move(result));
    } else if (method_name == "resumeVideoRecording") {
      camera_->StartVideoRecording(std::move(result));
    } else if (method_name == "setFlashMode") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        std::string mode;
        FlashMode flash_mode;
        if (GetValueFromEncodableMap(arguments, "mode", mode)) {
          if (StringToFlashMode(mode, flash_mode)) {
            camera_->SetFlashMode(flash_mode);
            result->Success();
            return;
          }
        }
      }
      result->Error("InvalidArguments", "Please check 'mode'");

    } else if (method_name == "setExposureMode") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        std::string mode;
        ExposureMode exposure_mode;
        if (GetValueFromEncodableMap(arguments, "mode", mode)) {
          if (StringToExposureMode(mode, exposure_mode)) {
            camera_->SetExposureMode(exposure_mode);
            result->Success();
            return;
          }
        }
      }
      result->Error("InvalidArguments", "Please check 'mode'");
    } else if (method_name == "setExposurePoint") {
      LOG_WARN("setExposurePoint is not supported!");
      result->NotImplemented();
    } else if (method_name == "getMinExposureOffset") {
      LOG_WARN("getMinExposureOffset is not supported!");
      result->NotImplemented();
    } else if (method_name == "getMaxExposureOffset") {
      LOG_WARN("getMaxExposureOffset is not supported!");
      result->NotImplemented();
    } else if (method_name == "getExposureOffsetStepSize") {
      LOG_WARN("getExposureOffsetStepSize is not supported!");
      result->NotImplemented();
    } else if (method_name == "setExposureOffset") {
      LOG_WARN("setExposureOffset is not supported!");
      result->NotImplemented();
    } else if (method_name == "setFocusMode") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        std::string mode;
        FocusMode focus_mode;
        if (GetValueFromEncodableMap(arguments, "mode", mode)) {
          if (StringToFocusMode(mode, focus_mode)) {
            camera_->SetFocusMode(focus_mode);
            result->Success();
            return;
          }
        }
      }
      result->Error("InvalidArguments", "Please check 'mode'");
    } else if (method_name == "setFocusPoint") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        bool reset;
        if (GetValueFromEncodableMap(arguments, "reset", reset)) {
          if (reset) {
            camera_->RestFocusPoint();
          }
        }
        double x, y;
        if (GetValueFromEncodableMap(arguments, "x", x) &&
            GetValueFromEncodableMap(arguments, "y", y)) {
          camera_->SetFocusPoint(x, y);
          result->Success();
          return;
        }
      }
      result->Error("InvalidArguments", "Please check arguments(reset or x,y");
    } else if (method_name == "startImageStream") {
      result->NotImplemented();
    } else if (method_name == "stopImageStream") {
      result->NotImplemented();
    } else if (method_name == "getMaxZoomLevel") {
      float max = camera_->GetMaxZoomLevel();
      result->Success(flutter::EncodableValue(max));
    } else if (method_name == "getMinZoomLevel") {
      float min = camera_->GetMinZoomLevel();
      result->Success(flutter::EncodableValue(min));
    } else if (method_name == "setZoomLevel") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        double zoom;
        if (GetValueFromEncodableMap(arguments, "zoom", zoom)) {
          camera_->SetZoomLevel(zoom);
          result->Success();
          return;
        }
      }
      result->Error("InvalidArguments", "Please check 'zoom'");
    } else if (method_name == "lockCaptureOrientation") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());
        std::string orientation;
        OrientationType orientation_type;
        if (GetValueFromEncodableMap(arguments, "orientation", orientation) &&
            StringToOrientationType(orientation, orientation_type)) {
          camera_->LockCaptureOrientation(orientation_type);
          result->Success();
          return;
        }
        result->Error("InvalidArguments", "Please check 'orientation'");
      }
    } else if (method_name == "unlockCaptureOrientation") {
      camera_->UnlockCaptureOrientation();
      result->Success();
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
    if (camera_) {
      camera_ = nullptr;
    }

    CameraDeviceType type;
    if (camera_name == "camera1") {
      type = CameraDeviceType::kRear;
    } else {
      type = CameraDeviceType::kFront;
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
  PermissionManager pmm_;
};

void CameraPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  CameraPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar),
      FlutterPluginRegistrarGetTexture(registrar));
}
