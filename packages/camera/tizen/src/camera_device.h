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
#include "orientation_event_listener.h"

typedef camera_media_packet_preview_cb MediaPacketPreviewCb;

enum class CameraDeviceType {
  Rear = CAMERA_DEVICE_CAMERA0,  // The back(rear) camera is usually the primary
  Front = CAMERA_DEVICE_CAMERA1  // the front camera is usually the secondary
};

struct Size {
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
  bool SetMediaPacketPreviewCb(MediaPacketPreviewCb callback);
  bool StartPreview();
  bool StopPreview();
  bool UnsetMediaPacketPreviewCb();

 private:
  void CreateCameraHandle();
  void DestroyCameraHandle();

  int GetDeviceCount();
  int GetLensOrientation();
  void PrintState();
  void PrintPreviewRotation();

  flutter::PluginRegistrar *registrar_{nullptr};
  FlutterTextureRegistrar *texture_registrar_{nullptr};

  long texture_id_{0};
  CameraDeviceType type_{CameraDeviceType::Rear};
  std::unique_ptr<CameraMethodChannel> camera_method_channel_;
  std::unique_ptr<DeviceMethodChannel> device_method_channel_;
  std::unique_ptr<OrientationEventListner> orientation_event_listner_;
  camera_h handle_{nullptr};
};

#endif
