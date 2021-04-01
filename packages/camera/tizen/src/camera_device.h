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

#include "camera_event_channel.h"

typedef camera_media_packet_preview_cb MediaPacketPreviewCb;

enum class CameraDeviceType {
  Rear = CAMERA_DEVICE_CAMERA0,  // The back(rear) camera is usually the primary
  Front = CAMERA_DEVICE_CAMERA1  // the front camera is usually the secondary
};

struct Size {
  int width;
  int height;
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
  long GetTextureId() { return texture_id_; }
  bool SetMediaPacketPreviewCb(MediaPacketPreviewCb callback);
  bool UnsetMediaPacketPreviewCb();
  bool StartPreview();
  bool StopPreview();
  FlutterTextureRegistrar *GetTextureRegistrar() { return texture_registrar_; }

 private:
  void CreateCameraHandle();
  void DestroyCameraHandle();

  int GetDeviceCount();
  int GetLensOrientation();
  void printState();
  void printPreviewRotation();

  flutter::PluginRegistrar *registrar_{nullptr};
  FlutterTextureRegistrar *texture_registrar_{nullptr};

  long texture_id_{0};
  CameraDeviceType type_{CameraDeviceType::Rear};
  std::unique_ptr<CameraEventChannel> camera_event_channel_;
  camera_h handle_{nullptr};
};

#endif
