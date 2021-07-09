/*
 * Copyright (C) 2017 Samsung Electronics.
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
 * @file    ewk_form_repost_decision_product.h
 * @brief   This file describes the Ewk RepostForm API.
 */

#ifndef ewk_form_repost_decision_product_h
#define ewk_form_repost_decision_product_h

#include <Eina.h>

#include "tizen.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The structure type that creates a type name for
 * #Ewk_Form_Repost_Decision_Request
 *
 */
typedef struct _Ewk_Form_Repost_Decision_Request
    Ewk_Form_Repost_Decision_Request;

/**
 * Reply the result about form repost decision
 *
 * @param request Ewk_Form_Repost_Decision_Request object to get the
 *        information about repostform decision request.
 * @param allow result about repostform decision request.
 */
EXPORT_API void ewk_form_repost_decision_request_reply(
    Ewk_Form_Repost_Decision_Request* request, Eina_Bool allow);

#ifdef __cplusplus
}
#endif

#endif  // ewk_form_repost_decision_product_h
