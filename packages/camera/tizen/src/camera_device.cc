// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "camera_device.h"

#include <flutter/encodable_value.h>

#include "log.h"

flutter::EncodableValue CameraDevice::GetAvailableCameras() {
  CameraDevice default_camera;
  int count = default_camera.GetDeviceCount();

  flutter::EncodableList cameras;
  for (int i = 0; i < count; i++) {
    flutter::EncodableMap camera;
    camera[flutter::EncodableValue("name")] =
        flutter::EncodableValue("camera" + std::to_string(i + 1));

    int angle = default_camera.GetLensOrientation();
    camera[flutter::EncodableValue("sensorOrientation")] =
        flutter::EncodableValue(angle);
    std::string lensFacing;
    if (i == 0) {
      lensFacing = "back";
    } else {
      lensFacing = "front";
    }
    camera[flutter::EncodableValue("lensFacing")] =
        flutter::EncodableValue(lensFacing);

    cameras.push_back(flutter::EncodableValue(camera));
    default_camera.ChangeCameraDeviceType(CameraDeviceType::Front);
  }
  return flutter::EncodableValue(cameras);
}

CameraDevice::CameraDevice() { CreateCameraHandle(); }

CameraDevice::CameraDevice(flutter::PluginRegistrar* registrar,
                           FlutterTextureRegistrar* texture_registrar,
                           CameraDeviceType type)
    : registrar_(registrar),
      texture_registrar_(texture_registrar),
      type_(type) {
  CreateCameraHandle();
  texture_id_ = FlutterRegisterExternalTexture(texture_registrar_);
  LOG_DEBUG("texture_id_[%ld]", texture_id_);
  camera_event_channel_ =
      std::make_unique<CameraEventChannel>(registrar_, texture_id_);
}

CameraDevice::~CameraDevice() { Dispose(); }

void CameraDevice::CreateCameraHandle() {
  int error = camera_create((camera_device_e)type_, &handle_);
  if (error != CAMERA_ERROR_NONE) {
    LOG_ERROR("Failed to camera_create");
  }
}

void CameraDevice::DestroyCameraHandle() {
  if (handle_) {
    int error = camera_destroy(handle_);
    if (error != CAMERA_ERROR_NONE) {
      LOG_ERROR("Failed to camera_destroy");
    }
    handle_ = nullptr;
  }
}

void CameraDevice::ChangeCameraDeviceType(CameraDeviceType type) {
  int error = camera_change_device(handle_, (camera_device_e)type);
  if (error != CAMERA_ERROR_NONE) {
    LOG_ERROR("Failed to camera_change_device");
  }
  type_ = type;
}

void CameraDevice::Dispose() {
  DestroyCameraHandle();

  if (texture_registrar_) {
    FlutterUnregisterExternalTexture(texture_registrar_, texture_id_);
    texture_registrar_ = nullptr;
  }
}

int CameraDevice::GetDeviceCount() {
  int count = 0;
  // If the device supports primary and secondary camera, this returns 2. If 1
  // is returned, the device only supports primary camera.
  int error = camera_get_device_count(handle_, &count);
  if (error != CAMERA_ERROR_NONE) {
    LOG_ERROR("Failed to camera_get_device_count");
  }
  LOG_DEBUG("count[%d]", count);
  return count;
}

int CameraDevice::GetLensOrientation() {
  int angle = 0;
  int error = camera_attr_get_lens_orientation(handle_, &angle);
  if (error != CAMERA_ERROR_NONE) {
    LOG_ERROR("Failed to camera_attr_get_lens_orientation");
  }
  LOG_DEBUG("angle[%d]", angle);
  return angle;
}

Size CameraDevice::GetRecommendedPreviewResolution() {
  Size preview_size;
  int error = camera_get_recommended_preview_resolution(
      handle_, &(preview_size.width), &(preview_size.height));
  if (error != CAMERA_ERROR_NONE) {
    LOG_ERROR("Failed to camera_get_recommended_preview_resolution");
  }

  LOG_DEBUG("width[%d] height[%d]", preview_size.width, preview_size.height);
  return preview_size;
}
