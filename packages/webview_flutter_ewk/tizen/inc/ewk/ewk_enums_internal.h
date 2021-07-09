/*
 * Copyright (C) 2013-2016 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG ELECTRONICS. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ewk_enums_internal_h
#define ewk_enums_internal_h

#ifdef __cplusplus
extern "C" {
#endif

// #if OS(TIZEN)
/// Represents types of gesture.
enum _Ewk_Gesture_Type {
  EWK_GESTURE_TAP,
  EWK_GESTURE_LONG_PRESS,
  EWK_GESTURE_PAN,
  EWK_GESTURE_FLICK,
  EWK_GESTURE_PINCH
};
/// Creates a type name for @a _Ewk_Gesture_Type.
typedef enum _Ewk_Gesture_Type Ewk_Gesture_Type;
// #endif // #if OS(TIZEN)

//#if ENABLE(TIZEN_ORIENTATION_EVENTS)
enum _Ewk_Screen_Orientation {
  EWK_SCREEN_ORIENTATION_PORTRAIT_PRIMARY = 1,
  EWK_SCREEN_ORIENTATION_LANDSCAPE_PRIMARY = 1 << 1,
  EWK_SCREEN_ORIENTATION_PORTRAIT_SECONDARY = 1 << 2,
  EWK_SCREEN_ORIENTATION_LANDSCAPE_SECONDARY = 1 << 3
};
typedef enum _Ewk_Screen_Orientation Ewk_Screen_Orientation;
//#endif

#ifdef __cplusplus
}
#endif

#endif  // ewk_enums_internal_h
