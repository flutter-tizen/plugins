// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CAMERA_DEVICE_H_
#define FLUTTER_PLUGIN_CAMERA_DEVICE_H_

#include <camera.h>
#include <flutter/encodable_value.h>
#include <flutter/method_result.h>
#include <flutter/plugin_registrar.h>
#include <recorder.h>

#include <mutex>

#include "camera_method_channel.h"
#include "device_method_channel.h"
#include "orientation_manager.h"

#define kCameraDeviceError "CameraDeviceError"

using CameraCapturingCb = camera_capturing_cb;
using CameraCaptureCompletedCb = camera_capture_completed_cb;
using CameraFocusChangedCb = camera_focus_changed_cb;
using CameraPrivewCb = camera_preview_cb;
using CameraMediaPacketPreviewCb = camera_media_packet_preview_cb;

using RecorderRecordingLimitReachedCb = recorder_recording_limit_reached_cb;
using RecorderStateChangedCb = recorder_state_changed_cb;
using RecorderStateChangedCb = recorder_state_changed_cb;

using ForeachResolutionCb = std::function<bool(int width, int height)>;
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
  kYUV420 = CAMERA_PIXEL_FORMAT_I420,
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

enum class CameraFlashMode {
  kOff = CAMERA_ATTR_FLASH_MODE_OFF,
  kOn = CAMERA_ATTR_FLASH_MODE_ON,
  kAuto = CAMERA_ATTR_FLASH_MODE_AUTO,
  kPermanent = CAMERA_ATTR_FLASH_MODE_PERMANENT,
};

enum class CameraExposureMode {
  kOff = CAMERA_ATTR_EXPOSURE_MODE_OFF,  // Not supported on TM1
  kAll = CAMERA_ATTR_EXPOSURE_MODE_ALL,
  kCenter = CAMERA_ATTR_EXPOSURE_MODE_CENTER,
  kSpot = CAMERA_ATTR_EXPOSURE_MODE_SPOT,
  kCustom = CAMERA_ATTR_EXPOSURE_MODE_CUSTOM,
};

enum class RecorderAudioChannel {
  kMono = 1,
  kStereo = 2,
};

enum class RecorderAudioCodec {
  kDisable = RECORDER_AUDIO_CODEC_DISABLE,
  kAMR = RECORDER_AUDIO_CODEC_AMR,
  kAAC = RECORDER_AUDIO_CODEC_AAC,
  kVorbis = RECORDER_AUDIO_CODEC_VORBIS,
  kPCM = RECORDER_AUDIO_CODEC_PCM,
  kMP3 = RECORDER_AUDIO_CODEC_MP3,
};

enum class RecorderAudioDevice {
  kMic = RECORDER_AUDIO_DEVICE_MIC,
  kModem = RECORDER_AUDIO_DEVICE_MODEM,
};

enum class RecorderState {
  kNone = RECORDER_STATE_NONE,
  kCreated = RECORDER_STATE_CREATED,
  kReady = RECORDER_STATE_READY,
  kRecording = RECORDER_STATE_RECORDING,
  kPaused = RECORDER_STATE_PAUSED,
};

enum class RecorderFileFormat {
  k3GP = RECORDER_FILE_FORMAT_3GP,
  kMP4 = RECORDER_FILE_FORMAT_MP4,
  kAMR = RECORDER_FILE_FORMAT_AMR,
  kADTS = RECORDER_FILE_FORMAT_ADTS,
  kWAV = RECORDER_FILE_FORMAT_WAV,
  kOGG = RECORDER_FILE_FORMAT_OGG,
  kM2TS = RECORDER_FILE_FORMAT_M2TS,
};

enum class RecorderVideoCodec {
  kH263 = RECORDER_VIDEO_CODEC_H263,
  kH264 = RECORDER_VIDEO_CODEC_H264,
  kMPEG4 = RECORDER_VIDEO_CODEC_MPEG4,
  kTHEORA = RECORDER_VIDEO_CODEC_THEORA,
};

enum class RecorderOrientationTag {
  kNone = RECORDER_ROTATION_NONE,
  k90 = RECORDER_ROTATION_90,
  k180 = RECORDER_ROTATION_180,
  k270 = RECORDER_ROTATION_270,
};

enum class ExposureMode {
  kAuto,
  kLocked,
};
bool ExposureModeToString(ExposureMode exposure_mode, std::string &mode);
bool StringToExposureMode(std::string mode, ExposureMode &exposure_mode);

