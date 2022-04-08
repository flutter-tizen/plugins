// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_resize.h"

#include <app_common.h>

#include <algorithm>

#include "log.h"

bool ImageResize::DecodeImage(const std::string& path,
                              image_util_image_h* image) {
  image_util_decode_h handle = nullptr;
  int ret = image_util_decode_create(&handle);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to initialize image decode util: %s",
              get_error_message(ret));
    return false;
  }

  ret = image_util_decode_set_input_path(handle, path.c_str());
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to set image path: %s", get_error_message(ret));
    image_util_decode_destroy(handle);
    return false;
  }

  ret = image_util_decode_run2(handle, image);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to decode image: %s", get_error_message(ret));
    image_util_decode_destroy(handle);
    return false;
  }

  image_util_decode_destroy(handle);
  return true;
}

bool ImageResize::TransformImage(image_util_image_h input,
                                 image_util_image_h* output) {
  transformation_h handle = nullptr;
  int ret = image_util_transform_create(&handle);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to initialize image transform util: %s",
              get_error_message(ret));
    return false;
  }

  uint32_t orig_width;
  uint32_t orig_height;
  ret = image_util_get_image(input, &orig_width, &orig_height, nullptr, nullptr,
                             nullptr);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to get image size: %s", get_error_message(ret));
    image_util_transform_destroy(handle);
    return false;
  }

  bool has_max_width = max_width_ != 0;
  bool has_max_height = max_height_ != 0;

  uint32_t width =
      has_max_width ? std::min(orig_width, max_width_) : orig_width;
  uint32_t height =
      has_max_height ? std::min(orig_height, max_height_) : orig_height;

  bool should_downscale_width = has_max_width && max_width_ < orig_width;
  bool should_downscale_height = has_max_height && max_height_ < orig_height;

  if (should_downscale_width || should_downscale_height) {
    uint32_t downscaled_width = (height / (float)orig_height) * orig_width;
    uint32_t downscaled_height = (width / (float)orig_width) * orig_height;

    if (width < height) {
      if (!has_max_width) {
        width = downscaled_width;
      } else {
        height = downscaled_height;
      }
    } else if (height < width) {
      if (!has_max_height) {
        height = downscaled_height;
      } else {
        width = downscaled_width;
      }
    } else {
      if (orig_width < orig_height) {
        width = downscaled_width;
      } else if (orig_height < orig_width) {
        height = downscaled_height;
      }
    }
  }

  LOG_DEBUG("Target image resolution: %d x %d", width, height);
  ret = image_util_transform_set_resolution(handle, width, height);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to set image resolution: %s", get_error_message(ret));
    image_util_transform_destroy(handle);
    return false;
  }

  ret = image_util_transform_run2(handle, input, output);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to transform image: %s", get_error_message(ret));
    image_util_transform_destroy(handle);
    return false;
  }

  image_util_transform_destroy(handle);
  return true;
}

bool ImageResize::EncodeImage(image_util_image_h image,
                              const std::string& path) {
  // Determine the image type from the output file extension.
  image_util_type_e type = IMAGE_UTIL_JPEG;
  size_t pos = path.rfind(".");
  if (pos != std::string::npos) {
    std::string extension = path.substr(pos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (extension == "png") {
      type = IMAGE_UTIL_PNG;
    } else if (extension == "gif") {
      type = IMAGE_UTIL_GIF;
    } else if (extension == "bmp") {
      type = IMAGE_UTIL_BMP;
    }
  }

  image_util_encode_h handle = nullptr;
  int ret = image_util_encode_create(type, &handle);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to initialize image encode util: %s",
              get_error_message(ret));
    return false;
  }

  if (type == IMAGE_UTIL_JPEG) {
    if (IsValidQuality()) {
      LOG_DEBUG("Target image quality: %d", quality_);
      int ret = image_util_encode_set_quality(handle, quality_);
      if (ret != IMAGE_UTIL_ERROR_NONE) {
        LOG_ERROR("Failed to set image quality: %s", get_error_message(ret));
        image_util_encode_destroy(handle);
        return false;
      }
    }
  } else {
    LOG_INFO(
        "image_picker: The image quality option only supports JPG type. "
        "Returning the image with original quality.");
  }

  LOG_DEBUG("Target image path: %s", path.c_str());
  ret = image_util_encode_run_to_file(handle, image, path.c_str());
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("Failed to encode image: %s", get_error_message(ret));
    image_util_encode_destroy(handle);
    return false;
  }

  image_util_encode_destroy(handle);
  return true;
}

bool ImageResize::Resize(const std::string& source_path,
                         std::string& dest_path) {
  bool should_scale = max_width_ != 0 || max_height_ != 0 || IsValidQuality();
  if (!should_scale) {
    return false;
  }

  image_util_image_h source_image = nullptr;
  if (!DecodeImage(source_path, &source_image)) {
    return false;
  }

  image_util_image_h image = nullptr;
  if (!TransformImage(source_image, &image)) {
    image_util_destroy_image(source_image);
    return false;
  }
  image_util_destroy_image(source_image);

  char* base_dir = app_get_cache_path();
  dest_path = std::string(base_dir);
  free(base_dir);

  size_t pos = source_path.rfind("/");
  if (pos != std::string::npos) {
    dest_path += "scaled_" + source_path.substr(pos + 1);
  } else {
    image_util_destroy_image(image);
    return false;
  }

  if (!EncodeImage(image, dest_path)) {
    image_util_destroy_image(image);
    return false;
  }
  image_util_destroy_image(image);

  return true;
}
