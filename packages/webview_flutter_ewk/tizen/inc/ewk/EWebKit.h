/*
 * Copyright (C) 2016 Samsung Electronics.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * @file    EWebKit.h
 * @brief   This file contains the header files that are required by
 *          Chromium-efl.
 *
 * It includes all the header files that are exported to public API.
 */

#ifndef EWebKit_h
#define EWebKit_h

#include "ewk_autofill_profile.h"
#include "ewk_back_forward_list.h"
#include "ewk_back_forward_list_item.h"
#include "ewk_context.h"
#include "ewk_context_menu.h"
#include "ewk_cookie_manager.h"
#include "ewk_error.h"
#include "ewk_geolocation.h"
#include "ewk_intercept_request.h"
#include "ewk_main.h"
#include "ewk_manifest.h"
#include "ewk_policy_decision.h"
#include "ewk_security_origin.h"
#include "ewk_settings.h"
#include "ewk_view.h"

/**
 * @ingroup  CAPI_WEB_FRAMEWORK
 * @defgroup WEBVIEW WebView
 * @brief    The WebView API provides functions to display web pages and control
 *           web pages.
 *
 * @section  WEBVIEW_HEADER Required Header
 *   \#include <EWebKit.h>
 *
 * @section  WEBVIEW_OVERVIEW Overview
 * The WebView API provides functions to display web pages and control
 * web pages. It is based on the Chromium engine, which is one of the most
 * popular layout engines to render web pages.
 *
 * @section  WEBVIEW_SMART_OBJECT Smart object
 * It is Chromium main smart object. This object provides view related APIs of
 * Chromium to EFL object.\n
 * The following signals (see evas_object_smart_callback_add()) are emitted:
 * <table>
 *     <tr>
 *         <th> Signals </th>
 *         <th> Type </th>
 *         <th> Description </th>
 *     </tr>
 *     <tr>
 *         <td> close,window </td>
 *         <td> void </td>
 *         <td> Window is closed </td>
 *     </tr>
 *     <tr>
 *         <td> contextmenu,customize </td>
 *         <td> Ewk_Context_Menu* </td>
 *         <td> Requested context menu items can be customized by app side </td>
 *     </tr>
 *     <tr>
 *         <td> contextmenu,selected </td>
 *         <td> Ewk_Context_Menu_Item* </td>
 *         <td> A context menu item is selected </td>
 *     </tr>
 *     <tr>
 *         <td> create,window </td>
 *         <td> Evas_Object** </td>
 *         <td> A new window is created </td>
 *     </tr>
 *     <tr>
 *         <td> fullscreen,enterfullscreen </td>
 *         <td> bool* </td>
 *         <td> Reports to enter fullscreen </td>
 *     </tr>
 *     <tr>
 *         <td> fullscreen,exitfullscreen </td>
 *         <td> void </td>
 *         <td> Reports to exit fullscreen </td>
 *     </tr>
 *     <tr>
 *         <td> load,committed </td>
 *         <td> void </td>
 *         <td> Reports load committed </td>
 *     </tr>
 *     <tr>
 *         <td> load,error </td>
 *         <td> Ewk_Error* </td>
 *         <td> Reports load error </td>
 *     </tr>
 *     <tr>
 *         <td> load,finished </td>
 *         <td> void </td>
 *         <td> Reports load finished </td>
 *     </tr>
 *     <tr>
 *         <td> load,progress </td>
 *         <td> double* </td>
 *         <td> Load progress has changed </td>
 *     </tr>
 *     <tr>
 *         <td> load,started </td>
 *         <td> void </td>
 *         <td> Reports load started </td>
 *     </tr>
 *     <tr>
 *         <td> geolocation,permission,request </td>
 *         <td> Ewk_Geolocation_Permission_Request* </td>
 *         <td> Requests geolocation permission </td>
 *     </tr>
 *     <tr>
 *         <td> policy,navigation,decide </td>
 *         <td> Ewk_Policy_Decision* </td>
 *         <td> A navigation policy decision should be taken </td>
 *     </tr>
 *     <tr>
 *         <td> policy,newwindow,decide </td>
 *         <td> Ewk_Policy_Decision* </td>
 *         <td> A new window policy decision should be taken </td>
 *     </tr>
 *     <tr>
 *         <td> policy,response,decide </td>
 *         <td> Ewk_Policy_Decision* </td>
 *         <td> A response policy decision should be taken </td>
 *     </tr>
 *     <tr>
 *         <td> text,found </td>
 *         <td> unsigned* </td>
 *         <td> The requested text was found and it gives the number of matches
 *         </td>
 *     </tr>
 *     <tr>
 *         <td> title,changed </td>
 *         <td> const char* </td>
 *         <td> Title of the main frame was changed </td>
 *     </tr>
 *     <tr>
 *         <td> url,changed </td>
 *         <td> const char* </td>
 *         <td> Url of the main frame was changed </td>
 *     </tr>
 *     <tr>
 *         <td> did,not,allow,script </td>
 *         <td> void </td>
 *         <td> Javascript did not allowed </td>
 *     </tr>
 *
 * </table>
 */

#endif  // EWebKit_h
