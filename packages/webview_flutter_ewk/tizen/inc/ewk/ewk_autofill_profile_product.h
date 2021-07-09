/*
 * Copyright (C) 2018-2019 Samsung Electronics. All rights reserved.
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

#ifndef ewk_autofill_profile_product_h
#define ewk_autofill_profile_product_h

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
 * @if TV
 * @brief  The structure type that creates a type name for #Ewk_Form_Info.
 *
 * @since_tizen 5.0
 * @endif
 */
typedef struct _Ewk_Form_Info Ewk_Form_Info;

/**
 * @if TV
 * \enum   _Ewk_Form_Type
 * @brief  Enumeration that provides an option to form types.
 *
 * @since_tizen 5.0
 * @endif
 */
enum _Ewk_Form_Type {
  EWK_FORM_NONE = 0,
  EWK_FORM_USERNAME,
  EWK_FORM_PASSWORD,
  EWK_FORM_BOTH
};

/**
 * @if TV
 * @brief  Enumeration that creates a type name for the Ewk_Form_Type.
 *
 * @since_tizen 5.0
 * @endif
 */
typedef enum _Ewk_Form_Type Ewk_Form_Type;

/**
 * @if TV
 * @brief Gets the form type from a given form information
 *
 * @since_tizen 5.0
 *
 * @param[in] form information
 *
 * @return @c Type of form
 *
 * @endif
 */
EXPORT_API Ewk_Form_Type
ewk_autofill_profile_form_type_get(Ewk_Form_Info* info);

/**
 * @if TV
 * @brief Gets the user name from a given form information
 *
 * @since_tizen 5.0
 *
 * @param[in] form information
 *
 * @return @c user name
 *
 * @endif
 */
EXPORT_API const char* ewk_autofill_profile_form_user_name_get(
    Ewk_Form_Info* info);

/**
 * @if TV
 * @brief Gets the password from a given form info
 *
 * @since_tizen 5.0
 *
 * @param[in] form information
 *
 * @return @c password
 *
 * @endif
 */
EXPORT_API const char* ewk_autofill_profile_form_password_get(
    Ewk_Form_Info* info);

/**
 * @if TV
 * @brief Gets the user name element from a given form info
 *
 * @since_tizen 5.0
 *
 * @param[in] form information
 *
 * @return @c user name element
 *
 * @endif
 */
EXPORT_API const char* ewk_autofill_profile_form_username_element_get(
    Ewk_Form_Info* info);

/**
 * @if TV
 * @brief Gets the password element from a given form info
 *
 * @since_tizen 5.0
 *
 * @param[in] form information
 *
 * @return @c password element
 *
 * @endif
 */
EXPORT_API const char* ewk_autofill_profile_form_password_element_get(
    Ewk_Form_Info* info);

/**
 * @if TV
 * @brief Gets the action url from a given form info
 *
 * @since_tizen 5.0
 *
 * @param[in] form information
 *
 * @return @c action url
 *
 * @endif
 */
EXPORT_API const char* ewk_autofill_profile_form_action_url_get(
    Ewk_Form_Info* info);

/**
 * @if TV
 * @brief Gets the domain from a given form info
 *
 * @since_tizen 5.0
 *
 * @param[in] form information
 *
 * @return @c domain
 *
 * @endif
 */
EXPORT_API const char* ewk_autofill_profile_form_domain_get(
    Ewk_Form_Info* info);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_autofill_profile_product_h
