/*
 * Copyright (C) 2017 Samsung Electronics. All rights reserved.
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

#ifndef ewk_manifest_internal_h
#define ewk_manifest_internal_h

#include <tizen.h>

#include "ewk_manifest.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ewk_View_Request_Manifest Ewk_View_Request_Manifest;

/**
 * @brief Get the spp_sender_id from the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest object to get manifest information.
 *
 * @return @c string of spp_sender_id.
 * The value is only valid until related
 * Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API const char* ewk_manifest_push_sender_id_get(
    Ewk_View_Request_Manifest* manifest);

#ifdef __cplusplus
}
#endif
#endif  // ewk_manifest_internal_h
