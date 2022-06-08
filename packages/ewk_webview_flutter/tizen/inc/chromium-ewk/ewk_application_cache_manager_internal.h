/*
 * Copyright (C) 2014-2016 Samsung Electronics. All rights reserved.
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

/**
 * @file    ewk_application_cache_manager_internal.h
 * @brief   Describes the Ewk Application Cache Manager API.
 */

#ifndef ewk_application_cache_manager_internal_h
#define ewk_application_cache_manager_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Creates a type name for Ewk_Application_Cache_Manager */
typedef struct EwkApplicationCacheManager Ewk_Application_Cache_Manager;

/**
 * Deletes all web application caches.
 *
 * @param manager Ewk_Application_Cache_Manager object
 *
 * @return @c EINA_TRUE on successful request or @c EINA FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_application_cache_manager_clear(Ewk_Application_Cache_Manager *manager);

#ifdef __cplusplus
}
#endif

#endif  // ewk_application_cache_manager_internal_h
