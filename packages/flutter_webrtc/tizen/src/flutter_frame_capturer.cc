#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "flutter_frame_capturer.h"

#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include "svpng.hpp"

namespace flutter_webrtc_plugin {

FlutterFrameCapturer::FlutterFrameCapturer(RTCVideoTrack* track,
                                           std::string path) {
  track_ = track;
  path_ = path;
}

void FlutterFrameCapturer::OnFrame(scoped_refptr<RTCVideoFrame> frame) {
  if (frame_ != nullptr) {
    return;
  }

  frame_ = frame.get()->Copy();
  catch_frame_ = true;
  cv_.notify_one();
}

void FlutterFrameCapturer::CaptureFrame(
    std::unique_ptr<MethodResultProxy> result) {
  std::unique_lock<std::mutex> lock(mutex_);
  // Here init catch_frame_ flag
  catch_frame_ = false;
  frame_ = nullptr;

  track_->AddRenderer(this);

  // Wait for frame with timeout (5 seconds)
  bool timeout = !cv_.wait_for(lock, std::chrono::seconds(5),
                               [this] { return catch_frame_.load(); });

  track_->RemoveRenderer(this);

  if (timeout) {
    std::shared_ptr<MethodResultProxy> result_ptr(result.release());
    result_ptr->Error("2", "Frame capture timed out");
    return;
  }

  bool success = SaveFrame();

  std::shared_ptr<MethodResultProxy> result_ptr(result.release());
  if (success) {
    result_ptr->Success();
  } else {
    result_ptr->Error("1", "Cannot save the frame as .png file");
  }
}

bool FlutterFrameCapturer::SaveFrame() {
  if (frame_ == nullptr) {
    return false;
  }

  int width = frame_.get()->width();
  int height = frame_.get()->height();
  int bytes_per_pixel = 4;
  std::vector<uint8_t> pixels(width * height * bytes_per_pixel);

  frame_.get()->ConvertToARGB(RTCVideoFrame::Type::kABGR, pixels.data(),
                              /* unused */ -1, width, height);

  FILE* file = fopen(path_.c_str(), "wb");
  if (!file) {
    return false;
  }

  svpng(file, width, height, pixels.data(), 1);
  fclose(file);
  return true;
}

}  // namespace flutter_webrtc_plugin