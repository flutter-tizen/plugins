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

#ifndef ewk_hit_test_internal_h
#define ewk_hit_test_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Ewk_Hit_Test_Mode:
 * @EWK_HIT_TEST_MODE_DEFAULT: link data.
 * @EWK_HIT_TEST_MODE_NODE_DATA: extra node data(tag name, node value, attribute
 * infomation, etc).
 * @EWK_HIT_TEST_MODE_IMAGE_DATA: extra image data(image data, image data
 * length, image file name exteionsion, etc).
 * @EWK_HIT_TEST_MODE_ALL: all data.
 *
 * Enum values with flags representing the hit test mode.
 */

typedef enum {
  EWK_HIT_TEST_MODE_DEFAULT = 1 << 1,
  EWK_HIT_TEST_MODE_NODE_DATA = 1 << 2,
  EWK_HIT_TEST_MODE_IMAGE_DATA = 1 << 3,
  EWK_HIT_TEST_MODE_ALL = EWK_HIT_TEST_MODE_DEFAULT |
                          EWK_HIT_TEST_MODE_NODE_DATA |
                          EWK_HIT_TEST_MODE_IMAGE_DATA
} Ewk_Hit_Test_Mode;

/**
 * Ewk_Hit_Test_Result_Context:
 * @EWK_HIT_TEST_RESULT_CONTEXT_DOCUMENT: anywhere in the document.
 * @EWK_HIT_TEST_RESULT_CONTEXT_LINK: a hyperlink element.
 * @EWK_HIT_TEST_RESULT_CONTEXT_IMAGE: an image element.
 * @EWK_HIT_TEST_RESULT_CONTEXT_MEDIA: a video or audio element.
 * @EWK_HIT_TEST_RESULT_CONTEXT_SELECTION: the area is selected by the user.
 * @EWK_HIT_TEST_RESULT_CONTEXT_EDITABLE: the area is editable by the user.
 * @EWK_HIT_TEST_RESULT_CONTEXT_TEXT: the area is text.
 *
 * Enum values with flags representing the context of a #Ewk_Hit_Test.
 */
typedef enum {
  EWK_HIT_TEST_RESULT_CONTEXT_DOCUMENT = 1 << 1,
  EWK_HIT_TEST_RESULT_CONTEXT_LINK = 1 << 2,
  EWK_HIT_TEST_RESULT_CONTEXT_IMAGE = 1 << 3,
  EWK_HIT_TEST_RESULT_CONTEXT_MEDIA = 1 << 4,
  EWK_HIT_TEST_RESULT_CONTEXT_SELECTION = 1 << 5,
  EWK_HIT_TEST_RESULT_CONTEXT_EDITABLE = 1 << 6,
  EWK_HIT_TEST_RESULT_CONTEXT_TEXT = 1 << 7
} Ewk_Hit_Test_Result_Context;

/** Creates a type name for _Ewk_Hit_Test */
typedef struct _Ewk_Hit_Test Ewk_Hit_Test;

/**
 * Frees hit test instance created by ewk_view_hit_test_new().
 *
 * @param hit_test instance
 */
EXPORT_API void ewk_hit_test_free(Ewk_Hit_Test* hit_test);

/**
 * Gets the context of  the hit test
 *
 * @param hit_test instance
 * @return a bitmask of the hit test context on success, @c
 * EWK_HIT_TEST_RESULT_CONTEXT_DOCUMENT otherwise.
 */
EXPORT_API Ewk_Hit_Test_Result_Context
ewk_hit_test_result_context_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the link uri string of the hit test
 *
 * @param hit_test instance
 * @return the URI of the link element in the coordinates of the hit test on
 * success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_link_uri_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the link title string of the hit test
 *
 * @param hit_test instance
 * @return the title of the link element in the coordinates of the hit test on
 * success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_link_title_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the link label string of the hit test
 *
 * @param hit_test instance
 * @return the label of the link element in the coordinates of the hit test on
 * success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_link_label_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the image uri string of the hit test
 *
 * @param hit_test instance
 * @return the URI of the image element in the coordinates of the hit test on
 * success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_image_uri_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the media uri string of the hit test
 *
 * @param hit_test instance
 * @return the URI of the media element in the coordinates of the hit test on
 * success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_media_uri_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the tag name string of hit element of the hit test
 *
 * @param hit_test instance
 * @return the tag name of the hit element in the coordinates of the hit test on
 * success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_tag_name_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the node value string of hit element of the hit test
 *
 * @param hit_test instance
 * @return the node value of the hit element in the coordinates of the hit test
 * on success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_node_value_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the attribute data of hit element of the hit test
 *
 * @param hit_test instance
 * @return the attribute data of the hit element in the coordinates of the hit
 * test on success, @c 0 otherwise.
 */
EXPORT_API Eina_Hash* ewk_hit_test_attribute_hash_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the image buffer of hit element of the hit test
 *
 * @param hit_test instance
 * @return the image buffer of the hit element in the coordinates of the hit
 * test on success, @c 0 otherwise.
 */
EXPORT_API void* ewk_hit_test_image_buffer_get(Ewk_Hit_Test* hit_test);

/**
 * Gets the image buffer length of hit element of the hit test
 *
 * @param hit_test instance
 * @return the image buffer length of the hit element in the coordinates of the
 * hit test on success, @c 0 otherwise. Buffer length is truncated to 32 bit.
 */
EXPORT_API unsigned int ewk_hit_test_image_buffer_length_get(
    Ewk_Hit_Test* hit_test);

/**
 * Gets the image fiile name extension string of hit element of the hit test
 *
 * @param hit_test instance
 * @return the image fiile name extension of the hit element in the coordinates
 * of the hit test on success, @c 0 otherwise.
 */
EXPORT_API const char* ewk_hit_test_image_file_name_extension_get(
    Ewk_Hit_Test* hit_test);

#ifdef __cplusplus
}
#endif

#endif  // ewk_hit_test_internal_h
