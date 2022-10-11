// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_CIPHER_H_
#define FLUTTER_PLUGIN_CIPHER_H_

#include <string>
#include <vector>

class Cipher {
 public:
  Cipher(std::string aes_key_name, size_t initialization_vector_size);
  ~Cipher();

  std::string Encrypt(const std::string &value);

  std::string Decrypt(const std::string &value);

 private:
  void CreateAesKeyOnce();
  std::vector<unsigned char> GenerateRandomVector();

  std::string aes_key_name_;
  size_t initialization_vector_size_;
};

#endif  // FLUTTER_PLUGIN_CIPHER_H_
