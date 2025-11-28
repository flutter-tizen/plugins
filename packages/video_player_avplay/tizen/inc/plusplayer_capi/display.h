/**
 * @file
 * @brief          Display related enums
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style display releted data structures
 *                 and enums.
 * @see            The display related enum values and data structures will be
 *                 converted by this managed C version types to avoid binary
 *                 compatibility.
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

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_DISPLAY_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_DISPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerations for controlling the still mode activation
 */
typedef enum {
  PLUSPLAYER_STILL_MODE_NONE, /**< Still mode is not set */
  PLUSPLAYER_STILL_MODE_OFF,  /**< Still mode is deactivated */
  PLUSPLAYER_STILL_MODE_ON    /**< Still mode is activated */
} plusplayer_still_mode_e;

/**
 * @brief  Enumerations for the display mode
 */
typedef enum {
  PLUSPLAYER_DISPLAY_MODE_LETTER_BOX,           /**< Letterboxing mode */
  PLUSPLAYER_DISPLAY_MODE_ORIGIN_SIZE,          /**< Original size */
  PLUSPLAYER_DISPLAY_MODE_FULL_SCREEN,          /**< Full screen */
  PLUSPLAYER_DISPLAY_MODE_CROPPED_FULL,         /**< Cropped full screen */
  PLUSPLAYER_DISPLAY_MODE_ORIGIN_OR_LETTER,     /**< Origin or letterbox */
  PLUSPLAYER_DISPLAY_MODE_DST_ROI,              /**< Destination ROI */
  PLUSPLAYER_DISPLAY_MODE_AUTO_ASPECT_RATIO,    /**< Auto aspect ratio */
  PLUSPLAYER_DISPLAY_MODE_ROI_AUTO_ASPECT_RATIO /**< ROI auto aspect ratio */
} plusplayer_display_mode_e;

/**
 * @brief Enumeration of display types
 */
typedef enum {
  PLUSPLAYER_DISPLAY_TYPE_NONE,         /**< No display type specified */
  PLUSPLAYER_DISPLAY_TYPE_OVERLAY,      /**< Overlay display type */
  PLUSPLAYER_DISPLAY_TYPE_EVAS,         /**< EVAS-based display */
  PLUSPLAYER_DISPLAY_TYPE_MIXER,        /**< Mixer display type */
  PLUSPLAYER_DISPLAY_TYPE_OVERLAYSYNCUI /**< Overlay with synchronized UI */
} plusplayer_display_type_e;

/**
 * @brief Enumeration for display rotation types
 */
typedef enum {
  PLUSPLAYER_DISPLAY_ROTATION_TYPE_NONE, /**< No rotation applied */
  PLUSPLAYER_DISPLAY_ROTATION_TYPE_90,   /**< Rotate 90 degrees clockwise */
  PLUSPLAYER_DISPLAY_ROTATION_TYPE_180,  /**< Rotate 180 degrees */
  PLUSPLAYER_DISPLAY_ROTATION_TYPE_270   /**< Rotate 270 degrees clockwise */
} plusplayer_display_rotation_type_e;

/**
 * @brief  struct for geometry
 */
typedef struct {
  /**
   * @brief  start X position of Display window. [Default = 0]
   */
  int x;
  /**
   * @brief  start Y position of Display window. [Default = 0]
   */
  int y;
  /**
   * @brief  Width of Display window. [Default = 1920]
   */
  int width;
  /**
   * @brief  Height of Display window. [Default = 1080]
   */
  int height;
} plusplayer_geometry_s;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_DISPLAY_H__
