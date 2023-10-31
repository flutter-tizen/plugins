/**
 * @file
 * @interfacetype  module
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        1.0
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

#ifndef __PLUSPLAYER_TRACK_H__
#define __PLUSPLAYER_TRACK_H__

#include <boost/any.hpp>
#include <boost/core/noncopyable.hpp>
#include <cstdint>
#include <limits>
#include <list>
#include <memory>
#include <string>

namespace plusplayer {

const int kInvalidTrackIndex = -1;

enum TrackType {
  kTrackTypeAudio = 0,
  kTrackTypeVideo,
  kTrackTypeSubtitle,
  kTrackTypeMax
};

struct Track {
  int index = kInvalidTrackIndex;
  int id = 0;
  std::string mimetype;
  std::string streamtype;
  std::string container_type;
  TrackType type = kTrackTypeMax;
  std::shared_ptr<char> codec_data;
  unsigned int codec_tag = 0;
  int codec_data_len = 0;
  int width = 0;
  int height = 0;
  int maxwidth = 0;
  int maxheight = 0;
  int framerate_num = 0;
  int framerate_den = 0;
  int sample_rate = 0;
  int sample_format = 0;
  int channels = 0;
  int version = 0;
  int layer = 0;
  int bits_per_sample = 0;
  int block_align = 0;
  int bitrate = 0;
  int endianness = 1234;  // little endian : 1234 others big endian
  bool is_signed = false;
  bool active = false;
  bool use_swdecoder = false;
  std::string language_code;
  std::string subtitle_format;
  Track(){};
  Track(int _index, int _id, std::string _mimetype, std::string _streamtype,
        std::string _container_type, TrackType _type,
        std::shared_ptr<char> _codec_data, unsigned int _codec_tag,
        int _codec_data_len, int _width, int _height, int _maxwidth,
        int _maxheight, int _framerate_num, int _framerate_den,
        int _sample_rate, int _sample_format, int _channels, int _version,
        int _layer, int _bits_per_sample, int _block_align, int _bitrate,
        int _endianness, bool _is_signed, bool _active, bool _use_swdecoder,
        std::string _language_code, std::string _subtitle_format)
      : index(_index),
        id(_id),
        mimetype(_mimetype),
        streamtype(_streamtype),
        container_type(_container_type),
        type(_type),
        codec_data(_codec_data),
        codec_tag(_codec_tag),
        codec_data_len(_codec_data_len),
        width(_width),
        height(_height),
        maxwidth(_maxwidth),
        maxheight(_maxheight),
        framerate_num(_framerate_num),
        framerate_den(_framerate_den),
        sample_rate(_sample_rate),
        sample_format(_sample_format),
        channels(_channels),
        version(_version),
        layer(_layer),
        bits_per_sample(_bits_per_sample),
        block_align(_block_align),
        bitrate(_bitrate),
        endianness(_endianness),
        is_signed(_is_signed),
        active(_active),
        use_swdecoder(_use_swdecoder),
        language_code(_language_code),
        subtitle_format(_subtitle_format){};
};

typedef struct _CaptionTracks {
  int index = kInvalidTrackIndex;
  std::string name;
  std::string language;
  std::string inStreamId;
} CaptionTracks;

enum SubtitleAttrType {
  kSubAttrRegionXPos = 0,            // float type
  kSubAttrRegionYPos,                // float type
  kSubAttrRegionWidth,               // float type
  kSubAttrRegionHeight,              // float type
  kSubAttrWindowXPadding,            // float type
  kSubAttrWindowYPadding,            // float type
  kSubAttrWindowLeftMargin,          // int type
  kSubAttrWindowRightMargin,         // int type
  kSubAttrWindowTopMargin,           // int type
  kSubAttrWindowBottomMargin,        // int type
  kSubAttrWindowBgColor,             // int type
  kSubAttrWindowOpacity,             // float type
  kSubAttrWindowShowBg,              // how to show window background, uint type
  kSubAttrFontFamily,                // char* type
  kSubAttrFontSize,                  // float type
  kSubAttrFontWeight,                // int type
  kSubAttrFontStyle,                 // int type
  kSubAttrFontColor,                 // int type
  kSubAttrFontBgColor,               // int type
  kSubAttrFontOpacity,               // float type
  kSubAttrFontBgOpacity,             // float type
  kSubAttrFontTextOutlineColor,      // int type
  kSubAttrFontTextOutlineThickness,  // int type
  kSubAttrFontTextOutlineBlurRadius,  // int type
  kSubAttrFontVerticalAlign,          // int type
  kSubAttrFontHorizontalAlign,        // int type
  kSubAttrRawSubtitle,                // char* type
  kSubAttrWebvttCueLine,              // float type
  kSubAttrWebvttCueLineNum,           // int type
  kSubAttrWebvttCueLineAlign,         // int type
  kSubAttrWebvttCueAlign,             // int type
  kSubAttrWebvttCueSize,              // float type
  kSubAttrWebvttCuePosition,          // float type
  kSubAttrWebvttCuePositionAlign,     // int type
  kSubAttrWebvttCueVertical,          // int type
  kSubAttrTimestamp,
  kSubAttrExtsubIndex,  // File index of external subtitle
  kSubAttrTypeNone
};

/**
 * @brief Enumeration for  player supported subtitle types
 */
enum class SubtitleType { kText, kPicture, kTTMLRender, kInvalid };

struct SubtitleAttr {
  explicit SubtitleAttr(const SubtitleAttrType _type,
                        const uint32_t _start_time, const uint32_t _stop_time,
                        const boost::any _value, const int _extsub_index)
      : type(_type),
        start_time(_start_time),
        stop_time(_stop_time),
        value(_value),
        extsub_index(_extsub_index) {}
  const SubtitleAttrType type = kSubAttrTypeNone;
  const uint32_t start_time = std::numeric_limits<uint32_t>::max();
  const uint32_t stop_time = std::numeric_limits<uint32_t>::max();
  const boost::any value;
  const int extsub_index = -1;
};
using SubtitleAttrList = std::list<SubtitleAttr>;
using SubtitleAttrListPtr = std::unique_ptr<SubtitleAttrList>;
struct Rational {
  int num = 0;  // the numerator value
  int den = 0;  // the denominator value
};
}  // namespace plusplayer

#endif  // __PLUSPLAYER_TRACK_H__
