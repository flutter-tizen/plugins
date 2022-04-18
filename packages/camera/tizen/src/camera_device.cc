// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "camera_device.h"

#include <app_common.h>
#include <flutter/encodable_value.h>
#include <sys/time.h>

#include <cmath>

#include "log.h"

// These macros came from tizen camera_app
#define VIDEO_ENCODE_BITRATE 40000000 /* bps */
#define AUDIO_SOURCE_SAMPLERATE_AAC 44100

namespace {

uint64_t Timestamp() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return (uint64_t)tv.tv_sec * 1000UL + tv.tv_usec / 1000UL;
}

std::string CreateTempFileName(const std::string &prefix,
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

ExifTagOrientation ChooseExifTagOrientatoin(OrientationType device_orientation,
                                            bool is_front_lens_facing) {
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

RecorderOrientationTag ChooseRecorderOrientationTag(
    OrientationType device_orientation) {
  RecorderOrientationTag tag = RecorderOrientationTag::kNone;
  switch (device_orientation) {
    case OrientationType::kPortraitUp:
      tag = RecorderOrientationTag::k90;
      break;
    case OrientationType::kLandscapeLeft:
      tag = RecorderOrientationTag::k180;
      break;
    case OrientationType::kPortraitDown:
      tag = RecorderOrientationTag::k270;
      break;
    case OrientationType::kLandscapeRight:
      tag = RecorderOrientationTag::kNone;
      break;
    default:
      LOG_ERROR("Unknown RecorderOrientationTag!");
      break;
  }
  return tag;
}

}  // namespace

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

bool StringToResolutionPreset(std::string preset,
                              ResolutionPreset &resolution_preset) {
  LOG_DEBUG("mode[%s]", preset.c_str());
  if (preset == "low") {
    resolution_preset = ResolutionPreset::kLow;
    return true;
  } else if (preset == "medium") {
    resolution_preset = ResolutionPreset::kMedium;
    return true;
  } else if (preset == "high") {
    resolution_preset = ResolutionPreset::kHigh;
    return true;
  } else if (preset == "veryHigh") {
    resolution_preset = ResolutionPreset::kVeryHigh;
    return true;
  } else if (preset == "ultraHigh") {
    resolution_preset = ResolutionPreset::kUltraHigh;
    return true;
  } else if (preset == "max") {
    resolution_preset = ResolutionPreset::kMax;
    return true;
  }
  LOG_WARN("Unknown ResolutionPreset!");
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
  default_camera.GetCameraDeviceCount(count);

  flutter::EncodableList cameras;
  for (int i = 0; i < count; i++) {
    flutter::EncodableMap camera;
    camera[flutter::EncodableValue("name")] =
        flutter::EncodableValue("camera" + std::to_string(i + 1));

    int angle = 0;
    default_camera.GetCameraLensOrientation(angle);
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
  CreateCamera();
  GetCameraState(camera_state_);
}

CameraDevice::CameraDevice(flutter::PluginRegistrar *registrar,
                           CameraDeviceType type,
                           ResolutionPreset resolution_preset,
                           bool enable_audio)
    : registrar_(registrar),
      type_(type),
      resolution_preset_(resolution_preset),
      enable_audio_(enable_audio) {
  // Init handles
  CreateCamera();
  CreateRecorder();

  // Init camera
  SetCameraExifTagEnable(true);
  SetCameraAutoFocusMode(CameraAutoFocusMode::kNormal);
  if (type == CameraDeviceType::kFront) {
    SetCameraFlip(CameraFlip::kVertical);
  }

  GetCameraPreviewResolution(preview_width_, preview_height_);

  // Init recoder
  SetRecorderFileFormat(RecorderFileFormat::kMP4);
  if (!enable_audio_) {
    LOG_DEBUG("Disable audio");
    SetRecorderAudioEncorder(RecorderAudioCodec::kDisable);
  } else {
    SetRecorderAudioEncorder(RecorderAudioCodec::kAAC);
    SetRecorderAudioChannel(RecorderAudioChannel::kStereo);
    SetRecorderAudioDevice(RecorderAudioDevice::kMic);
    SetRecorderAudioSamplerate(AUDIO_SOURCE_SAMPLERATE_AAC);
  }

  SetRecorderVideoEncorder(RecorderVideoCodec::kH264);
  SetRecorderVideoEncorderBitrate(VIDEO_ENCODE_BITRATE);

  SetRecorderRecordingLimitReachedCb(
      [](recorder_recording_limit_type_e type, void *data) {
        LOG_WARN("Recording limit reached: %d\n", type);
      });
  SetRecorderStateChangedCb([](recorder_state_e previous,
                               recorder_state_e current, bool by_asm,
                               void *data) {
    LOG_DEBUG("Recorder state is changed : prev[%d], curr[%d]", previous,
              current);
    auto self = (CameraDevice *)data;
    if (previous != current) {
      self->UpdateStates();
    }
  });

  // Gather supported resolutions
  ForeachCameraSupportedCaptureResolutions(
      [this](int supported_width, int supported_height) -> bool {
        LOG_DEBUG("supported camera capture resolution width[%d] height[%d]",
                  supported_width, supported_height);
        std::pair<int, int> resolution = {supported_width, supported_height};
        supported_camera_resolutions_.emplace_back(resolution);
        return true;
      });
  ForeachRecorderSupprotedVideoResolutions(
      [this](int supported_width, int supported_height) -> bool {
        LOG_DEBUG("supported recorder video resolution width[%d] height[%d]",
                  supported_width, supported_height);
        std::pair<int, int> resolution = {supported_width, supported_height};
        supported_recorder_resolutions_.emplace_back(resolution);
        return true;
      });

  SetResolutionPreset(resolution_preset_);

  // Init channels
  texture_variant_ =
      std::make_unique<flutter::TextureVariant>(flutter::GpuBufferTexture(
          [this](size_t width,
                 size_t height) -> const FlutterDesktopGpuBuffer * {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!current_packet_) {
              return nullptr;
            }
            tbm_surface_h surface = nullptr;
            int ret = media_packet_get_tbm_surface(current_packet_, &surface);
            if (ret != MEDIA_PACKET_ERROR_NONE) {
              LOG_ERROR("media_packet_get_tbm_surface failed, error: %d", ret);
              media_packet_destroy(current_packet_);
              current_packet_ = nullptr;
              return nullptr;
            }
            flutter_desktop_gpu_buffer_->buffer = surface;
            flutter_desktop_gpu_buffer_->width = width;
            flutter_desktop_gpu_buffer_->height = height;
            flutter_desktop_gpu_buffer_->release_callback =
                [](void *release_context) {
                  CameraDevice *cd = (CameraDevice *)release_context;
                  cd->ReleaseMediaPacket();
                };
            flutter_desktop_gpu_buffer_->release_context = this;
            return flutter_desktop_gpu_buffer_.get();
          }));
  texture_id_ =
      registrar_->texture_registrar()->RegisterTexture(texture_variant_.get());
  flutter_desktop_gpu_buffer_ = std::make_unique<FlutterDesktopGpuBuffer>();

  LOG_DEBUG("texture_id_[%ld]", texture_id_);
  camera_method_channel_ =
      std::make_unique<CameraMethodChannel>(registrar_, texture_id_);
  device_method_channel_ = std::make_unique<DeviceMethodChannel>(registrar_);

  int angle = 0;
  GetCameraLensOrientation(angle);
  orientation_manager_ = std::make_unique<OrientationManager>(
      device_method_channel_.get(), (OrientationType)angle,
      type == CameraDeviceType::kFront);

  orientation_manager_->Start();

  UpdateStates();
}

CameraDevice::~CameraDevice() { Dispose(); }

void CameraDevice::ReleaseMediaPacket() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (current_packet_) {
    media_packet_destroy(current_packet_);
    current_packet_ = nullptr;
  }
}

