// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_IMAGE_RESIZE_H_
#define FLUTTER_PLUGIN_IMAGE_RESIZE_H_

#include <image_util.h>

#include <string>

class ImageResize {
 public:
  ImageResize() {}
  ~ImageResize() {}

  void SetSize(uint32_t max_width, uint32_t max_height, int32_t quality) {
    max_width_ = max_width;
    max_height_ = max_height;
    quality_ = quality;
  }

  bool Resize(const std::string& input_path, std::string* output_path);

 private:
  bool IsValidQuality() { return quality_ > 0 && quality_ < 100; }

  bool DecodeImage(const std::string& path, image_util_image_h* image);

  bool TransformImage(image_util_image_h input, image_util_image_h* output);

  bool EncodeImage(image_util_image_h image, const std::string& path);

  uint32_t max_width_ = 0;
  uint32_t max_height_ = 0;
  int32_t quality_ = 0;
};

#endif
