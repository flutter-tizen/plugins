/**
 * @file           external_drm.h
 * @brief          the extrnal drm information for elementary stream
 * @interfacetype  Module
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        0.0.1
 * @SDK_Support    N
 *
 * Copyright (c) 2018 Samsung Electronics Co., Ltd All Rights Reserved
 * PROPRIETARY/CONFIDENTIAL
 * This software is the confidential and proprietary
 * information of SAMSUNG ELECTRONICS ("Confidential Information"). You shall
 * not disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into with
 * SAMSUNG ELECTRONICS. SAMSUNG make no representations or warranties about the
 * suitability of the software, either express or implied, including but not
 * limited to the implied warranties of merchantability, fitness for a
 * particular purpose, or non-infringement. SAMSUNG shall not be liable for any
 * damages suffered by licensee as a result of using, modifying or distributing
 * this software or its derivatives.
 */
#ifndef __PLUSPLAYER_EXTERNAL_DRM_H__
#define __PLUSPLAYER_EXTERNAL_DRM_H__

#include <array>
#include <string>
#include <vector>

namespace plusplayer {

namespace drm {

using MetaData = void*;
/**
 * @brief  Enumerations for cipher algorithm for drm
 */
enum class DrmbEsCipherAlgorithm : int {
  kUnknown = -1,
  kRc4 = 0,
  kAes128Ctr = 1,
  kAes128Cbc = 2
};
/**
 * @brief  Enumerations for media format for drm
 */
enum class DrmbEsMediaFormat : int {
  kNone = 0,
  kFragmentedMp4 = 1,
  kTs = 2,
  kAsf = 3,
  kFragmentedMp4Audio = 4,
  kFragmentedMp4Video = 5,
  kCleanAudio = 6,  // Clean Audio Data
  kPes = 7,         // Packetized ES
};
/**
 * @brief  Enumerations for cipher phase for drm
 */
enum class DrmbEsCipherPhase : int {
  kNone = 0,
  kInit = 1,
  kUpdate = 2,
  kFinal = 3
};
/**
 * @brief  Structure of subsample information for drm
 */
struct DrmbEsSubSampleInfo {
  explicit DrmbEsSubSampleInfo(const uint32_t _bytes_of_clear_data,
                               const uint32_t _bytes_of_encrypted_data)
      : bytes_of_clear_data(_bytes_of_clear_data),
        bytes_of_encrypted_data(_bytes_of_encrypted_data) {}
  uint32_t bytes_of_clear_data;
  uint32_t bytes_of_encrypted_data;
};
/**
 * @brief  Structure of fragmented mp4 data for drm
 */
struct DrmbEsFragmentedMp4Data {
  std::vector<DrmbEsSubSampleInfo> sub_sample_info_vector;
};
/**
 * @brief  Structure of encrypted information for es playback
 */
struct EsPlayerEncryptedInfo {
  int32_t handle = 0;

  DrmbEsCipherAlgorithm algorithm = DrmbEsCipherAlgorithm::kUnknown;
  DrmbEsMediaFormat format = DrmbEsMediaFormat::kNone;
  DrmbEsCipherPhase phase = DrmbEsCipherPhase::kNone;

  std::vector<unsigned char> kid;
  std::vector<unsigned char> initialization_vector;

  MetaData sub_data = nullptr;
  std::array<int, 15> split_offsets;

  bool use_out_buffer = false;
  bool use_pattern = false;

  uint32_t crypt_byte_block = 0;
  uint32_t skip_byte_block = 0;
};

}  // namespace drm

}  // namespace plusplayer

#endif  // __PLUSPLAYER_EXTERNAL_DRM_H__