bool CameraDevice::CreateCamera() {
  int error = camera_create((camera_device_e)type_, &camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_create fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::DestroyCamera() {
  int error = camera_destroy(camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_destroy fail - error[%d]: %s", error,
                    get_error_message(error));
  camera_ = nullptr;
  return true;
}

bool CameraDevice::ClearCameraAutoFocusArea() {
  int error = camera_attr_clear_af_area(camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_clear_af_area fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

void CameraDevice::ChangeCameraDeviceType(CameraDeviceType type) {
  int error = camera_change_device(camera_, (camera_device_e)type);
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_change_device fail - error[%d]: %s", error,
               get_error_message(error));
  type_ = type;
}

void CameraDevice::Dispose() {
  LOG_DEBUG("enter");
  if (recorder_) {
    DestroyRecorder();
  }

  if (camera_) {
    if (camera_state_ == CameraDeviceState::kPreview) {
      StopCameraPreview();
      UnsetCameraMediaPacketPreviewCb();
      UnsetCameraAutoFocusChangedCb();
    }
    DestroyCamera();
  }

  if (orientation_manager_) {
    orientation_manager_->Stop();
  }

  if (texture_id_ != 0) {
    registrar_->texture_registrar()->UnregisterTexture(texture_id_);
  }

  if (current_packet_) {
    media_packet_destroy(current_packet_);
    current_packet_ = nullptr;
  }
}

bool CameraDevice::ForeachCameraSupportedCaptureResolutions(
    const ForeachResolutionCb &callback) {
  int error = camera_foreach_supported_capture_resolution(
      camera_,
      [](int width, int height, void *callback) -> bool {
        auto cb = static_cast<ForeachResolutionCb *>(callback);
        return (*cb)(width, height);
      },
      (void *)&callback);
  RETV_LOG_ERROR_IF(
      error != CAMERA_ERROR_NONE, false,
      "camera_foreach_supported_capture_resolution fail - error[%d]: %s", error,
      get_error_message(error));
  return true;
}

bool CameraDevice::GetCameraCaptureResolution(int &width, int &height) {
  int error = camera_get_capture_resolution(camera_, &width, &height);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_get_capture_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetCameraDeviceCount(int &count) {
  // If the device supports primary and secondary camera, this returns 2. If 1
  // is returned, the device only supports primary camera.
  int error = camera_get_device_count(camera_, &count);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_get_device_count fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetCameraFocusMode(CameraAutoFocusMode &mode) {
  int error = camera_attr_get_af_mode(camera_, (camera_attr_af_mode_e *)&mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_get_af_mode fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetCameraLensOrientation(int &angle) {
  int error = camera_attr_get_lens_orientation(camera_, &angle);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_get_lens_orientation fail - error[%d]: %s",
                    error, get_error_message(error));

  return true;
}

bool CameraDevice::GetCameraPreviewResolution(int &width, int &height) {
  int w, h;
  int error = camera_get_preview_resolution(camera_, &w, &h);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_get_preview_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  width = w;
  height = h;
  return true;
}

bool CameraDevice::GetCameraState(CameraDeviceState &state) {
  int error = camera_get_state(camera_, (camera_state_e *)&state);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_get_state fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetCameraZoomRange(int &min, int &max) {
  int error = camera_attr_get_zoom_range(camera_, &min, &max);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_get_zoom_range fail - error[%d]: %s", error,
                    get_error_message(error));

  if (min == max) {
    // FIXME : The maximum and minimum values must not be the same,
    // if they are the same, zoom behavior will malfunction.
    LOG_WARN("maximum zoom level and minimum level are same!");
    return false;
  }

  if (min > max) {
    // According to the API doc, this means that it is not supported on device
    LOG_WARN("Not supported");
    return false;
  }
  return true;
}

bool CameraDevice::IsCameraSupportedCaptureResolution(
    std::pair<int, int> resolution) {
  auto iter = find_if(supported_camera_resolutions_.begin(),
                      supported_camera_resolutions_.end(),
                      [resolution](std::pair<int, int> supported) -> bool {
                        return supported.first == resolution.first &&
                               supported.second == resolution.second;
                      });
  return iter != supported_camera_resolutions_.end();
}

bool CameraDevice::SetCameraExifTagEnable(bool enable) {
  int error = camera_attr_enable_tag(camera_, enable);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_enable_tag fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::SetCameraExifTagOrientatoin(ExifTagOrientation orientation) {
  int error = camera_attr_set_tag_orientation(
      camera_, (camera_attr_tag_orientation_e)orientation);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_tag_orientation fail - error[%d]: %s",
                    error, get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraAutoFocusArea(int x, int y) {
  LOG_DEBUG("camera_attr_set_af_area x[%d], y[%d]", x, y);
  int error = camera_attr_set_af_area(camera_, x, y);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_af_area fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraAutoFocusChangedCb(CameraFocusChangedCb callback) {
  int error = camera_set_focus_changed_cb(camera_, callback, this);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_focus_changed_cb fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraAutoFocusMode(CameraAutoFocusMode mode) {
  int error = camera_attr_set_af_mode(camera_, (camera_attr_af_mode_e)mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_af_mode fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraExposure(int offset) {
  int error = camera_attr_set_exposure(camera_, offset);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_exposure fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraExposureMode(CameraExposureMode mode) {
  int error =
      camera_attr_set_exposure_mode(camera_, (camera_attr_exposure_mode_e)mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_exposure_mode fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetCameraExposureRange(int &min, int &max) {
  int error = camera_attr_get_exposure_range(camera_, &min, &max);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    " camera_attr_get_exposure_range fail - error[%d]: %s",
                    error, get_error_message(error));
  // According to the API doc, this means that it is not supported on device
  if (min > max) {
    // Not supported on TM1
    LOG_WARN("Not supported");
    return false;
  }
  return true;
}

bool CameraDevice::SetCameraFlashMode(CameraFlashMode mode) {
  int error =
      camera_attr_set_flash_mode(camera_, (camera_attr_flash_mode_e)mode);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_flash_mode fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::SetCameraFlip(CameraFlip flip) {
  int error = camera_attr_set_stream_flip(camera_, (camera_flip_e)flip);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_stream_flip fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::SetCameraCaptureFormat(CameraPixelFormat format) {
  int error = camera_set_capture_format(camera_, (camera_pixel_format_e)format);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_capture_format fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraCaptureResolution(int width, int height) {
  int error = camera_set_capture_resolution(camera_, width, height);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_capture_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::CancleRecorder() {
  int error = recorder_cancel(recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_cancel fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::CreateRecorder() {
  int error = recorder_create_videorecorder(camera_, &recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_create_videorecorder fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::CommitRecorder() {
  int error = recorder_commit(recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_commit fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::DestroyRecorder() {
  int error = recorder_destroy(recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_destroy fail - error[%d]: %s", error,
                    get_error_message(error));
  recorder_ = nullptr;
  return true;
}

bool CameraDevice::ForeachRecorderSupprotedVideoResolutions(
    const ForeachResolutionCb &callback) {
  int error = recorder_foreach_supported_video_resolution(
      recorder_,
      [](int width, int height, void *callback) -> bool {
        auto cb = static_cast<ForeachResolutionCb *>(callback);
        return (*cb)(width, height);
      },
      (void *)&callback);
  RETV_LOG_ERROR_IF(
      error != RECORDER_ERROR_NONE, false,
      "recorder_foreach_supported_video_resolution fail - error[%d]: %s", error,
      get_error_message(error));
  return true;
}

bool CameraDevice::GetRecorderState(RecorderState &state) {
  int error = recorder_get_state(recorder_, (recorder_state_e *)&state);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_get_state fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::GetRecorderFileName(std::string &name) {
  char *file_name;
  int error = recorder_get_filename(recorder_, &file_name);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_get_filename fail - error[%d]: %s", error,
                    get_error_message(error));
  name = file_name;
  free(file_name);
  return true;
}

bool CameraDevice::GetRecorderVideoResolution(int &width, int &height) {
  int error = recorder_get_video_resolution(recorder_, &width, &height);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_get_video_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::IsRecorderSupportedVideoResolution(
    std::pair<int, int> resolution) {
  auto iter = find_if(supported_recorder_resolutions_.begin(),
                      supported_recorder_resolutions_.end(),
                      [resolution](std::pair<int, int> supported) -> bool {
                        return supported.first == resolution.first &&
                               supported.second == resolution.second;
                      });
  return iter != supported_recorder_resolutions_.end();
}

bool CameraDevice::SetRecorderAudioChannel(RecorderAudioChannel chennel) {
  int error = recorder_attr_set_audio_channel(recorder_, (int)chennel);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_attr_set_audio_channel fail - error[%d]: %s",
                    error, get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderAudioDevice(RecorderAudioDevice device) {
  int error = recorder_attr_set_audio_device(recorder_,
                                             (recorder_audio_device_e)device);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_attr_set_audio_device fail - error[%d]: %s",
                    error, get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderAudioEncorder(RecorderAudioCodec codec) {
  int error =
      recorder_set_audio_encoder(recorder_, (recorder_audio_codec_e)codec);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_set_audio_encoder fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderAudioSamplerate(int samplerate) {
  int error = recorder_attr_set_audio_samplerate(recorder_, samplerate);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    " recorder_attr_set_audio_samplerate fail - error[%d]: %s",
                    error, get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderFileFormat(RecorderFileFormat format) {
  int error =
      recorder_set_file_format(recorder_, (recorder_file_format_e)format);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_set_file_format fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderFileName(std::string &name) {
  int error = recorder_set_filename(recorder_, name.c_str());
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_set_filename fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderOrientationTag(RecorderOrientationTag tag) {
  int error =
      recorder_attr_set_orientation_tag(recorder_, (recorder_rotation_e)tag);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_attr_set_orientation_tag fail - error[%d]: %s",
                    error, get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderRecordingLimitReachedCb(
    RecorderRecordingLimitReachedCb callback) {
  int error =
      recorder_set_recording_limit_reached_cb(recorder_, callback, this);
  RETV_LOG_ERROR_IF(
      error != RECORDER_ERROR_NONE, false,
      "recorder_set_recording_limit_reached_cb fail - error[%d]: %s", error,
      get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderStateChangedCb(RecorderStateChangedCb callback) {
  int error = recorder_set_state_changed_cb(recorder_, callback, this);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    " recorder_set_state_changed_cb fail - error[%d]: %s",
                    error, get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderVideoEncorder(RecorderVideoCodec codec) {
  int error =
      recorder_set_video_encoder(recorder_, (recorder_video_codec_e)codec);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_set_video_encoder fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderVideoEncorderBitrate(int bitrate) {
  int error = recorder_attr_set_video_encoder_bitrate(recorder_, bitrate);
  RETV_LOG_ERROR_IF(
      error != RECORDER_ERROR_NONE, false,
      " recorder_attr_set_video_encoder_bitrate fail - error[%d]: %s", error,
      get_error_message(error));
  return true;
}

bool CameraDevice::SetRecorderVideoResolution(int width, int height) {
  int error = recorder_set_video_resolution(recorder_, width, height);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_set_video_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::PauseRecorder() {
  int error = recorder_pause(recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_pause fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::PrepareRecorder() {
  int error = recorder_prepare(recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_prepare fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::StartRecorder() {
  int error = recorder_start(recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_start fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::UnprepareRecorder() {
  int error = recorder_unprepare(recorder_);
  RETV_LOG_ERROR_IF(error != RECORDER_ERROR_NONE, false,
                    "recorder_unprepare fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::UnsetRecorderRecordingLimitReachedCb() {
  int error = recorder_unset_recording_limit_reached_cb(recorder_);
  RETV_LOG_ERROR_IF(
      error != RECORDER_ERROR_NONE, false,
      "recorder_unset_recording_limit_reached_cb fail - error[%d]: %s", error,
      get_error_message(error));
  return true;
}

void CameraDevice::UpdateStates() {
  GetCameraState(camera_state_);
  GetRecorderState(recorder_state_);
}

Size CameraDevice::GetRecommendedPreviewResolution() {
  Size preview_size;
  int w, h;
  int error = camera_get_recommended_preview_resolution(camera_, &w, &h);
  LOG_ERROR_IF(error != CAMERA_ERROR_NONE,
               "camera_get_recommended_preview_resolution fail - error[%d]: %s",
               error, get_error_message(error));

  auto target_orientation =
      orientation_manager_->ConvertOrientation(OrientationType::kPortraitUp);
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

double CameraDevice::GetMaxExposureOffset() {
  int min = 0, max = 0;
  if (!GetCameraExposureRange(min, max)) {
    throw CameraDeviceError("Failed to get max exposure offset");
  }
  return static_cast<double>(max);
}

double CameraDevice::GetMinExposureOffset() {
  int min = 0, max = 0;
  if (!GetCameraExposureRange(min, max)) {
    throw CameraDeviceError("Failed to get min exposure offset");
  }
  return static_cast<double>(min);
}

double CameraDevice::GetMaxZoomLevel() {
  int min = 0, max = 0;
  if (!GetCameraZoomRange(min, max)) {
    throw CameraDeviceError("Failed to get max zoom level");
  }
  return static_cast<double>(max);
}

double CameraDevice::GetMinZoomLevel() {
  int min = 0, max = 0;
  if (!GetCameraZoomRange(min, max)) {
    throw CameraDeviceError("Failed to get min zoom level");
  }
  return static_cast<double>(min);
}

void CameraDevice::Open(
    std::string image_format_group,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
        &&result) noexcept {
  LOG_DEBUG("enter");
  LOG_DEBUG("Recieved image_format_group[%s]", image_format_group.c_str());
  CameraPixelFormat pixel_format;
  if (StringToCameraPixelFormat(image_format_group, pixel_format)) {
    LOG_DEBUG("Try to set pixel_format[%d]", static_cast<int>(pixel_format));
    SetCameraCaptureFormat(pixel_format);
    SetCameraPreviewFormat(pixel_format);
  }

  if (!SetCameraMediaPacketPreviewCb([](media_packet_h packet, void *data) {
        auto self = static_cast<CameraDevice *>(data);
        std::lock_guard<std::mutex> lock(self->mutex_);
        if (self->current_packet_) {
          media_packet_destroy(self->current_packet_);
          self->current_packet_ = packet;
          return;
        }
        if (self->is_preview_paused_) {
          media_packet_destroy(packet);
          self->current_packet_ = nullptr;
          return;
        }
        self->current_packet_ = packet;
        self->registrar_->texture_registrar()->MarkTextureFrameAvailable(
            self->texture_id_);
      })) {
    result->Error(kCameraDeviceError, "Failed to set media callback");
    return;
  }

  if (!StartCameraPreview()) {
    result->Error(kCameraDeviceError, "Failed to start preview");
    return;
  }

  SetCameraAutoFocusChangedCb([](camera_focus_state_e state, void *user_data) {
    LOG_DEBUG("Change auto focus state[%d]", static_cast<int>(state));
  });

  try {
    // Start or stop focusing according to FocusMode
    SetFocusMode(focus_mode_);
    // Start or stop exposure according to ExpoureMode
    SetExposureMode(exposure_mode_);
  } catch (const CameraDeviceError &error) {
    LOG_WARN("[%s] %s", error.GetErrorCode().c_str(),
             error.GetErrorMessage().c_str());
  }

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
  result->Success();
}

void CameraDevice::PauseVideoRecording(
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
        &&result) noexcept {
  LOG_DEBUG("enter");
  if (PauseRecorder()) {
    result->Success();
  } else {
    result->Error(kCameraDeviceError, "Failed to pause recorder");
  }
  UpdateStates();
}

void CameraDevice::RestFocusPoint() {
  LOG_DEBUG("enter");
  ClearCameraAutoFocusArea();
}

void CameraDevice::ResumeVideoRecording(
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
        &&result) noexcept {
  LOG_DEBUG("enter");
  if (StartRecorder()) {
    result->Success();
  } else {
    result->Error(kCameraDeviceError, "Failed to resume recorder");
  }
  UpdateStates();
}

void CameraDevice::SetExposureMode(ExposureMode exposure_mode) {
  bool ret = false;
  switch (exposure_mode) {
    case ExposureMode::kAuto:
      // Use CAMERA_ATTR_EXPOSURE_MODE_CENTER as default
      LOG_DEBUG("Start exposure");
      ret = SetCameraExposureMode(CameraExposureMode::kCenter);
      break;
    case ExposureMode::kLocked:
      LOG_DEBUG("Stop exposure");
      ret = SetCameraExposureMode(CameraExposureMode::kOff);
      break;
    default:
      LOG_WARN("Unknown ExposureMode!");
      break;
  }
  if (!ret) {
    throw CameraDeviceError("Failed to set exposure mode");
  }
  exposure_mode_ = exposure_mode;
}

void CameraDevice::SetExposureOffset(double exposure_offset) {
  int offset = static_cast<int>(round(exposure_offset));
  LOG_DEBUG("SetExposure [%d]", offset);
  if (!SetCameraExposure(offset)) {
    throw CameraDeviceError("Failed to set exposure offset");
  }
}

void CameraDevice::SetFlashMode(FlashMode flash_mode) {
  bool ret = false;
  switch (flash_mode) {
    case FlashMode::kOff:
      ret = SetCameraFlashMode(CameraFlashMode::kOff);
      break;
    case FlashMode::kAuto:
      ret = SetCameraFlashMode(CameraFlashMode::kAuto);
      break;
    case FlashMode::kAlways:
      ret = SetCameraFlashMode(CameraFlashMode::kOn);
      break;
    case FlashMode::kTorch:
      ret = SetCameraFlashMode(CameraFlashMode::kPermanent);
      break;
    default:
      LOG_WARN("Unknown flash mode!");
      break;
  }
  if (!ret) {
    throw CameraDeviceError("Failed to set flash mode");
  }
}

void CameraDevice::SetFocusMode(FocusMode focus_mode) {
  bool ret = false;
  switch (focus_mode) {
    case FocusMode::kAuto: {
      LOG_DEBUG("Start auto focusing");
      if (!(ret = StartCameraAutoFocusing(true))) {
        // Fallback, try to Auto focusing with non-continuous
        LOG_DEBUG("try to start auto focusing with non-continuous");
        ret = StartCameraAutoFocusing(false);
      }
    } break;
    case FocusMode::kLocked:
      LOG_DEBUG("Stop auto focusing");
      ret = StopCameraAutoFocusing();
      break;
    default:
      LOG_WARN("Unknown FocusMode!");
      break;
  }
  if (!ret) {
    throw CameraDeviceError("Failed to set focus mode");
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
  if (!SetCameraAutoFocusArea(computed_x, computed_y)) {
    throw CameraDeviceError("Failed to set auto focus area");
  }
  return;
}

void CameraDevice::SetResolutionPreset(ResolutionPreset resolution_preset) {
  std::pair<int, int> resolution{0, 0};
  LOG_DEBUG("ResolutionPreset[%d]", (int)resolution_preset);
  switch (resolution_preset) {
    case ResolutionPreset::kLow:
      resolution.first = 320;
      resolution.second = 240;
      break;
    case ResolutionPreset::kMedium:
      resolution.first = 640;
      resolution.second = 480;
      break;
    case ResolutionPreset::kHigh:
      resolution.first = 1280;
      resolution.second = 720;
      break;
    case ResolutionPreset::kVeryHigh:
      resolution.first = 1920;
      resolution.second = 1080;
      break;
    case ResolutionPreset::kUltraHigh:
      resolution.first = 3840;
      resolution.second = 2160;
      break;
    case ResolutionPreset::kMax: {
      // The highest resolution available
      SetCameraCaptureResolution(supported_camera_resolutions_.back().first,
                                 supported_camera_resolutions_.back().second);
      SetRecorderVideoResolution(supported_recorder_resolutions_.back().first,
                                 supported_recorder_resolutions_.back().second);
      return;
    } break;
    default:
      LOG_DEBUG("Unknown ResolutionPreset!");
      break;
  }
  if (IsCameraSupportedCaptureResolution(resolution)) {
    LOG_DEBUG("Set camera caputre resolution : width[%d], height[%d]",
              resolution.first, resolution.second);
    SetCameraCaptureResolution(resolution.first, resolution.second);
  } else {
    LOG_WARN("Not supported capture resolution!");
  }
  if (IsRecorderSupportedVideoResolution(resolution)) {
    LOG_DEBUG("Set recorder video resolution : width[%d], height[%d]",
              resolution.first, resolution.second);
    SetRecorderVideoResolution(resolution.first, resolution.second);
  } else {
    LOG_WARN("Not supported video resolution!");
  }
}

void CameraDevice::SetZoomLevel(double zoom_level) {
  auto z = static_cast<int>(round(zoom_level));
  if (zoom_level_ != z) {
    zoom_level_ = z;
    LOG_DEBUG("zoom_level_[%d]", zoom_level_);
    if (!SetCameraZoom(zoom_level_)) {
      throw CameraDeviceError("Failed to set zoom level");
    }
  }
}

void CameraDevice::StartVideoRecording(
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
        &&result) noexcept {
  LOG_DEBUG("enter");
  StopCameraPreview();

  std::string file_name = CreateTempFileName("REC", "mp4");
  SetRecorderFileName(file_name);
  SetRecorderOrientationTag(ChooseRecorderOrientationTag(
      is_orientation_locked_
          ? locked_orientation_
          : orientation_manager_->GetDeviceOrientationType()));
  PrepareRecorder();

  if (StartRecorder()) {
    result->Success();
  } else {
    result->Error(kCameraDeviceError, "Failed to start recorder");
  }
  UpdateStates();
}

void CameraDevice::StopVideoRecording(
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
        &&result) noexcept {
  LOG_DEBUG("enter");
  std::string file_name;
  int success = false;
  if (CommitRecorder() && GetRecorderFileName(file_name)) {
    success = true;
  }

  UnprepareRecorder();
  StartCameraPreview();

  if (success) {
    result->Success(flutter::EncodableValue(file_name));
  } else {
    result->Error(kCameraDeviceError, "Failed to stop recorder");
  }

  UpdateStates();
}

void CameraDevice::TakePicture(
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
        &&result) noexcept {
  SetCameraExifTagOrientatoin(ChooseExifTagOrientatoin(
      is_orientation_locked_ ? locked_orientation_
                             : orientation_manager_->GetDeviceOrientationType(),
      type_ == CameraDeviceType::kFront));
  auto p_result = result.release();
  if (!StartCameraCapture(
          [p_result, this](const std::string &captured_file_path) {
            flutter::EncodableValue value(captured_file_path);
            p_result->Success(value);
            StartCameraPreview();
            UpdateStates();
            delete p_result;
          },
          [p_result](const std::string &code, const std::string &message) {
            p_result->Error(code, message);
            delete p_result;
          })) {
    p_result->Error(kCameraDeviceError, "Failed to take picture");
    delete p_result;
  }
  UpdateStates();
}

void CameraDevice::LockCaptureOrientation(OrientationType orientation) {
  locked_orientation_ =
      orientation_manager_->ConvertOrientation(orientation, false);
  is_orientation_locked_ = true;

  LOG_DEBUG("Recived lock orientatoin[%d] -> Device orientation [%d]",
            (int)orientation, (int)locked_orientation_);
}

void CameraDevice::UnlockCaptureOrientation() {
  is_orientation_locked_ = false;
}

bool CameraDevice::SetCameraMediaPacketPreviewCb(
    CameraMediaPacketPreviewCb callback) {
  int error = camera_set_media_packet_preview_cb(camera_, callback, this);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_media_packet_preview_cb fail - error[%d]: %s",
                    error, get_error_message(error));

  return true;
}

bool CameraDevice::SetCameraPreviewCb(CameraPrivewCb callback) {
  int error = camera_set_preview_cb(camera_, callback, this);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_preview_cb fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraPreviewFormat(CameraPixelFormat format) {
  int error = camera_set_preview_format(camera_, (camera_pixel_format_e)format);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_preview_format fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraPreviewSize(Size size) {
  int w, h;
  w = static_cast<int>(round(size.width));
  h = static_cast<int>(round(size.height));

  LOG_DEBUG("camera_set_preview_resolution w[%d] h[%d]", w, h);

  int error = camera_set_preview_resolution(camera_, w, h);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_set_preview_resolution fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::SetCameraZoom(int zoom) {
  int error = camera_attr_set_zoom(camera_, zoom);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_attr_set_zoom fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::StartCameraCapture(const OnCaptureSuccessCb &on_success,
                                      const OnCaptureFailureCb &on_failure) {
  struct Param {
    OnCaptureSuccessCb on_success;
    OnCaptureFailureCb on_failure;
    std::string captured_file_path;
    std::string error;
    std::string error_message;
  };

  Param *p = new Param;  // Must delete on capture_completed_callback
  p->on_success = on_success;
  p->on_failure = on_failure;

  int error = camera_start_capture(
      camera_,
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
    return false;
  }
  return true;
}

bool CameraDevice::StartCameraAutoFocusing(bool continuous) {
  int error = camera_start_focusing(camera_, continuous);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_start_focusing fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::UnsetCameraMediaPacketPreviewCb() {
  int error = camera_unset_media_packet_preview_cb(camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_unset_media_packet_preview_cb fail - error[%d]: %s",
                    error, get_error_message(error));

  return true;
}

bool CameraDevice::UnsetCameraAutoFocusChangedCb() {
  int error = camera_unset_focus_changed_cb(camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_unset_focus_changed_cb fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::StartCameraPreview() {
  int error = camera_start_preview(camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_start_preview fail - error[%d]: %s", error,
                    get_error_message(error));
  return true;
}

bool CameraDevice::StopCameraAutoFocusing() {
  int error = camera_cancel_focusing(camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_cancel_focusing fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}

bool CameraDevice::StopCameraPreview() {
  int error = camera_stop_preview(camera_);
  RETV_LOG_ERROR_IF(error != CAMERA_ERROR_NONE, false,
                    "camera_stop_preview fail - error[%d]: %s", error,
                    get_error_message(error));

  return true;
}
