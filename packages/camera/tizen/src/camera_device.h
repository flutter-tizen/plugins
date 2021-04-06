// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CAMERA_DEVICE_H_
#define FLUTTER_PLUGIN_CAMERA_DEVICE_H_

#include <camera.h>
#include <flutter/encodable_value.h>
#include <flutter/method_result.h>
#include <flutter/plugin_registrar.h>
#include <flutter_tizen_texture_registrar.h>

#include "camera_method_channel.h"
#include "device_method_channel.h"
#include "orientation_manager.h"

typedef camera_media_packet_preview_cb MediaPacketPreviewCb;

enum class CameraDeviceType {
  kRear =
      CAMERA_DEVICE_CAMERA0,  // The back(rear) camera is usually the primary
  kFront = CAMERA_DEVICE_CAMERA1  // the front camera is usually the secondary
};

enum class CameraDeviceState {
  kNone = CAMERA_STATE_NONE,
  kCreated = CAMERA_STATE_CREATED,
  kPreview = CAMERA_STATE_PREVIEW,
  kCapturing = CAMERA_STATE_CAPTURING,
  kCaputred = CAMERA_STATE_CAPTURED,
};

struct Size {
  // Dart implementation use double as a unit of preview size
  double width;
  double height;
};

class CameraDevice {
 public:
  static flutter::EncodableValue GetAvailableCameras();

  CameraDevice();
  CameraDevice(flutter::PluginRegistrar *registrar,
               FlutterTextureRegistrar *texture_registrar,
               CameraDeviceType type);
  ~CameraDevice();

  void ChangeCameraDeviceType(CameraDeviceType type);
  void Dispose();
  Size GetRecommendedPreviewResolution();
  FlutterTextureRegistrar *GetTextureRegistrar() { return texture_registrar_; }
  long GetTextureId() { return texture_id_; }
  bool Open(std::string image_format_group);

 private:
  void CreateCameraHandle();
  int GetDeviceCount();
  int GetLensOrientation();
  CameraDeviceState GetState();
  bool SetMediaPacketPreviewCb(MediaPacketPreviewCb callback);
  bool SetPreviewSize(Size size);
  bool StartPreview();
  bool StopPreview();
  bool UnsetMediaPacketPreviewCb();
  void DestroyCameraHandle();

  void PrintSupportedPreviewResolution();
  void PrintState();
  void PrintPreviewRotation();

  flutter::PluginRegistrar *registrar_{nullptr};
  FlutterTextureRegistrar *texture_registrar_{nullptr};

  long texture_id_{0};
  CameraDeviceState state_{CameraDeviceState::kNone};
  CameraDeviceType type_{CameraDeviceType::kRear};
  std::unique_ptr<CameraMethodChannel> camera_method_channel_;
  std::unique_ptr<DeviceMethodChannel> device_method_channel_;
  std::unique_ptr<OrientationManager> orientation_manager_;
  camera_h handle_{nullptr};
};

#endif
