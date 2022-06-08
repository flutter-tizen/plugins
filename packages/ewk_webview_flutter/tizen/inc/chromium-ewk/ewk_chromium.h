// Copyright 2016 Samsung Electronics. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ewk_chromium_efl_h
#define ewk_chromium_efl_h

#include "EWebKit.h"
#include "EWebKit_internal.h"
#include "EWebKit_product.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This file is no longer supported and all its function are
 * deprecated now. It will be removed soon.
 */

/**
 * Append specified flag to chromium command line. This method should
 * be called after @ewk_set_arguments, but before @ewk_init. Calling it
 * after @ewk_init won't have any affects as in most cases chromium has
 * already processed most command line parameters.
 *
 * @deprecated Deprecated since Tizen 3.0.
 * This has been deprecated due to it was exposed on the internal needs.
 * Secondly, this function can not add command line switches defined as pair
 * (flag=value).
 * If there is a need to pass command line param it can be achieved
 * by ewk_set_arguments.
 *
 * @param flag Chromium command line flag
 *
 * @returns EINA_TRUE in case the call succeeded, EINA_FALSE in case it
 *     was called before ewk_set_arguments, or the flag argument was NULL.
 */
EXPORT_API Eina_Bool ewk_chromium_append_command_line_flag(const char* flag);

#ifdef __cplusplus
}
#endif
#endif
