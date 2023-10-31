/**
 * @file           display.h
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

#ifndef __PLUSPLAYER_TYPES_DISPLAY_H__
#define __PLUSPLAYER_TYPES_DISPLAY_H__

namespace plusplayer {

enum class DisplayType { kNone, kOverlay, kEvas, kMixer, kOverlaySyncUI };

enum class DisplayMode {
  kLetterBox,
  kOriginSize,
  kFullScreen,
  kCroppedFull,
  kOriginOrLetter,
  kDstRoi,
  kAutoAspectRatio,
  kMax
};

enum class DisplayRotation { kNone, kRotate90, kRotate180, kRotate270 };

struct Geometry {
  int x = 0, y = 0;
  int w = 1920, h = 1080;
};

struct CropArea {
  double scale_x = 0.0;
  double scale_y = 0.0;
  double scale_w = 1.0;
  double scale_h = 1.0;
};

struct RenderRect {
  int x = 0, y = 0;
  int w = 1920, h = 1080;
};

enum class VisibleStatus { kHide, kVisible };

struct DisplayInfo {
  Geometry geometry;
  CropArea croparea;
  VisibleStatus visible_status = VisibleStatus::kVisible;
};

enum class StillMode { kNone, kOff, kOn };

struct DisplayObject {
  DisplayType type_;
  int surface_id_;
  DisplayMode mode_;
  Geometry geometry_;
  void* obj_;
  bool is_obj_ = false;
};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_TYPES_DISPLAY_H__
