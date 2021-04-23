// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "camera_device.h"

#include <app_common.h>
#include <flutter/encodable_value.h>
#include <sys/time.h>

#include <cmath>

#include "log.h"

static uint64_t Timestamp() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;
}

static std::string CreateTempFileName(const std::string &prefix,
                                      const std::string &extension) {
  std::string file_name;
  char *cache_dir_path = app_get_cache_path();
  if (!cache_dir_path) {
    return file_name;
  }
  file_name.append(cache_dir_path);
  file_name.append(prefix);
  file_name.append(std::to_string(Timestamp()));
  file_name.append(".");
  file_name.append(extension);
  free(cache_dir_path);
  return file_name;
}

static ExifTagOrientation ChooseExifTagOrientatoin(
    OrientationType device_orientation, bool is_front_lens_facing) {
  ExifTagOrientation orientation = ExifTagOrientation::kTopLeft;

  switch (device_orientation) {
    case OrientationType::kPortraitUp:
      if (is_front_lens_facing) {
        orientation = ExifTagOrientation::kLeftBottom;
      } else {
        orientation = ExifTagOrientation::kRightTop;
      }
      break;
    case OrientationType::kLandscapeLeft:
      if (is_front_lens_facing) {
        orientation = ExifTagOrientation::kTopLeft;
      } else {
        orientation = ExifTagOrientation::kBottomLeft;
      }
      break;
    case OrientationType::kPortraitDown:
      if (is_front_lens_facing) {
        orientation = ExifTagOrientation::kRightTop;
      } else {
        orientation = ExifTagOrientation::kLeftBottom;
      }
      break;
    case OrientationType::kLandscapeRight:
      if (is_front_lens_facing) {
        orientation = ExifTagOrientation::kBottomRight;
      } else {
        orientation = ExifTagOrientation::kTopRight;
      }
      break;
    default:
      LOG_ERROR("Unknown OrientationType!");
      break;
  }

  return orientation;
}

bool StringToCameraPixelFormat(std::string Image_format,
                               CameraPixelFormat &pixel_format) {
  if (Image_format == "bgra8888") {
    LOG_WARN("bgra8888 is unsupported");
    return false;
  } else if (Image_format == "yuv420") {
    pixel_format = CameraPixelFormat::kYUV420;
    return true;
  } else if (Image_format == "jpeg") {
    pixel_format = CameraPixelFormat::kJPEG;
    return true;
  }
  LOG_WARN("Unknown Image format!");
  return false;
}

bool StringToExposureMode(std::string mode, ExposureMode &exposure_mode) {
  LOG_DEBUG("mode[%s]", mode.c_str());
  if (mode == "auto") {
    exposure_mode = ExposureMode::kAuto;
    return true;
  } else if (mode == "locked") {
    exposure_mode = ExposureMode::kLocked;
    return true;
  }
  LOG_WARN("Unknown exposure mode!");
  return false;
}

bool StringToFlashMode(std::string mode, FlashMode &flash_mode) {
  LOG_DEBUG("mode[%s]", mode.c_str());
  if (mode == "off") {
    flash_mode = FlashMode::kOff;
    return true;
  } else if (mode == "auto") {
    flash_mode = FlashMode::kAuto;
    return true;
  } else if (mode == "always") {
    flash_mode = FlashMode::kAlways;
    return true;
  } else if (mode == "torch") {
    flash_mode = FlashMode::kTorch;
    return true;
  }
  LOG_WARN("Unknown flash mode!");
  return false;
}

bool StringToFocusMode(std::string mode, FocusMode &focus_mode) {
  LOG_DEBUG("mode[%s]", mode.c_str());
  if (mode == "auto") {
    focus_mode = FocusMode::kAuto;
    return true;
  } else if (mode == "locked") {
    focus_mode = FocusMode::kLocked;
    return true;
  }
  LOG_WARN("Unknown focus mode!");
  return false;
}

bool ExposureModeToString(ExposureMode exposure_mode, std::string &mode) {
  switch (exposure_mode) {
    case ExposureMode::kAuto:
      mode = "auto";
      return true;
    case ExposureMode::kLocked:
      mode = "locked";
      return true;
    default:
      LOG_WARN("Unknown exposure mode!");
      return false;
      break;
  }
}

