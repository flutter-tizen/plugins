/*
 * Copyright (C) 2012 Intel Corporation.
 * Copyright (C) 2014-2016 Samsung Electronics.
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
 * @file    ewk_back_forward_list_item.h
 * @brief   This file describes the Ewk Back Forward List Item API.
 */

#ifndef ewk_back_forward_list_item_h
#define ewk_back_forward_list_item_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @brief The structure type that creates a type name for
 *        Ewk_Back_Forward_List_Item.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef struct _Ewk_Back_Forward_List_Item Ewk_Back_Forward_List_Item;

/**
 * @brief Increases the reference count of the given object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] item The back-forward list item instance to increase the reference
 *            count
 *
 * @return A pointer to the object on success,\n
 *         otherwise @c NULL
 */
EXPORT_API Ewk_Back_Forward_List_Item* ewk_back_forward_list_item_ref(
    Ewk_Back_Forward_List_Item* item);

/**
 * @brief Decreases the reference count of the given object,
 *        possibly freeing it.
 *
 * @details When the reference count reaches @c 0, the item is freed.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] item The back-forward list item instance to decrease the reference
 *            count
 */
EXPORT_API void ewk_back_forward_list_item_unref(
    Ewk_Back_Forward_List_Item* item);

/**
 * @brief Returns the URL of the item.
 *
 * @details The returned URL may differ from the original URL (For example,
 *          if the page is redirected).
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] item The back-forward list item instance
 *
 * @return The URL of the @a item,\n
 *         otherwise @c NULL in case of an error\n
 *         This pointer is guaranteed to be eina_stringshare,\n
 *         so whenever possible save yourself some CPU cycles and\n
 *         use eina_stringshare_ref() instead of eina_stringshare_add() or
 *         strdup()
 *
 * @see ewk_back_forward_list_item_original_url_get()
 */
EXPORT_API const char* ewk_back_forward_list_item_url_get(
    const Ewk_Back_Forward_List_Item* item);

/**
 * @brief Returns the title of the item.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] item The back-forward list item instance
 *
 * @return The title of the @a item,\n
 *         otherwise @c NULL in case of an error\n
 *         This pointer is guaranteed to be eina_stringshare,\n
 *         so whenever possible save yourself some CPU cycles and\n
 *         use eina_stringshare_ref() instead of eina_stringshare_add() or
 *         strdup()
 */
EXPORT_API const char* ewk_back_forward_list_item_title_get(
    const Ewk_Back_Forward_List_Item* item);

/**
 * @brief Returns the original URL of the item.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] item The back-forward list item instance
 *
 * @return The original URL of the @a item,\n
 *         otherwise @c NULL in case of an error\n
 *         This pointer is guaranteed to be eina_stringshare,\n
 *         so whenever possible save yourself some CPU cycles and\n
 *         use eina_stringshare_ref() instead of eina_stringshare_add() or
 *         strdup()
 *
 * @see ewk_back_forward_list_item_url_get()
 */
EXPORT_API const char* ewk_back_forward_list_item_original_url_get(
    const Ewk_Back_Forward_List_Item* item);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  // ewk_back_forward_list_item_h
