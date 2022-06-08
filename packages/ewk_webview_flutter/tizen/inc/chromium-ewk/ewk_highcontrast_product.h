/*
 * Copyright (C) 2016 Samsung Electronics. All rights reserved.
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

#ifndef ewk_highcontrast_h
#define ewk_highcontrast_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set enable/disable highcontrast.
 *
 * If highcontrast enabled, highcontrast controller listen system highcontrast
 * configuration.
 *
 * @param enabled enable/disable set for high contrast
 */
EXPORT_API void ewk_highcontrast_enabled_set(Eina_Bool enabled);

/**
 * Returns current hightcontrast settings.
 *
 * @return EINA_TRUE if highcontrast enabled
 */
EXPORT_API Eina_Bool ewk_highcontrast_enabled_get();

/**
 * Add highcontrast forbidden URL.
 *
 * If the current page's url contains this url, it should not use highcontrast
 filter
 *
 * @param url URL that should not apply hightcontrast filter
 * @return @c EINA_TRUE if successfully added or @c EINA_FALSE otherwise
           Note that add a reduplicative url will return EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_highcontrast_forbidden_url_add(const char* url);

/**
 * Remove the specific highcontrast forbidden URL.
 *
 * @param url URL that already in the forbidden url list
 * @return @c EINA_TRUE if successfully removed or @c EINA_FALSE otherwise
           Note that if the url is not exist in the list it will return
 EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_highcontrast_forbidden_url_remove(const char* url);

#ifdef __cplusplus
}
#endif

#endif  // ewk_highcontrast_h
