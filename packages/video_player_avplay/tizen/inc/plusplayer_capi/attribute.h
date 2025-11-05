/**
 * @file
 * @brief          Subtitle Attribute Enums and structures.
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style Subtitle Attribute related enums
 * and structures.
 * @see            Subtitle Attribute enum conversion and structures.
 *
 * Copyright (c) 2025 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_ATTRIBUTE_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_ATTRIBUTE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Enumeration for supported subtitle attributes
 */
typedef enum {
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_REGION_XPOS,          /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_REGION_YPOS,          /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_REGION_WIDTH,         /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_REGION_HEIGHT,        /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_XPADDING,      /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_YPADDING,      /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_LEFT_MARGIN,   /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_RIGHT_MARGIN,  /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_TOP_MARGIN,    /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_BOTTOM_MARGIN, /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_BG_COLOR,      /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_OPACITY,       /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WINDOW_SHOW_BG,  /**< how to show window background,
                                               uint type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_FAMILY,     /**< char* type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_SIZE,       /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_WEIGHT,     /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_STYLE,      /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_COLOR,      /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_BG_COLOR,   /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_OPACITY,    /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_BG_OPACITY, /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_TEXT_OUTLINE_COLOR,       /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_TEXT_OUTLINE_THICKNESS,   /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_TEXT_OUTLINE_BLUR_RADIUS, /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_VERTICAL_ALIGN,           /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_FONT_HORIZONTAL_ALIGN,         /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_RAW_SUBTITLE,                  /**< char* type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_LINE,               /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_LINE_NUM,           /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_LINE_ALIGN,         /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_ALIGN,              /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_SIZE,               /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_POSITION,           /**< float type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_POSITION_ALIGN,     /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_WEBVTT_CUE_VERTICAL,           /**< int type */
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_TIMESTAMP,
  PLUSPLAYER_SUBTITLE_ATTR_TYPE_EXTSUB_INDEX /**< File index of external subtitle */
} plusplayer_subtitle_attr_type_e;

/**
 * @brief Enumeration for  player supported subtitle types
 */
typedef enum {
  PLUSPLAYER_SUBTITLE_TYPE_TEXT,    /**< subtitle type text */
  PLUSPLAYER_SUBTITLE_TYPE_PICTURE, /**< subtitle type picture */
  PLUSPLAYER_SUBTITLE_TYPE_TTML,    /**< subtitle type ttml */
} plusplayer_subtitle_type_e;

/**
 * @brief Enumeration for  player supported subtitle attribute data types
 */
typedef enum {
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_BOOL,  /**< subtitle attribute data type bool */
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_FLOAT, /**< subtitle attribute data type float
                                        */
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_DOUBLE, /**< subtitle attribute data type double
                                         */
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_STRING, /**< subtitle attribute data type string
                                         */
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_INT,    /**< subtitle attribute data type int */
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_UINT,  /**< subtitle attribute data type uint */
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_INT64, /**< subtitle attribute data type int64
                                        */
  PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_UINT64 /**< subtitle attribute data type uint64
                                        */
} plusplayer_subtitle_attr_data_type_e;

/**
 * @brief structure definition of subtitle attribute
 */
typedef struct {
  /**
   * @brief   Subtitle attribute.
   */
  plusplayer_subtitle_attr_type_e attr;

  /**
   * @brief   Subtitle attribute data type.
   */
  plusplayer_subtitle_attr_data_type_e dtype;

  /**
   * @brief   Start time.
   */
  uint32_t start_time;

  /**
   * @brief   Stop time.
   */
  uint32_t stop_time;

  /**
   * @brief   Subtitle attribute value. It can be float, int32, char*,
   *                unsigned int.
   */
  union {
    /**
     * @brief   Floating type subtitle attribute value.
     */
    float float_value;

    /**
     * @brief   32bit integer type subtitle atribute value.
     */
    int32_t int32_value;

    /**
     * @brief   String type subtitle atribute value.
     */
    const char* str_value;

    /**
     * @brief   Unsigned 32bit integer type subtitle attribute value.
     */
    uint32_t uint32_value;

    /**
     * @brief   Unsigned 64bit integer type subtitle attribute value.
     */
    uint64_t uint64_value;

    /**
     * @brief   64bit integer type subtitle attribute value.
     */
    int64_t int64_value;

    /**
     * @brief   Double type subtitle attribute value.
     */
    double double_value;
  } value;

  /**
   * @brief   Extra subtitle attribute index.
   */
  int extsub_index;
} plusplayer_subtitle_attr_s;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_ATTRIBUTE_H__