enum class FlashMode {
  kOff,
  kAuto,
  kAlways,
  kTorch,
};
bool StringToFlashMode(std::string mode, FlashMode &flash_mode);

enum class FocusMode {
  kAuto,
  kLocked,
};
bool FocusModeToString(FocusMode focus_mode, std::string &mode);
bool StringToFocusMode(std::string mode, FocusMode &focus_mode);

// These resolution values came from resolution_preset.dart
// These may not be supported by device.
// Note : Only kMedium is supported on TM1
enum class ResolutionPreset {
  kLow,        // 320x240 (352x288 on iOS, 240p (320x240) on Android
  kMedium,     // 480p 640x480 (640x480 on iOS, 720x480 on Android)
  kHigh,       // 720p 1280x720
  kVeryHigh,   // 1080p 1920x1080
  kUltraHigh,  // 2160p 3840x2160
  kMax,        // The highest resolution available.
};
bool StringToResolutionPreset(std::string preset,
                              ResolutionPreset &resolution_preset);

struct Size {
  // Dart implementation use double as a unit of preview size
  double width;
  double height;
};

class CameraDeviceError {
 public:
  CameraDeviceError(const std::string &error_code,
                    const std::string &error_message)
      : error_code_(error_code), error_message_(error_message) {}
  CameraDeviceError(const std::string &error_message)
      : error_code_(kCameraDeviceError), error_message_(error_message) {}
  std::string GetErrorCode() const { return error_code_; }
  std::string GetErrorMessage() const { return error_message_; }

 private:
  std::string error_code_;
  std::string error_message_;
};

class CameraDevice {
 public:
  static flutter::EncodableValue GetAvailableCameras();

  CameraDevice();
  CameraDevice(flutter::PluginRegistrar *registrar, CameraDeviceType typem,
               ResolutionPreset resolution_preset, bool enable_audio);
  ~CameraDevice();

  void ChangeCameraDeviceType(CameraDeviceType type);
  void Dispose();
  Size GetRecommendedPreviewResolution();
  long GetTextureId() { return texture_id_; }
  double GetMaxExposureOffset();
  double GetMinExposureOffset();
  double GetMaxZoomLevel();
  double GetMinZoomLevel();
  void Open(std::string image_format_group,
            std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
                &&result) noexcept;
  void PauseVideoRecording(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
          &&result) noexcept;
  void RestFocusPoint();
  void ResumeVideoRecording(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
          &&result) noexcept;
  void SetExposureMode(ExposureMode exposure_mode);
  void SetExposureOffset(double exposure_offset);
  void SetFlashMode(FlashMode flash_mode);
  void SetFocusMode(FocusMode focus_mode);
  void SetFocusPoint(double x, double y);
  void SetResolutionPreset(ResolutionPreset resolution_preset);
  void SetZoomLevel(double zoom_level);
  void StartVideoRecording(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
          &&result) noexcept;
  void StopVideoRecording(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
          &&result) noexcept;
  void TakePicture(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
          &&result) noexcept;

  void LockCaptureOrientation(OrientationType orientation);
  void UnlockCaptureOrientation();

  void PausePreview() { is_preview_paused_ = true; }
  void ResumePreview() { is_preview_paused_ = false; }

  void ReleaseMediaPacket();

