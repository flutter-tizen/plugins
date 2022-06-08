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

#ifndef EWK_EFL_INTEGRATION_PUBLIC_EWK_VALUE_PRODUCT_H_
#define EWK_EFL_INTEGRATION_PUBLIC_EWK_VALUE_PRODUCT_H_

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const void* Ewk_Value;

typedef int32_t Ewk_Value_Type;

/**
 * Increases the reference count of the given Ewk_Value.
 *
 * @param value the Ewk_Value instance to increase the reference count
 *
 * @return a pointer to the object on success, @c NULL otherwise.
 */
EXPORT_API Ewk_Value ewk_value_ref(Ewk_Value value);

/**
 * Decreases the reference count of the given Ewk_Value, freeing it when
 * the reference count reaches 0. For compound values (like arrays and
 * dictionaries) all values are removed.
 *
 * @param value the Ewk_Value instance to decrease the reference count
 */
EXPORT_API void ewk_value_unref(Ewk_Value value);

/**
 * Returns type of the given value.
 * The returned type can be compared to one returned by @c
 * ewk_value_xxx_type_get() functions.
 *
 * @param value the Ewk_Value to get type from
 *
 * @return the type of value
 *
 * @see ewk_value_null_type_get
 * @see ewk_value_boolean_type_get
 * @see ewk_value_double_type_get
 * @see ewk_value_int_type_get
 * @see ewk_value_string_type_get
 * @see ewk_value_array_type_get
 * @see ewk_value_dictionary_type_get
 */
EXPORT_API Ewk_Value_Type ewk_value_type_get(Ewk_Value value);

/**
 * Returns type of null/invalid Ewk_Value.
 *
 * @return the type
 */
EXPORT_API Ewk_Value_Type ewk_value_null_type_get();

/**
 * Creates Ewk_Value from boolean value and passes ownership to the caller.
 *
 * @param initial_value the initial value
 *
 * @return the created Ewk_Value
 */
EXPORT_API Ewk_Value ewk_value_boolean_new(Eina_Bool initial_value);

/**
 * Returns type of boolean Ewk_Value.
 *
 * @return the type
 */
EXPORT_API Ewk_Value_Type ewk_value_boolean_type_get();

/**
 * Gets boolean from Ewk_Value.
 * Fails if Ewk_Value is not of boolean type.
 *
 * @param value the value
 * @param dst the extracted value
 *
 * @return the EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_boolean_value_get(Ewk_Value value,
                                                 Eina_Bool* dst);

/**
 * Creates Ewk_Value from double and passes ownership to the caller.
 *
 * @param initial_value the initial value
 *
 * @return the created Ewk_Value
 */
EXPORT_API Ewk_Value ewk_value_double_new(double initial_value);

/**
 * Returns type of double Ewk_Value.
 *
 * @return the type
 */
EXPORT_API Ewk_Value_Type ewk_value_double_type_get();

/**
 * Gets double from Ewk_Value.
 * Fails if Ewk_Value is not of double type.
 *
 * @param value the value
 * @param dst the extracted value
 *
 * @return the EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_double_value_get(Ewk_Value value, double* dst);

/**
 * Creates Ewk_Value from integer and passes ownership to the caller.
 *
 * @param initial_value the initial value
 *
 * @return the created Ewk_Value
 */
EXPORT_API Ewk_Value ewk_value_int_new(int initial_value);

/**
 * Returns type of integer Ewk_Value.
 *
 * @return the type
 */
EXPORT_API Ewk_Value_Type ewk_value_int_type_get();

/**
 * Gets integer from Ewk_Value.
 * Fails if Ewk_Value is not of integer type.
 *
 * @param value the value
 * @param dst the extracted value
 *
 * @return the EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_int_value_get(Ewk_Value value, int* dst);

/**
 * Creates Ewk_Value from string and passes ownership to the caller.
 *
 * @param initial_value the initial value
 *
 * @return the created Ewk_Value
 */
EXPORT_API Ewk_Value ewk_value_string_new(const char* initial_value);

/**
 * Returns type of string Ewk_Value.
 *
 * @return the type
 */
EXPORT_API Ewk_Value_Type ewk_value_string_type_get();

/**
 * Gets string from Ewk_Value.
 * Fails if Ewk_Value is not of string type.
 *
 * @param value the value
 *
 * @return the EINA shared string value if successful, NULL otherwise.
 */