bool FocusModeToString(FocusMode focus_mode, std::string &mode) {
  switch (focus_mode) {
    case FocusMode::kAuto:
      mode = "auto";
      return true;
    case FocusMode::kLocked:
      mode = "locked";
      return true;
    default:
      LOG_WARN("Unknown focus mode!");
      return false;
  }
}

flutter::EncodableValue CameraDevice::GetAvailableCameras() {
  CameraDevice default_camera;
  int count = 0;
  default_camera.GetDeviceCount(count);

  flutter::EncodableList cameras;
  for (int i = 0; i < count; i++) {
    flutter::EncodableMap camera;
    camera[flutter::EncodableValue("name")] =
        flutter::EncodableValue("camera" + std::to_string(i + 1));

    int angle = 0;
    default_camera.GetLensOrientation(angle);
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
    default_camera.ChangeCameraDeviceType(CameraDeviceType::kFront);
  }
  return flutter::EncodableValue(cameras);
}

CameraDevice::CameraDevice() {
  CameraCreate();
  GetState(state_);
}

CameraDevice::CameraDevice(flutter::PluginRegistrar *registrar,
                           FlutterTextureRegistrar *texture_registrar,
                           CameraDeviceType type)
    : registrar_(registrar),
      texture_registrar_(texture_registrar),
      type_(type) {
  CameraCreate();
  SetExifTagEnable(true);
  SetCameraAutoFocusMode(CameraAutoFocusMode::kNormal);
  if (type == CameraDeviceType::kFront) {
    SetCameraFlip(CameraFlip::kVertical);
  }

  GetState(state_);
  GetPreviewResolution(preview_width_, preview_height_);

  texture_id_ = FlutterRegisterExternalTexture(texture_registrar_);
  LOG_DEBUG("texture_id_[%ld]", texture_id_);
  camera_method_channel_ =
      std::make_unique<CameraMethodChannel>(registrar_, texture_id_);
  device_method_channel_ = std::make_unique<DeviceMethodChannel>(registrar_);

  int angle = 0;
  GetLensOrientation(angle);
  orientation_manager_ = std::make_unique<OrientationManager>(
      device_method_channel_.get(), (OrientationType)angle,
      type == CameraDeviceType::kFront);

  orientation_manager_->Start();

  // Print camera info for convenience of development, These will be removed
  // after release
  PrintState();
  PrintPreviewRotation();
  PrintSupportedPreviewResolution();
}

CameraDevice::~CameraDevice() { Dispose(); }

