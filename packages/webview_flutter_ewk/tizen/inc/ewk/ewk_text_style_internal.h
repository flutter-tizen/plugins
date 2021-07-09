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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG ELECTRONICS. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ewk_text_style_internal_h
#define ewk_text_style_internal_h

#include <Eina.h>
#include <Evas.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EWK_TEXT_STYLE_STATE_FALSE,
  EWK_TEXT_STYLE_STATE_TRUE,
  EWK_TEXT_STYLE_STATE_MIXED
} Ewk_Text_Style_State;

typedef struct _Ewk_Text_Style Ewk_Text_Style;

EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_underline_get(Ewk_Text_Style* text_style);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_italic_get(Ewk_Text_Style* text_style);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_bold_get(Ewk_Text_Style* text_style);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_ordered_list_get(Ewk_Text_Style* text_style);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_unordered_list_get(Ewk_Text_Style* text_style);
EINA_DEPRECATED EXPORT_API Eina_Bool ewk_text_style_position_get(
    Ewk_Text_Style* text_style, Evas_Point* start_point, Evas_Point* end_point);
EINA_DEPRECATED EXPORT_API Eina_Bool ewk_text_style_bg_color_get(
    Ewk_Text_Style* textStyle, int* r, int* g, int* b, int* a);
EINA_DEPRECATED EXPORT_API Eina_Bool ewk_text_style_color_get(
    Ewk_Text_Style* textStyle, int* r, int* g, int* b, int* a);
EINA_DEPRECATED EXPORT_API const char* ewk_text_style_font_size_get(
    Ewk_Text_Style* textStyle);
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_text_style_has_composition_get(Ewk_Text_Style* textStyle);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_align_center_get(Ewk_Text_Style* textStyle);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_align_left_get(Ewk_Text_Style* textStyle);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_align_right_get(Ewk_Text_Style* textStyle);
EINA_DEPRECATED EXPORT_API Ewk_Text_Style_State
ewk_text_style_align_full_get(Ewk_Text_Style* textStyle);

#ifdef __cplusplus
}
#endif

#endif  // ewk_text_style_internal_h