 private:
  bool CreateCamera();
  bool ClearCameraAutoFocusArea();
  bool DestroyCamera();
  bool ForeachCameraSupportedCaptureResolutions(
      const ForeachResolutionCb &callback);
  bool GetCameraCaptureResolution(int &width, int &height);
  bool GetCameraDeviceCount(int &count);
  bool GetCameraFocusMode(CameraAutoFocusMode &mode);
  bool GetCameraLensOrientation(int &angle);
  bool GetCameraPreviewResolution(int &width, int &height);
  bool GetCameraState(CameraDeviceState &state);
  bool GetCameraZoomRange(int &min, int &max);
  bool IsCameraSupportedCaptureResolution(std::pair<int, int> resolution);
  bool SetCameraFlashMode(CameraFlashMode mode);
  bool SetCameraFlip(CameraFlip flip);
  bool SetCameraExposure(int offset);
  bool SetCameraExposureMode(CameraExposureMode mode);
  bool GetCameraExposureRange(int &min, int &max);
  bool SetCameraCaptureFormat(CameraPixelFormat format);
  bool SetCameraCaptureResolution(int width, int height);
  bool SetCameraExifTagEnable(bool enable);
  bool SetCameraExifTagOrientatoin(ExifTagOrientation orientation);
  bool SetCameraAutoFocusMode(CameraAutoFocusMode mode);
  bool SetCameraAutoFocusArea(int x, int y);
  bool SetCameraAutoFocusChangedCb(CameraFocusChangedCb callback);
  bool SetCameraMediaPacketPreviewCb(CameraMediaPacketPreviewCb callback);
  bool SetCameraPreviewCb(CameraPrivewCb callback);
  bool SetCameraPreviewFormat(CameraPixelFormat format);
  bool SetCameraPreviewSize(Size size);
  bool SetCameraZoom(int zoom);
  bool StartCameraCapture(const OnCaptureSuccessCb &on_success,
                          const OnCaptureFailureCb &on_failure);
  bool StartCameraAutoFocusing(bool continuous);
  bool StartCameraPreview();
  bool StopCameraAutoFocusing();
  bool StopCameraPreview();
  bool UnsetCameraMediaPacketPreviewCb();
  bool UnsetCameraAutoFocusChangedCb();

  bool CancleRecorder();
  bool CreateRecorder();
  bool CommitRecorder();
  bool DestroyRecorder();
  bool ForeachRecorderSupprotedVideoResolutions(
      const ForeachResolutionCb &callback);
  bool GetRecorderState(RecorderState &state);
  bool GetRecorderFileName(std::string &name);
  bool GetRecorderVideoResolution(int &width, int &height);
  bool IsRecorderSupportedVideoResolution(std::pair<int, int> resolution);
  bool SetRecorderAudioChannel(RecorderAudioChannel chennel);
  bool SetRecorderAudioDevice(RecorderAudioDevice device);
  bool SetRecorderAudioEncorder(RecorderAudioCodec codec);
  bool SetRecorderAudioSamplerate(int samplerate);
  bool SetRecorderFileFormat(RecorderFileFormat format);
  bool SetRecorderFileName(std::string &name);
  bool SetRecorderOrientationTag(RecorderOrientationTag tag);
  bool SetRecorderRecordingLimitReachedCb(
      RecorderRecordingLimitReachedCb callback);
  bool SetRecorderStateChangedCb(RecorderStateChangedCb callback);
  bool SetRecorderVideoEncorder(RecorderVideoCodec codec);
  bool SetRecorderVideoEncorderBitrate(int bitrate);
  bool SetRecorderVideoResolution(int width, int height);

  bool PauseRecorder();
  bool PrepareRecorder();
  bool StartRecorder();
  bool UnprepareRecorder();
  bool UnsetRecorderRecordingLimitReachedCb();
  void UpdateStates();

  long texture_id_{0};
  flutter::PluginRegistrar *registrar_{nullptr};
  std::unique_ptr<flutter::TextureVariant> texture_variant_;
  std::unique_ptr<FlutterDesktopGpuSurfaceDescriptor> gpu_surface_;
  media_packet_h current_packet_{nullptr};

  std::mutex mutex_;

  std::unique_ptr<CameraMethodChannel> camera_method_channel_;
  std::unique_ptr<DeviceMethodChannel> device_method_channel_;
  std::unique_ptr<OrientationManager> orientation_manager_;

  camera_h camera_{nullptr};

  CameraDeviceState camera_state_{CameraDeviceState::kNone};
  CameraDeviceType type_{CameraDeviceType::kRear};

  ExposureMode exposure_mode_{ExposureMode::kAuto};
  FocusMode focus_mode_{FocusMode::kAuto};

  int preview_width_{0};
  int preview_height_{0};

  recorder_h recorder_{nullptr};
  RecorderState recorder_state_{RecorderState::kNone};

  OrientationType locked_orientation_{OrientationType::kPortraitUp};
  bool is_orientation_locked_{false};
  int zoom_level_{0};

  ResolutionPreset resolution_preset_{ResolutionPreset::kLow};
  std::vector<std::pair<int, int>> supported_camera_resolutions_;
  std::vector<std::pair<int, int>> supported_recorder_resolutions_;

  bool enable_audio_{true};
  bool is_preview_paused_{false};
};

#endif
