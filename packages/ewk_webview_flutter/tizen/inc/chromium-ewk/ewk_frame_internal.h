/*
 * Copyright (C) 2012-2016 Samsung Electronics. All rights reserved.
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

#ifndef ewk_frame_internal_h
#define ewk_frame_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ewk_Frame _Ewk_Frame;

typedef _Ewk_Frame* Ewk_Frame_Ref;

/**
 * Callback for ewk_frame_source_get
 *
 * @param frame frame object to get the frame source
 * @param source frame source on success, or NULL on failure
 * @param user_data user data
 */
typedef void (*Ewk_Frame_Source_Get_Callback)(Ewk_Frame_Ref frame,
                                              const char* source,
                                              void* user_data);

/**
 * Gets whether a MIME type can be displayed in the frame.
 *
 * @param frame frame object
 * @param mime_type a mime type
 *
 * @return @c EINA_TRUE if the MIME type can be displayed or @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool ewk_frame_can_show_mime_type(Ewk_Frame_Ref frame,
                                                  char* mime_type);

/**
 * Gets whether the frame is main frame.
 *
 * @param frame frame object
 *
 * @return @c EINA_TRUE if the frame is main frame or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_frame_is_main_frame(Ewk_Frame_Ref frame);

#ifdef __cplusplus
}
#endif
#endif  // ewk_frame_internal_h
