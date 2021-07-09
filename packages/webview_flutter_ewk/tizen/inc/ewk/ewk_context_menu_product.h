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

/**
 * @file    ewk_context_menu_product.h
 * @brief   Describes the context menu product API.
 */

#ifndef ewk_context_menu_product_h
#define ewk_context_menu_product_h

#include "ewk_context_menu_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The enum of Ewk_Context_Menu_Item_Tag for additional item tags
 * @since_tizen 2.4.0.2
 */

enum _Ewk_Context_Menu_New_Item_Tag {
  EWK_CONTEXT_MENU_ITEM_NEW_TAGS = 9000,
  EWK_CONTEXT_MENU_ITEM_TAG_QUICKMEMO,
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_BACKGROUND
};

/**
 * Gets a title of the item.
 *
 * @param item the item to get the title
 * @return a title of the item on success, or @c NULL on failure
 *
 * @see ewk_context_menu_item_title_set
 */
EXPORT_API const char *ewk_context_menu_item_title_get(
    const Ewk_Context_Menu_Item *item);

/**
 * Queries if the item is toggled.
 *
 * @param item the item to query if the item is toggled
 * @return @c EINA_TRUE if the item is toggled or @c EINA_FALSE if not or on
 * failure
 */
EXPORT_API Eina_Bool
ewk_context_menu_item_checked_get(const Ewk_Context_Menu_Item *item);

/**
 * Gets the submenu for the item.
 *
 * @param item item to get the submenu
 *
 * @return the pointer to submenu on success or @c NULL on failure
 */
EXPORT_API Ewk_Context_Menu *ewk_context_menu_item_submenu_get(
    const Ewk_Context_Menu_Item *item);

/**
 * Gets the list of items.
 *
 * @param o the context menu to get list of the items
 * @return the list of the items on success or @c NULL on failure
 */
EXPORT_API const Eina_List *ewk_context_menu_items_get(
    const Ewk_Context_Menu *o);

/**
 * Gets the parent menu for the item.
 *
 * @param o item to get the parent
 *
 * @return the pointer to parent menu on success or @c NULL on failure
 */
EXPORT_API Ewk_Context_Menu *ewk_context_menu_item_parent_menu_get(
    const Ewk_Context_Menu_Item *o);

/**
 * Selects the item from the context menu.
 *
 * @param menu the context menu
 * @param item the item is selected
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_menu_item_select(Ewk_Context_Menu *menu,
                                                  Ewk_Context_Menu_Item *item);

/**
 * Hides the context menu.
 *
 * @param menu the context menu to hide
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_menu_hide(Ewk_Context_Menu *menu);

#if defined(OS_TIZEN_TV)
/**
 * Gets the x_position of context menu.
 *
 * @param menu the context menu to get x_position
 * @return the x_position of context menu on success or @c 0 on failure
 */
EXPORT_API int ewk_context_menu_pos_x_get(Ewk_Context_Menu *menu);

/**
 * Gets the y_position of context menu.
 *
 * @param menu the context menu to get y_position
 * @return the y_position of context menu on success or @c 0 on failure
 */
EXPORT_API int ewk_context_menu_pos_y_get(Ewk_Context_Menu *menu);
#endif  // OS_TIZEN_TV

#ifdef __cplusplus
}
#endif

#endif  // ewk_context_menu_product_h
