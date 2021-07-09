/*
 * Copyright (C) 2019 Samsung Electronics.
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
 * @file    ewk_autofill_credit_card_internal.h
 * @brief   This file describes the Ewk Autofill CreditCard API.
 */

#ifndef ewk_autofill_credit_card_internal_h
#define ewk_autofill_credit_card_internal_h

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
 * @if MOBILE
 * \enum   _Ewk_Autofill_Credit_Card_Data_Type
 * @brief  Enumeration that provides an option to autofill credit_card data
 * types.
 *
 * @since_tizen 4.0
 * @endif
 */

enum _Ewk_Autofill_Credit_Card_Data_Type {
  EWK_CREDIT_CARD_ID = 0,
  EWK_CREDIT_CARD_NAME_FULL,
  EWK_CREDIT_CARD_NUMBER,
  EWK_CREDIT_CARD_EXP_MONTH,
  EWK_CREDIT_CARD_EXP_4_DIGIT_YEAR,
  EWK_MAX_CREDIT_CARD
};

/**
 * @if MOBILE
 * @brief Enumeration that creates a type name for the
 * Ewk_Autofill_Credit_Card_Data_Type.
 *
 * @since_tizen 4.0
 * @endif
 */
typedef enum _Ewk_Autofill_Credit_Card_Data_Type
    Ewk_Autofill_Credit_Card_Data_Type;

/**
 * @if MOBILE
 * @brief The structure type that creates a type name for
 * #Ewk_Autofill_CreditCard.
 *
 * @since_tizen 4.0
 * @endif
 */
typedef struct _Ewk_Autofill_CreditCard Ewk_Autofill_CreditCard;

/**
 * @if MOBILE
 * @brief Creates a new credit_card
 *
 * The created credit_card must be deleted by ewk_autofill_credit_card_delete
 *
 * @since_tizen 4.0
 *
 * @return @c Ewk_Autofill_CreditCard if new credit_card is successfully
 created,
           @c NULL otherwise
 *
 * @see ewk_autofill_credit_card_data_set
 * @see ewk_autofill_credit_card_delete
 * @endif
 */
EXPORT_API Ewk_Autofill_CreditCard* ewk_autofill_credit_card_new(void);

/**
 * @if MOBILE
 * @brief Deletes a given credit_card
 *
 * The API will delete the a particular credit_card only from the memory.
 * To remove the credit_card permenantly use
 * ewk_context_form_autofill_credit_card_remove
 *
 * @since_tizen 4.0
 *
 * @param[in] credit_card name
 *
 * @see ewk_autofill_credit_card_new
 * @see ewk_context_form_autofill_credit_card_get
 * @see ewk_context_form_autofill_credit_card_remove
 * @endif
 */
EXPORT_API void ewk_autofill_credit_card_delete(Ewk_Autofill_CreditCard* card);

/**
 * @if MOBILE
 * @brief Sets the data in the credit_card created by
 * ewk_autofill_credit_card_new
 *
 * The data set by this function is set locally. To save it to database use
 * ewk_context_form_autofill_credit_card_add
 *
 * @since_tizen 4.0
 *
 * @param[in] credit_card contains the credit_card data
 * @param[in] type type of attribute to be set
 * @param[in] value value of the attribute
 *
 * @see ewk_autofill_credit_card_data_get
 * @see Ewk_Autofill_Credit_Card_Data_Type
 * @see ewk_context_form_autofill_credit_card_add
 * @endif
 */
EXPORT_API void ewk_autofill_credit_card_data_set(
    Ewk_Autofill_CreditCard* card, Ewk_Autofill_Credit_Card_Data_Type type,
    const char* value);

/**
 * @if MOBILE
 * @brief Gets the id attribute value from a given credit_card
 *
 * The credit_card obtained from ewk_context_form_autofill_credit_card_get will
 * be used to get the credit_cardid
 *
 * @param[in] credit_card name of credit_card
 *
 * @since_tizen 4.0
 *
 * @return @c Value of attribute (unsigned), @c 0 otherwise
 *
 * @see ewk_autofill_credit_card_new
 * @see ewk_context_form_autofill_credit_card_get
 * @see ewk_context_form_autofill_credit_card_get_all
 * @endif
 */
EXPORT_API unsigned ewk_autofill_credit_card_id_get(
    Ewk_Autofill_CreditCard* card);

/**
 * @if MOBILE
 * @brief Gets the attribute value from a given credit_card
 *
 * The credit_card obtained from ewk_context_form_autofill_credit_card_get will
 * be used to get the data
 *
 * @since_tizen 4.0
 *
 * @param[in] credit_card name of credit_card
 * @param[in] type name of attribute
 *
 * @return @c Value of attribute (Eina_Stringshare*), @c NULL otherwise
 * The string should be released with eina_stringshare_del()
 *
 * @see ewk_autofill_credit_card_new
 * @see ewk_context_form_autofill_credit_card_get
 * @see ewk_context_form_autofill_credit_card_get_all
 * @endif
 */
EXPORT_API Eina_Stringshare* ewk_autofill_credit_card_data_get(
    Ewk_Autofill_CreditCard* credit_card,
    Ewk_Autofill_Credit_Card_Data_Type type);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_autofill_credit_card_internal_h
