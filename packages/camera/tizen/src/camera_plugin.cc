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
    } else if (method_name == "initialize") {
      if (method_call.arguments()) {
        flutter::EncodableMap arguments =
            std::get<flutter::EncodableMap>(*method_call.arguments());

        auto camera_name_value =
            arguments[flutter::EncodableValue("cameraName")];
        std::string camera_name;
        if (!camera_name_value.IsNull()) {
          camera_name = std::get<std::string>(camera_name_value);
        }

        auto resolution_preset_value =
            arguments[flutter::EncodableValue("resolutionPreset")];
        std::string resolution_preset;
        if (!resolution_preset_value.IsNull()) {
          resolution_preset = std::get<std::string>(resolution_preset_value);
        }

        auto enable_audio_value =
            arguments[flutter::EncodableValue("enableAudio")];
        bool enable_audio = false;
        if (!enable_audio_value.IsNull()) {
          enable_audio = std::get<bool>(enable_audio_value);
        }
        LOG_DEBUG("camera_name[%s], resolution_preset[%s], enableAudio[%s]",
                  camera_name.data(), resolution_preset.data(),
                  enable_audio ? "true" : "false");
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

    camera_->SetMediaPacketPreviewCb([](media_packet_h pkt, void *data) {
      tbm_surface_h surface = nullptr;
      int error = media_packet_get_tbm_surface(pkt, &surface);
      LOG_ERROR_IF(error != MEDIA_PACKET_ERROR_NONE,
                   "media_packet_get_tbm_surface fail - error : %s",
                   get_error_message(error));

      if (error == 0) {
        CameraDevice *camera_device = (CameraDevice *)data;
        FlutterMarkExternalTextureFrameAvailable(
            camera_device->GetTextureRegistrar(), camera_device->GetTextureId(),
            surface);
      }

      // destroy packet
      if (pkt) {
        error = media_packet_destroy(pkt);
        LOG_ERROR_IF(error != MEDIA_PACKET_ERROR_NONE,
                     "media_packet_destroy fail - error : %s",
                     get_error_message(error));
      }
    });
    camera_->StartPreview();

    flutter::EncodableMap ret;
    ret[flutter::EncodableValue("textureId")] =
        flutter::EncodableValue((int64_t)camera_->GetTextureId());

    Size size = camera_->GetRecommendedPreviewResolution();
    ret[flutter::EncodableValue("previewWidth")] =
        flutter::EncodableValue(size.width);
    ret[flutter::EncodableValue("previewHeight")] =
        flutter::EncodableValue(size.height);

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
