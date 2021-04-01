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
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE, "camera_create fail - error : %s",
               get_error_message(error));
  printState();
  printPreviewRotation();
}

void CameraDevice::DestroyCameraHandle() {
  if (handle_) {
    int error = camera_destroy(handle_);
    LOG_ERROR_IF(error != CAMERA_ERROR_NONE, "camera_destroy fail - error : %s",
                 get_error_message(error));
    handle_ = nullptr;
  }
}

void CameraDevice::ChangeCameraDeviceType(CameraDeviceType type) {
  int error = camera_change_device(handle_, (camera_device_e)type);
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_change_device fail - error : %s",
               get_error_message(error));
  type_ = type;
}

void CameraDevice::Dispose() {
  LOG_DEBUG("enter");
  StopPreview();
  UnsetMediaPacketPreviewCb();
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
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_get_device_count fail - error : %s",
               get_error_message(error));

  LOG_DEBUG("count[%d]", count);
  return count;
}

int CameraDevice::GetLensOrientation() {
  int angle = 0;
  int error = camera_attr_get_lens_orientation(handle_, &angle);
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_attr_get_lens_orientation fail - error : %s",
               get_error_message(error));

  LOG_DEBUG("angle[%d]", angle);
  return angle;
}

void CameraDevice::printState() {
  camera_state_e state;
  camera_get_state(handle_, &state);
  switch (state) {
    case CAMERA_STATE_NONE:
      LOG_DEBUG("CAMERA_STATE_NONE");
      break;
    case CAMERA_STATE_CREATED:
      LOG_DEBUG("CAMERA_STATE_CREATED");
      break;
    case CAMERA_STATE_PREVIEW:
      LOG_DEBUG("CAMERA_STATE_PREVIEW");
      break;
    case CAMERA_STATE_CAPTURING:
      LOG_DEBUG("CAMERA_STATE_CAPTURING");
      break;
    case CAMERA_STATE_CAPTURED:
      LOG_DEBUG("CAMERA_STATE_CAPTURED");
      break;
    default:
      LOG_DEBUG("Unknown State");
      break;
  }
}

void CameraDevice::printPreviewRotation() {
  camera_rotation_e val;
  int error = camera_attr_get_stream_rotation(handle_, &val);
  switch (val) {
    case CAMERA_ROTATION_NONE:
      LOG_DEBUG("CAMERA_ROTATION_NONE");
      break;
    case CAMERA_ROTATION_90:
      LOG_DEBUG("CAMERA_ROTATION_90");
      break;
    case CAMERA_ROTATION_180:
      LOG_DEBUG("CAMERA_ROTATION_180");
      break;
    case CAMERA_ROTATION_270:
      LOG_DEBUG("CAMERA_ROTATION_270");
      break;
    default:
      break;
  }
}

Size CameraDevice::GetRecommendedPreviewResolution() {
  Size preview_size;
  int error = camera_get_recommended_preview_resolution(
      handle_, &(preview_size.width), &(preview_size.height));
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_get_recommended_preview_resolution fail - error : %s",
               get_error_message(error));

  LOG_DEBUG("width[%d] height[%d]", preview_size.width, preview_size.height);
  return preview_size;
}

bool CameraDevice::SetMediaPacketPreviewCb(MediaPacketPreviewCb callback) {
  int error = camera_set_media_packet_preview_cb(handle_, callback, this);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_media_packet_preview_cb fail - error : %s",
                    get_error_message(error));

  return true;
}
bool CameraDevice::UnsetMediaPacketPreviewCb() {
  int error = camera_unset_media_packet_preview_cb(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_unset_media_packet_preview_cb fail - error : %s",
                    get_error_message(error));

  return true;
}

bool CameraDevice::StartPreview() {
  int error = camera_start_preview(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_start_preview fail - error : %s",
                    get_error_message(error));
  return true;
}

bool CameraDevice::StopPreview() {
  int error = camera_stop_preview(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_stop_preview fail - error : %s",
                    get_error_message(error));
  return true;
}
