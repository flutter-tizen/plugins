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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
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
 * @file    ewk_context_menu_internal.h
 * @brief   Describes the context menu API.
 */

#ifndef ewk_context_menu_internal_h
#define ewk_context_menu_internal_h

#include "ewk_context_menu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \enum    _Ewk_Context_Menu_Item_Type
 * @brief   Enumeration that defines the types of the items for the context
 * menu.
 */
enum _Ewk_Context_Menu_Item_Type {
  EWK_CONTEXT_MENU_ITEM_TYPE_ACTION,
  EWK_CONTEXT_MENU_ITEM_TYPE_CHECKABLE_ACTION,
  EWK_CONTEXT_MENU_ITEM_TYPE_SEPARATOR,
  EWK_CONTEXT_MENU_ITEM_TYPE_SUBMENU
};

/**
 * @brief Creates a type name for _Ewk_Context_Menu_Item_Type
 */
typedef enum _Ewk_Context_Menu_Item_Type Ewk_Context_Menu_Item_Type;

/**
 * Returns the type of context menu item.
 *
 * @param item The context menu item object
 *
 * @return The type of context menu item
 */
EXPORT_API Ewk_Context_Menu_Item_Type
ewk_context_menu_item_type_get(Ewk_Context_Menu_Item* item);

/**
 * Returns the item is enabled.
 *
 * @param item The context menu item object to get enabled state
 *
 * @return EINA_TRUE if it is enabled, @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_context_menu_item_enabled_get(const Ewk_Context_Menu_Item* item);

#ifdef __cplusplus
}
#endif

#endif  // ewk_context_menu_internal_h
