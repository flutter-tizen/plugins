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

#include <atomic>

#include "camera_method_channel.h"
#include "device_method_channel.h"
#include "orientation_manager.h"

using CameraCapturingCb = camera_capturing_cb;
using CameraCaptureCompletedCb = camera_capture_completed_cb;
using CameraFocusChangedCb = camera_focus_changed_cb;
using CameraPrivewCb = camera_preview_cb;
using MediaPacketPreviewCb = camera_media_packet_preview_cb;
using OnCaptureSuccessCb =
    std::function<void(const std::string &captured_file_path)>;
using OnCaptureFailureCb =
    std::function<void(const std::string &code, const std::string &message)>;

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

enum class ExifTagOrientation {
  kTopLeft = CAMERA_ATTR_TAG_ORIENTATION_TOP_LEFT,
  kTopRight = CAMERA_ATTR_TAG_ORIENTATION_TOP_RIGHT,
  kBottomRight = CAMERA_ATTR_TAG_ORIENTATION_BOTTOM_RIGHT,
  kBottomLeft = CAMERA_ATTR_TAG_ORIENTATION_BOTTOM_LEFT,
  kLeftTop = CAMERA_ATTR_TAG_ORIENTATION_LEFT_TOP,
  kRightTop = CAMERA_ATTR_TAG_ORIENTATION_RIGHT_TOP,
  kRightBottom = CAMERA_ATTR_TAG_ORIENTATION_RIGHT_BOTTOM,
  kLeftBottom = CAMERA_ATTR_TAG_ORIENTATION_LEFT_BOTTOM,
};

enum class CameraPixelFormat {
  kInvalid = CAMERA_PIXEL_FORMAT_INVALID,
  kJPEG = CAMERA_PIXEL_FORMAT_JPEG,
};

enum class CameraFlip {
  kNone = CAMERA_FLIP_NONE,
  kHorizontal = CAMERA_FLIP_HORIZONTAL,
  kVertical = CAMERA_FLIP_VERTICAL,
  kBoth = CAMERA_FLIP_BOTH,
};

enum class CameraAutoFocusMode {
  kNone = CAMERA_ATTR_AF_NONE,
  kNormal = CAMERA_ATTR_AF_NORMAL,
  kMacro = CAMERA_ATTR_AF_MACRO,
  kFull = CAMERA_ATTR_AF_FULL,
};

enum class FocusMode {
  kAuto,
  kLocked,
};

bool StringToFocusMode(std::string mode, FocusMode &focus_mode);
bool FocusModeToString(FocusMode focus_mode, std::string &mode);

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
  void SetFocusMode(FocusMode focus_mode);
  void TakePicture(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> &&result);

 private:
  void CreateCameraHandle();
  bool GetDeviceCount(int &count);
  bool GetAutoFocusMode(CameraAutoFocusMode &mode);
  bool GetLensOrientation(int &angle);
  bool GetState(CameraDeviceState &state);
  bool SetCameraFlip(CameraFlip flip);
  bool SetCaptureFormat(CameraPixelFormat format);
  bool SetExifTagEnable(bool enable);
  bool SetExifTagOrientatoin(ExifTagOrientation orientation);
  bool SetAutoFocusMode(CameraAutoFocusMode mode);
  bool SetAutoFocusChangedCb(CameraFocusChangedCb callback);
  bool SetMediaPacketPreviewCb(MediaPacketPreviewCb callback);
  bool SetPreviewCb(CameraPrivewCb callback);
  bool SetPreviewSize(Size size);
  bool StartCapture(OnCaptureSuccessCb on_success,
                    OnCaptureFailureCb on_failure);
  bool StartAutoFocusing(bool continuous);
  bool StartPreview();
  bool StopAutoFocusing();
  bool StopPreview();
  bool UnsetMediaPacketPreviewCb();
  bool UnsetAutoFocusChangedCb();
  void DestroyCameraHandle();

  bool PrintSupportedPreviewResolution();
  void PrintState();
  bool PrintPreviewRotation();

  flutter::PluginRegistrar *registrar_{nullptr};
  FlutterTextureRegistrar *texture_registrar_{nullptr};

  long texture_id_{0};
  CameraDeviceState state_{CameraDeviceState::kNone};
  CameraDeviceType type_{CameraDeviceType::kRear};
  std::unique_ptr<CameraMethodChannel> camera_method_channel_;
  std::unique_ptr<DeviceMethodChannel> device_method_channel_;
  std::unique_ptr<OrientationManager> orientation_manager_;
  camera_h handle_{nullptr};
  FocusMode focus_mode_{FocusMode::kAuto};
};

#endif