bool CameraDevice::CameraCreate() {
  int error = camera_create((camera_device_e)type_, &handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_create fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::CameraDestroy() {
  LOG_DEBUG("enter");
  int error = camera_destroy(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_destroy fail - error[%d]: %s", error,
                    get_error_message(error));
  handle_ = nullptr;
  LOG_DEBUG("done");
  return true;
}

bool CameraDevice::ClearAutoFocusArea() {
  int error = camera_attr_clear_af_area(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_clear_af_area fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

void CameraDevice::ChangeCameraDeviceType(CameraDeviceType type) {
  int error = camera_change_device(handle_, (camera_device_e)type);
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_change_device fail - error[%d]: %s", error,
               get_error_message(error));
  type_ = type;
}

void CameraDevice::Dispose() {
  LOG_DEBUG("enter");
  if (state_ == CameraDeviceState::kPreview) {
    StopPreview();
    UnsetMediaPacketPreviewCb();
    UnsetAutoFocusChangedCb();
  }

  CameraDestroy();

  if (orientation_manager_) {
    orientation_manager_->Stop();
  }

  if (texture_registrar_) {
    FlutterUnregisterExternalTexture(texture_registrar_, texture_id_);
    texture_registrar_ = nullptr;
  }
}

bool CameraDevice::GetDeviceCount(int &count) {
  // If the device supports primary and secondary camera, this returns 2. If 1
  // is returned, the device only supports primary camera.
  int error = camera_get_device_count(handle_, &count);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_get_device_count fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetAutoFocusMode(CameraAutoFocusMode &mode) {
  int error = camera_attr_get_af_mode(handle_, (camera_attr_af_mode_e *)&mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_get_af_mode fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetLensOrientation(int &angle) {
  int error = camera_attr_get_lens_orientation(handle_, &angle);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_get_lens_orientation fail - error[%d]: %s",
                    error, get_error_message(error));

  return true;
}

bool CameraDevice::GetPreviewResolution(int &width, int &height) {
  int w, h;
  int error = camera_get_preview_resolution(handle_, &w, &h);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_get_preview_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  width = w;
  height = h;
  return true;
}

bool CameraDevice::GetState(CameraDeviceState &state) {
  int error = camera_get_state(handle_, (camera_state_e *)&state);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_get_state fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetZoomRange(int &min, int &max) {
  int error = camera_attr_get_zoom_range(handle_, &min, &max);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_get_zoom_range fail - error[%d]: %s", error,
                    get_error_message(error));
  if (min > max) {
    // According to the API doc, this means that it is not supported on device
    LOG_WARN("Not supported");
    return false;
  }
  LOG_DEBUG("zoom range : min[%d] max[%d]", min, max);
  return true;
}

bool CameraDevice::SetExifTagEnable(bool enable) {
  int error = camera_attr_enable_tag(handle_, enable);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_enable_tag fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::SetExifTagOrientatoin(ExifTagOrientation orientation) {
  int error = camera_attr_set_tag_orientation(
      handle_, (camera_attr_tag_orientation_e)orientation);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_tag_orientation fail - error[%d]: %s",
                    error, get_error_message(error));
  return true;
}

bool CameraDevice::SetAutoFocusArea(int x, int y) {
  LOG_DEBUG("camera_attr_set_af_area x[%d], y[%d]", x, y);
  int error = camera_attr_set_af_area(handle_, x, y);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_af_area fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetAutoFocusChangedCb(CameraFocusChangedCb callback) {
  int error = camera_set_focus_changed_cb(handle_, callback, this);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_focus_changed_cb fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraAutoFocusMode(CameraAutoFocusMode mode) {
  int error = camera_attr_set_af_mode(handle_, (camera_attr_af_mode_e)mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_af_mode fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraExposureMode(CameraExposureMode mode) {
  int error =
      camera_attr_set_exposure_mode(handle_, (camera_attr_exposure_mode_e)mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_exposure_mode fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraFlashMode(CameraFlashMode mode) {
  int error =
      camera_attr_set_flash_mode(handle_, (camera_attr_flash_mode_e)mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_flash_mode fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::SetCameraFlip(CameraFlip flip) {
  int error = camera_attr_set_stream_flip(handle_, (camera_flip_e)flip);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_stream_flip fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::SetCaptureFormat(CameraPixelFormat format) {
  int error = camera_set_capture_format(handle_, (camera_pixel_format_e)format);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_capture_format ( fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::PrintSupportedPreviewResolution() {
  LOG_DEBUG("enter");
  int error = camera_foreach_supported_preview_resolution(
      handle_,
      [](int width, int height, void *user_data) -> bool {
        LOG_DEBUG("supported preview w[%d] h[%d]", width, height);
        return true;
      },
      nullptr);
  RETV_LOG_ERROR_IF(
      error != CAMERA_ERROR_NONE, false,
      "camera_foreach_supported_preview_resolution fail - error[%d]: %s", error,
      get_error_message(error));

  return true;
}

void CameraDevice::PrintState() {
  switch (state_) {
    case CameraDeviceState::kNone:
      LOG_DEBUG("CameraDeviceState[None]");
      break;
    case CameraDeviceState::kCreated:
      LOG_DEBUG("CameraDeviceState[Created]");
      break;
    case CameraDeviceState::kPreview:
      LOG_DEBUG("CameraDeviceState[Preview]");
      break;
    case CameraDeviceState::kCapturing:
      LOG_DEBUG("CameraDeviceState[Capturing]");
      break;
    case CameraDeviceState::kCaputred:
      LOG_DEBUG("CameraDeviceState[Caputred]");
      break;
    default:
      LOG_DEBUG("CameraDeviceState[Unknown]");
      break;
  }
}

bool CameraDevice::PrintPreviewRotation() {
  camera_rotation_e val = CAMERA_ROTATION_NONE;
  int error = camera_attr_get_stream_rotation(handle_, &val);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_get_stream_rotation fail - error[%d]: %s",
                    error, get_error_message(error));
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
  return true;
}

Size CameraDevice::GetRecommendedPreviewResolution() {
  Size preview_size;
  int w, h;
  int error = camera_get_recommended_preview_resolution(handle_, &w, &h);
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_get_recommended_preview_resolution fail - error[%d]: %s",
               error, get_error_message(error));

  auto target_orientation = orientation_manager_->ConvertTargetOrientation(
      OrientationType::kPortraitUp);
  if (target_orientation == OrientationType::kLandscapeLeft ||
      target_orientation == OrientationType::kLandscapeRight) {
    preview_size.width = h;
    preview_size.height = w;
  } else {
    preview_size.width = w;
    preview_size.height = h;
  }

  LOG_DEBUG("width[%f] height[%f]", preview_size.width, preview_size.height);
  return preview_size;
}

double CameraDevice::GetMaxZoomLevel() {
  int min = 0, max = 0;
  GetZoomRange(min, max);
  return static_cast<double>(max);
}

double CameraDevice::GetMinZoomLevel() {
  int min = 0, max = 0;
  GetZoomRange(min, max);
  return static_cast<double>(min);
}

bool CameraDevice::Open(std::string image_format_group) {
  LOG_DEBUG("enter");
  LOG_DEBUG("Recieved image_format_group[%s]", image_format_group.c_str());
  CameraPixelFormat pixel_format;
  if (StringToCameraPixelFormat(image_format_group, pixel_format)) {
    LOG_DEBUG("Try to set pixel_format[%d]", static_cast<int>(pixel_format));
    SetCaptureFormat(pixel_format);
    SetPreviewFormat(pixel_format);
  }

  SetMediaPacketPreviewCb([](media_packet_h pkt, void *data) {
    tbm_surface_h surface = nullptr;
    int error = media_packet_get_tbm_surface(pkt, &surface);
    LOG_ERROR_IF(error != MEDIA_PACKET_ERROR_NONE,
                 "media_packet_get_tbm_surface fail - error[%d]: %s", error,
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
                   "media_packet_destroy fail - error[%d]: %s", error,
                   get_error_message(error));
    }
  });

  StartPreview();

  SetAutoFocusChangedCb([](camera_focus_state_e state, void *user_data) {
    LOG_DEBUG("Change auto focus state[%d]", static_cast<int>(state));
  });

  // Start or stop focusing according to FocusMode
  SetFocusMode(focus_mode_);
  // Start or stop exposure accroding to ExpoureMode
  SetExposureMode(exposure_mode_);

  flutter::EncodableMap map;
  Size size = GetRecommendedPreviewResolution();
  map[flutter::EncodableValue("previewWidth")] =
      flutter::EncodableValue(size.width);
  map[flutter::EncodableValue("previewHeight")] =
      flutter::EncodableValue(size.height);

  std::string focus_mode;
  if (!FocusModeToString(focus_mode_, focus_mode)) {
    // fall back
    LOG_WARN("Send fallback focus mode(auto)");
    focus_mode = "auto";
  }
  map[flutter::EncodableValue("focusMode")] =
      flutter::EncodableValue(focus_mode);

  std::string exposure_mode;
  if (!ExposureModeToString(exposure_mode_, exposure_mode)) {
    // fall back
    LOG_WARN("Send fallback exposure mode(auto)");
    exposure_mode = "auto";
  }
  map[flutter::EncodableValue("exposureMode")] =
      flutter::EncodableValue(exposure_mode);

  map[flutter::EncodableValue("focusPointSupported")] =
      flutter::EncodableValue(true);

  // exposurePoint is unsupported on Tizen
  map[flutter::EncodableValue("exposurePointSupported")] =
      flutter::EncodableValue(false);

  auto value = std::make_unique<flutter::EncodableValue>(map);
  camera_method_channel_->Send(CameraEventType::kInitialized, std::move(value));
  return true;
}

void CameraDevice::RestFocusPoint() {
  LOG_DEBUG("enter");
  ClearAutoFocusArea();
}

void CameraDevice::SetExposureMode(ExposureMode exposure_mode) {
  switch (exposure_mode) {
    case ExposureMode::kAuto:
      // Use CAMERA_ATTR_EXPOSURE_MODE_CENTER as default
      LOG_DEBUG("Start exposure");
      SetCameraExposureMode(CameraExposureMode::kCenter);
      break;
    case ExposureMode::kLocked:
      LOG_DEBUG("Stop exposure");
      SetCameraExposureMode(CameraExposureMode::kOff);
      break;
    default:
      LOG_WARN("Unknown ExposureMode!");
      return;
  }
  exposure_mode_ = exposure_mode;
}

void CameraDevice::SetFlashMode(FlashMode flash_mode) {
  switch (flash_mode) {
    case FlashMode::kOff:
      SetCameraFlashMode(CameraFlashMode::kOff);
      break;
    case FlashMode::kAuto:
      SetCameraFlashMode(CameraFlashMode::kAuto);
      break;
    case FlashMode::kAlways:
      SetCameraFlashMode(CameraFlashMode::kOn);
      break;
    case FlashMode::kTorch:
      SetCameraFlashMode(CameraFlashMode::kPermanent);
      break;
    default:
      LOG_WARN("Unknown flash mode!");
      break;
  }
}

void CameraDevice::SetFocusMode(FocusMode focus_mode) {
  switch (focus_mode) {
    case FocusMode::kAuto: {
      LOG_DEBUG("Start auto focusing");
      if (!StartAutoFocusing(true)) {
        // Fallback, try to Auto focusing with non-continuous
        LOG_DEBUG("try to start auto focusing with non-continuous");
        StartAutoFocusing(false);
      }
    } break;
    case FocusMode::kLocked:
      LOG_DEBUG("Stop auto focusing");
      StopAutoFocusing();
      break;
    default:
      LOG_WARN("Unknown FocusMode!");
      return;
  }
  focus_mode_ = focus_mode;
}

void CameraDevice::SetFocusPoint(double x, double y) {
  LOG_DEBUG("x[%f], y[%f]", x, y);
  auto target_orientation = orientation_manager_->GetTargetOrientationType();
  int computed_x, computed_y;
  switch (target_orientation) {
    case OrientationType::kPortraitUp:
      computed_x = preview_width_ - (preview_width_ * x);
      computed_y = preview_height_ - (preview_height_ * y);
      break;
    case OrientationType::kLandscapeLeft:
      computed_x = preview_width_ - (preview_width_ * y);
      computed_y = preview_height_ * x;
      break;
    case OrientationType::kPortraitDown:
      computed_x = preview_width_ * x;
      computed_y = preview_height_ * y;
      break;
    case OrientationType::kLandscapeRight:
      computed_x = preview_width_ * y;
      computed_y = preview_height_ - (preview_height_ * x);
      break;
    default:
      LOG_ERROR("Unknown OrientationType!");
      break;
  }
  SetAutoFocusArea(computed_x, computed_y);
  return;
}

void CameraDevice::SetZoomLevel(double zoom) {
  LOG_DEBUG("zoom[%f]", zoom);
  SetZoom(static_cast<int>(round(zoom)));
};

void CameraDevice::TakePicture(
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> &&result) {
  SetExifTagOrientatoin(
      ChooseExifTagOrientatoin(orientation_manager_->GetDeviceOrientationType(),
                               type_ == CameraDeviceType::kFront));
  auto p_result = result.release();
  StartCapture(
      [p_result, this](const std::string &captured_file_path) {
        LOG_DEBUG("OnSuccess!!!");
        flutter::EncodableValue value(captured_file_path);
        p_result->Success(value);
        StartPreview();
        delete p_result;
      },
      [p_result](const std::string &code, const std::string &message) {
        LOG_DEBUG("OnFailure!!!");
        p_result->Error(code, message);
        delete p_result;
      });
}

bool CameraDevice::SetMediaPacketPreviewCb(MediaPacketPreviewCb callback) {
  int error = camera_set_media_packet_preview_cb(handle_, callback, this);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_media_packet_preview_cb fail - error[%d]: %s",
                    error, get_error_message(error));

  return true;
}

bool CameraDevice::SetPreviewCb(CameraPrivewCb callback) {
  int error = camera_set_preview_cb(handle_, callback, this);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_preview_cb fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetPreviewFormat(CameraPixelFormat format) {
  int error = camera_set_preview_format(handle_, (camera_pixel_format_e)format);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_preview_format fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetPreviewSize(Size size) {
  int w, h;
  w = static_cast<int>(round(size.width));
  h = static_cast<int>(round(size.height));

  LOG_DEBUG("camera_set_preview_resolution w[%d] h[%d]", w, h);

  int error = camera_set_preview_resolution(handle_, w, h);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_preview_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetZoom(int zoom) {
  LOG_DEBUG("zoom[%d]", zoom);
  int error = camera_attr_set_zoom(handle_, zoom);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_zoom fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::StartCapture(OnCaptureSuccessCb on_success,
                                OnCaptureFailureCb on_failure) {
  struct Param {
    OnCaptureSuccessCb on_success;
    OnCaptureFailureCb on_failure;
    std::string captured_file_path;
    std::string error;
    std::string error_message;
  };

  Param *p = new Param;  // Must delete on capture_completed_callback
  p->on_success = std::move(on_success);
  p->on_failure = std::move(on_failure);

  int error = camera_start_capture(
      handle_,
      [](camera_image_data_s *image, camera_image_data_s *postview,
         camera_image_data_s *thumbnail, void *user_data) {
        Param *p = (Param *)user_data;
        if (!image || !image->data) {
          p->error = "Capturing error";
          p->error_message = "camera_start_capture fail";
          return;
        }

        p->captured_file_path = CreateTempFileName("CAP", "jpg");
        if (!p->captured_file_path.size()) {
          p->error = "Insufficient memory";
          p->error_message = "app_get_cache_path fail";
          return;
        }

        FILE *file = fopen(p->captured_file_path.c_str(), "w+");
        if (!file) {
          p->error = "Insufficient memory";
          p->error_message = "fopen fail";
          return;
        }

        if (fwrite(image->data, 1, image->size, file) != image->size) {
          p->error = "Insufficient memory";
          p->error_message = "fwrite fail";
        }
        fclose(file);
      },
      [](void *user_data) {
        Param *p = (Param *)user_data;
        if (p->error.size()) {
          p->on_failure(p->error, p->error_message);
        } else {
          p->on_success(p->captured_file_path);
        }
        delete p;
      },
      p);
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_start_capture fail - error[%d]: %s", error,
               get_error_message(error));

  if (error != CAMERA_ERROR_NONE) {
    delete p;
    on_failure(get_error_message(error), "camera_start_capture fail");
    return false;
  }
  return true;
}

bool CameraDevice::StartAutoFocusing(bool continuous) {
  int error = camera_start_focusing(handle_, continuous);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_start_focusing fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::UnsetMediaPacketPreviewCb() {
  int error = camera_unset_media_packet_preview_cb(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_unset_media_packet_preview_cb fail - error[%d]: %s",
                    error, get_error_message(error));

  return true;
}

bool CameraDevice::UnsetAutoFocusChangedCb() {
  int error = camera_unset_focus_changed_cb(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_unset_focus_changed_cb fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::StartPreview() {
  int error = camera_start_preview(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_start_preview fail - error[%d]: %s", error,
                    get_error_message(error));

  GetState(state_);
  return true;
}

bool CameraDevice::StopAutoFocusing() {
  int error = camera_cancel_focusing(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_cancel_focusing fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::StopPreview() {
  int error = camera_stop_preview(handle_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_stop_preview fail - error[%d]: %s", error,
                    get_error_message(error));

  GetState(state_);
  return true;
}
