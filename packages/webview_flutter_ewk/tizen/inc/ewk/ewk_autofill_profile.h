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
 * @file    ewk_autofill_profile.h
 * @brief   This file describes the Ewk Autofill Profile API.
 */

#ifndef ewk_autofill_profile_h
#define ewk_autofill_profile_h

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
 * \enum   _Ewk_Autofill_Profile_Data_Type
 * @brief  Enumeration that provides an option to autofill profile data types.
 *
 * @since_tizen 2.4
 * @endif
 */
enum _Ewk_Autofill_Profile_Data_Type {
  EWK_PROFILE_ID = 0,                /**< Id */
  EWK_PROFILE_NAME,                  /**< Name */
  EWK_PROFILE_COMPANY,               /**< Company */
  EWK_PROFILE_ADDRESS1,              /**< Address1 */
  EWK_PROFILE_ADDRESS2,              /**< Address2 */
  EWK_PROFILE_CITY_TOWN,             /**< City Town */
  EWK_PROFILE_STATE_PROVINCE_REGION, /**< State Province Region */
  EWK_PROFILE_ZIPCODE,               /**< Zipcode */
  EWK_PROFILE_COUNTRY,               /**< Country */
  EWK_PROFILE_PHONE,                 /**< Phone */
  EWK_PROFILE_EMAIL,                 /**< Email */
  EWK_MAX_AUTOFILL                   /**< Max Autofill */
};

/**
 * @if MOBILE
 * @brief Enumeration that creates a type name for the
 * Ewk_Autofill_Profile_Data_Type.
 *
 * @since_tizen 2.4
 * @endif
 */
typedef enum _Ewk_Autofill_Profile_Data_Type Ewk_Autofill_Profile_Data_Type;

/**
 * @if MOBILE
 * @brief The structure type that creates a type name for #Ewk_Autofill_Profile.
 *
 * @since_tizen 2.4
 * @endif
 */
typedef struct _Ewk_Autofill_Profile Ewk_Autofill_Profile;

/**
 * @if MOBILE
 * @brief Creates a new profile
 *
 * The created profile must be deleted by ewk_autofill_profile_delete
 *
 * @since_tizen 2.4
 *
 * @return @c Ewk_Autofill_Profile if new profile is successfully created,
           @c NULL otherwise
 *
 * @see ewk_autofill_profile_data_set
 * @see ewk_autofill_profile_delete
 * @endif
 */
EXPORT_API Ewk_Autofill_Profile* ewk_autofill_profile_new(void);

/**
 * @if MOBILE
 * @brief Deletes a given profile
 *
 * The API will delete the a particular profile only from the memory.
 * To remove the profile permanently use
 * ewk_context_form_autofill_profile_remove
 *
 * @since_tizen 2.4
 *
 * @param[in] profile name
 *
 * @see ewk_autofill_profile_new
 * @see ewk_context_form_autofill_profile_get
 * @see ewk_context_form_autofill_profile_remove
 * @endif
 */
EXPORT_API void ewk_autofill_profile_delete(Ewk_Autofill_Profile* profile);

/**
 * @if MOBILE
 * @brief Sets the data in the profile created by ewk_autofill_profile_new
 *
 * The data set by this function is set locally. To save it to database use
 * ewk_context_form_autofill_profile_add
 *
 * @since_tizen 2.4
 *
 * @param[in] profile contains the profile data
 * @param[in] name type of attribute to be set
 * @param[in] value value of the attribute
 *
 * @see ewk_autofill_profile_data_get
 * @see Ewk_Autofill_Profile_Data_Type
 * @see ewk_context_form_autofill_profile_add
 * @endif
 */
EXPORT_API void ewk_autofill_profile_data_set(
    Ewk_Autofill_Profile* profile, Ewk_Autofill_Profile_Data_Type name,
    const char* value);

/**
 * @if MOBILE
 * @brief Gets the id attribute value from a given profile
 *
 * The profile obtained from ewk_context_form_autofill_profile_get will be used
 * to get the profile id
 *
 * @param[in] profile name of profile
 *
 * @since_tizen 2.4
 *
 * @return @c Value of attribute (unsigned), @c 0 otherwise
 *
 * @see ewk_autofill_profile_new
 * @see ewk_context_form_autofill_profile_get
 * @see ewk_context_form_autofill_profile_get_all
 * @endif
 */
EXPORT_API unsigned ewk_autofill_profile_id_get(Ewk_Autofill_Profile* profile);

/**
 * @if MOBILE
 * @brief Gets the attribute value from a given profile
 *
 * The profile obtained from ewk_context_form_autofill_profile_get will be used
 * to get the data
 *
 * @since_tizen 2.4
 *
 * @param[in] profile name of profile
 * @param[in] name name of attribute
 *
 * @return @c Value of attribute (char*), @c NULL otherwise
 *
 * @see ewk_autofill_profile_new
 * @see ewk_context_form_autofill_profile_get
 * @see ewk_context_form_autofill_profile_get_all
 * @endif
 */
EXPORT_API const char* ewk_autofill_profile_data_get(
    Ewk_Autofill_Profile* profile, Ewk_Autofill_Profile_Data_Type name);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_autofill_profile_h
