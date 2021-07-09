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
 * @file    ewk_history_internal.h
 * @brief   Describes the history(back & forward list) API of visited page.
 */

#ifndef ewk_history_internal_h
#define ewk_history_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Creates a type name for _Ewk_History */
typedef struct _Ewk_History Ewk_History;

/** Creates a type name for _Ewk_History_Item */
typedef struct _Ewk_History_Item Ewk_History_Item;

/**
 * Get the whole length of back list.
 *
 * @param history to get the whole back list
 *
 * @return number of elements in whole list.
 */
EXPORT_API int ewk_history_back_list_length_get(Ewk_History* history);

/**
 * Get the whole length of forward list.
 *
 * @param history to get the whole forward list
 *
 * @return number of elements in whole list.
 */
EXPORT_API int ewk_history_forward_list_length_get(Ewk_History* history);

/**
 * Get the item at a given index relative to the current item.
 *
 * @param history which history instance to query.
 * @param index position of item to get.
 *
 * @note The back list item in case of index < 0, the current item in case of
 * index == 0, the forward list item in case of index > 0.
 *
 * @return the item at a given index relative to the current item.
 */
EXPORT_API Ewk_History_Item* ewk_history_nth_item_get(Ewk_History* history,
                                                      int index);

/**
 * Query URI for given list item.
 *
 * @param item history item to query.
 *
 * @return the URI pointer, that may be @c NULL.
 */
EXPORT_API const char* ewk_history_item_uri_get(Ewk_History_Item* item);

/**
 * Query title for given list item.
 *
 * @param item history item to query.
 *
 * @return the title pointer, that may be @c NULL.
 */
EXPORT_API const char* ewk_history_item_title_get(Ewk_History_Item* item);

/**
 * Free given history instance.
 *
 * @param history what to free.
 */
EXPORT_API void ewk_history_free(Ewk_History* history);

#ifdef __cplusplus
}
#endif

#endif  // ewk_history_internal_h
