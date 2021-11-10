#ifndef FLUTTER_PLUGIN_IMAGE_RESIZE_H_
#define FLUTTER_PLUGIN_IMAGE_RESIZE_H_

#include <image_util.h>

#include <string>

class ImageResize {
 public:
  ImageResize() {}
  bool Resize(const std::string& src_file, std::string& dst_file);
  void SetSize(unsigned int w, unsigned int h, int q);

 private:
  bool DecodeImage(image_util_decode_h decode_h, image_util_image_h& src_image,
                   const std::string& src_file);
  bool TransformImage(transformation_h transform_h,
                      image_util_image_h src_image,
                      image_util_image_h& dst_image);
  bool EncodeImage(image_util_encode_h encode_h, image_util_image_h dst_image,
                   image_util_type_e encoder_type, const std::string& dst_file);
  unsigned int max_width_ = 0;
  unsigned int max_height_ = 0;
  int quality_ = 0;
};

#endif