EXPORT_API Eina_Stringshare* ewk_value_string_value_get(Ewk_Value value);

/**
 * Creates Ewk_Value as array and passes ownership to the caller.
 *
 * @return the created Ewk_Value
 */
EXPORT_API Ewk_Value ewk_value_array_new();

/**
 * Returns type of array Ewk_Value.
 *
 * @return the type
 */
EXPORT_API Ewk_Value_Type ewk_value_array_type_get();

/**
 * Checks whether array is mutable. The given Ewk_Value must be of array type.
 *
 * @param array the value
 *
 * @return EINA_TRUE if array is mutable, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_array_is_mutable(Ewk_Value array);

/**
 * Adds @c value at the end of the @c array. The given @c array must be of array
 * type.
 * The value can be of any type. The value is deep copied.
 *
 * @param array the array
 * @param value the value
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_array_append(Ewk_Value array, Ewk_Value value);

/**
 * Returns size of the given @c array.
 *
 * @param array the array
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API size_t ewk_value_array_count(Ewk_Value array);

/**
 * Gets @c value from the @c array at given @c position.
 * WARNING: Currently copy of the value is returned and the ownership
 * of that value is passed to the caller.
 *
 * @param array the array
 * @param position the position
 * @param dst the extracted value
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_array_get(Ewk_Value array, size_t position,
                                         Ewk_Value* dst);

/**
 * Removes value from the @c array at given @c position.
 *
 * @param array the array
 * @param position the position
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_array_remove(Ewk_Value array, size_t position);

/**
 * Creates Ewk_Value as dictionary and passes ownership to the caller.
 *
 * @return the created Ewk_Value
 */
EXPORT_API Ewk_Value ewk_value_dictionary_new();

/**
 * Returns type of dictionary Ewk_Value.
 *
 * @return the type
 */
EXPORT_API Ewk_Value_Type ewk_value_dictionary_type_get();

/**
 * Checks whether dictionary is mutable. The given Ewk_Value must be of
 * dictionary type.
 *
 * @param dictionary the value
 *
 * @return EINA_TRUE if dictionary is mutable, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_dictionary_is_mutable(Ewk_Value dictionary);

/**
 * Gets array of existing keys in the @c dictionary. The given @c dictionary
 * must be of dictionary type.
 * WARNING: Currently array containing copies is returned and the ownership
 * of that array is passed to the caller.
 *
 * @param dictionary the dictionary
 * @param keys array of strings
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_dictionary_keys(Ewk_Value dictionary,
                                               Ewk_Value* keys);

/**
 * Sets @c value in the @c dictionary at given @c key. The given @c dictionary
 * must be of dictionary type.
 * The value can be of any type. Key must be a string value.
 * Overwrites old value if key existed before.
 * The value is deep copied.
 *
 * @param dictionary the dictionary
 * @param key the key
 * @param value the value
 * @param new_entry the EINA_TRUE if key did not exist before in dictionary,
 *        EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_dictionary_set(Ewk_Value dictionary,
                                              Ewk_Value key, Ewk_Value value,
                                              Eina_Bool* new_entry);

/**
 * Adds @c value to the @c dictionary at given @c key. The given @c dictionary
 * must be of dictionary type.
 * The value can be of any type. Key must be a string value.
 * Does nothing if key existed before.
 * The value is deep copied.
 *
 * @param dictionary the dictionary
 * @param key the key
 * @param value the value
 * @param new_entry the EINA_TRUE if key did not exist before in dictionary,
 *        EINA_FALSE otherwise.
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_dictionary_add(Ewk_Value dictionary,
                                              Ewk_Value key, Ewk_Value value,
                                              Eina_Bool* new_entry);

/**
 * Gets @c value from the @c dictionary at given @c key. The given @c dictionary
 * must be of dictionary type.
 * WARNING: Currently copy of the value is returned and the ownership
 * of that value is passed to the caller.
 *
 * @param dictionary the dictionary
 * @param key the key
 * @param dst the value
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_dictionary_get(Ewk_Value dictionary,
                                              Ewk_Value key, Ewk_Value* dst);

/**
 * Removes value from the @c dictionary at given @c key.
 *
 * @param dictionary the dictionary
 * @param key the key
 *
 * @return EINA_TRUE if successful, EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_value_dictionary_remove(Ewk_Value dictionary,
                                                 Ewk_Value key);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // EWK_EFL_INTEGRATION_PUBLIC_EWK_VALUE_PRODUCT_H_
