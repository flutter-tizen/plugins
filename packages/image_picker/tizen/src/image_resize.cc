// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "image_resize.h"

#include <app_common.h>

#include <algorithm>

#include "log.h"

static std::string imageUitlErrorToString(int error) {
  switch (error) {
    case IMAGE_UTIL_ERROR_NONE:
      return "IMAGE_UTIL - Successful";
    case IMAGE_UTIL_ERROR_INVALID_PARAMETER:
      return "IMAGE_UTIL - Invalid parameter";
    case IMAGE_UTIL_ERROR_OUT_OF_MEMORY:
      return "IMAGE_UTIL - Out of Memory";
    case IMAGE_UTIL_ERROR_NO_SUCH_FILE:
      return "IMAGE_UTIL - No such file";
    case IMAGE_UTIL_ERROR_INVALID_OPERATION:
      return "IMAGE_UTIL - Internal error";
    case IMAGE_UTIL_ERROR_NOT_SUPPORTED_FORMAT:
      return "IMAGE_UTIL - Not supported format";
    case IMAGE_UTIL_ERROR_PERMISSION_DENIED:
      return "IMAGE_UTIL - Permission denied";
    case IMAGE_UTIL_ERROR_NOT_SUPPORTED:
      return "IMAGE_UTIL - Not supported";
    default:
      return "IMAGE_UTIL - Unknown Error";
  }
}

void ImageResize::SetSize(unsigned int w, unsigned int h, int q) {
  max_width_ = w;
  max_height_ = h;
  quality_ = q;
}

bool ImageResize::DecodeImage(image_util_decode_h decode_h,
                              image_util_image_h& src_image,
                              const std::string& src_file) {
  int ret = image_util_decode_set_input_path(decode_h, src_file.c_str());
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_decode_set_input_path fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    return false;
  }

  // TODO: we need to check this api later
  // ret = image_util_decode_set_colorspace(decode_h, colorspace);

  ret = image_util_decode_run2(decode_h, &src_image);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_decode_run2 fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    return false;
  }

  return true;
}

bool ImageResize::TransformImage(transformation_h transform_h,
                                 image_util_image_h src_image,
                                 image_util_image_h& dst_image) {
  unsigned int org_width;
  unsigned int org_height;
  int ret = image_util_get_image(src_image, &org_width, &org_height, nullptr,
                                 nullptr, nullptr);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_get_image fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    return false;
  }

  bool has_max_width = max_width_ != 0;
  bool has_max_height = max_height_ != 0;

  unsigned int width =
      has_max_width ? std::min(org_width, max_width_) : org_width;
  unsigned int height =
      has_max_height ? std::min(org_height, max_height_) : org_height;

  bool should_downscale_width = has_max_width && max_width_ < org_width;
  bool should_downscale_height = has_max_height && max_height_ < org_height;

  if (should_downscale_width || should_downscale_height) {
    unsigned int downscaled_width = (height / (float)org_height) * org_width;
    unsigned int downscaled_height = (width / (float)org_width) * org_height;

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
      if (org_width < org_height) {
        width = downscaled_width;
      } else if (org_height < org_width) {
        height = downscaled_height;
      }
    }
  }

  LOG_DEBUG("transform width:[%d], height:[%d]", width, height);
  ret = image_util_transform_set_resolution(transform_h, width, height);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_transform_set_resolution fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    return false;
  }
  ret = image_util_transform_run2(transform_h, src_image, &dst_image);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_transform_run2 fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    return false;
  }

  return true;
}

bool ImageResize::EncodeImage(image_util_encode_h encode_h,
                              image_util_image_h dst_image,
                              image_util_type_e encoder_type,
                              const std::string& dst_file) {
  if (encoder_type == IMAGE_UTIL_JPEG) {
    if (quality_ > 0 && quality_ <= 100) {
      LOG_DEBUG("quality_ [%d]", quality_);
      int ret = image_util_encode_set_quality(encode_h, quality_);
      if (ret != IMAGE_UTIL_ERROR_NONE) {
        LOG_ERROR("image_util_encode_set_quality fail! [%s]",
                  imageUitlErrorToString(ret).c_str());
        return false;
      }
    }
  } else {
    LOG_DEBUG(
        "image_picker: image quality option supports only JPG type.. "
        "Returning the image with original quality");
  }

  LOG_DEBUG("dst_path : %s", dst_file.c_str());
  int ret =
      image_util_encode_run_to_file(encode_h, dst_image, dst_file.c_str());
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_encode_run_to_file fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    return false;
  }

  return true;
}

bool ImageResize::Resize(const std::string& src_file, std::string& dst_file) {
  LOG_DEBUG("source image path: %s", src_file.c_str());

  if (max_width_ == 0 && max_height_ == 0 &&
      (quality_ <= 0 || quality_ > 100)) {
    return false;
  }

  // ===========================================================
  image_util_image_h src_image = nullptr;
  image_util_image_h dst_image = nullptr;
  image_util_decode_h decode_h = nullptr;

  int ret = image_util_decode_create(&decode_h);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_decode_create fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    return false;
  }
  bool is_decoded = DecodeImage(decode_h, src_image, src_file);
  image_util_decode_destroy(decode_h);
  if (!is_decoded) {
    if (src_image) {
      image_util_destroy_image(src_image);
    }
    return false;
  }

  // ===========================================================
  transformation_h transform_h = nullptr;
  ret = image_util_transform_create(&transform_h);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_transform_create fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    if (src_image) {
      image_util_destroy_image(src_image);
    }
    return false;
  }

  bool is_transformed = TransformImage(transform_h, src_image, dst_image);
  image_util_transform_destroy(transform_h);
  if (!is_transformed) {
    if (src_image) {
      image_util_destroy_image(src_image);
    }
    if (dst_image) {
      image_util_destroy_image(dst_image);
    }
    return false;
  }

  image_util_destroy_image(src_image);

  // ===========================================================
  char* temp = app_get_cache_path();
  dst_file = std::string(temp);
  free(temp);

  size_t found = src_file.rfind("/");
  if (found != std::string::npos) {
    dst_file += "scaled_" + src_file.substr(found + 1);
    LOG_DEBUG("dest image path: %s", dst_file.c_str());
  } else {
    if (dst_image) {
      image_util_destroy_image(dst_image);
    }
    return false;
  }

  image_util_type_e encoder_type = IMAGE_UTIL_JPEG;
  size_t pos = dst_file.rfind(".");
  if (pos != std::string::npos) {
    std::string ext = dst_file.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (ext == "png") {
      encoder_type = IMAGE_UTIL_PNG;
    } else if (ext == "gif") {
      encoder_type = IMAGE_UTIL_GIF;
    } else if (ext == "bmp") {
      encoder_type = IMAGE_UTIL_BMP;
    }
  }

  image_util_encode_h encode_h = nullptr;
  ret = image_util_encode_create(encoder_type, &encode_h);
  if (ret != IMAGE_UTIL_ERROR_NONE) {
    LOG_ERROR("image_util_encode_create fail! [%s]",
              imageUitlErrorToString(ret).c_str());
    if (dst_image) {
      image_util_destroy_image(dst_image);
    }
    return false;
  }
  bool is_encoded = EncodeImage(encode_h, dst_image, encoder_type, dst_file);
  image_util_encode_destroy(encode_h);
  if (!is_encoded) {
    if (dst_image) {
      image_util_destroy_image(dst_image);
    }
    return false;
  }

  image_util_destroy_image(dst_image);

  return true;
}
