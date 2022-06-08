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

/**
 * @file    EWebKit_product.h
 * @brief   Contains the header files that are required by Chromium-EFL.
 *
 * It includes the all header files that are exported to product API.
 */

#ifndef EWebKit_product_h
#define EWebKit_product_h

#include "ewk_autofill_profile_product.h"
#include "ewk_context_menu_product.h"
#include "ewk_context_product.h"
#include "ewk_cookie_manager_product.h"
#include "ewk_file_chooser_request_product.h"
#include "ewk_form_repost_decision_product.h"
#include "ewk_highcontrast_product.h"
#include "ewk_media_downloadable_font_info_product.h"
#include "ewk_media_parental_rating_info_product.h"
#include "ewk_media_playback_info_product.h"
#include "ewk_media_subtitle_info_product.h"
#include "ewk_settings_product.h"
#include "ewk_value_product.h"
#include "ewk_view_product.h"

/**
 * @ingroup  CAPI_WEB_FRAMEWORK
 * @brief    The WebView API product specific functions.
 *
 * @section  WEBVIEW_HEADER Required Header
 *   \#include <EWebKit_product.h>
 *
 * @section  WEBVIEW_OVERVIEW Overview
 * Product specific API
 *
 * @section  WEBVIEW_SMART_OBJECT Smart object
 * The following signals (see evas_object_smart_callback_add()) are emitted:
 * <table>
 *     <tr>
 *         <th> Signals </th>
 *         <th> Type </th>
 *         <th> Description </th>
 *     </tr>
 *     <tr>
 *         <td> link,hover,over </td>
 *         <td> char* </td>
 *         <td> Mouse cursor hovers over a link </td>
 *     </tr>
 *     <tr>
 *         <td> link,hover,out </td>
 *         <td> char* </td>
 *         <td> Mouse cursor is moved away from a link </td>
 *     </tr>
 * </table>
 */

#endif  // EWebKit_product_h
