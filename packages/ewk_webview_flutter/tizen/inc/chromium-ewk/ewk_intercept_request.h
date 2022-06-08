/*
 * Copyright (C) 2013-2016 Samsung Electronics.
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
 * @file    ewk_intercept_request.h
 * @brief   This file describes the Intercept Request EWK API.
 */

#ifndef EWK_EFL_INTEGRATION_PUBLIC_EWK_INTERCEPT_REQUEST_H_
#define EWK_EFL_INTEGRATION_PUBLIC_EWK_INTERCEPT_REQUEST_H_

#include <Eina.h>
#include <stddef.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @brief Handle for intercepted request. Used for getting information about
 *        request and writing custom response.
 *
 * @since_tizen 3.0
 */
typedef struct _Ewk_Intercept_Request Ewk_Intercept_Request;

/**
 * @brief Returns request url from Intercept Request object.
 *
 * @remarks Returned string is owned by Intercept Request object, you have to
 *          make a copy if you want to use it past owner lifetime.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request intercept request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 *
 * @return @c url string on success or NULL on failure
 */
EXPORT_API const char* ewk_intercept_request_url_get(
    Ewk_Intercept_Request* intercept_request);

/**
 * @brief Returns request method from Intercept Request object.
 *
 * @remarks Returned string is owned by Intercept Request object, you have to
 *          make a copy if you want to use it past owner lifetime.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request intercept request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 *
 * @return @c method string on success or NULL on failure
 */
EXPORT_API const char* ewk_intercept_request_http_method_get(
    Ewk_Intercept_Request* intercept_request);

/**
 * @brief Returns request headers from Intercept Request object.
 *
 * @remarks Returned hash map is owned by Intercept Request object, you have to
 *          make a copy if you want to use it past owner lifetime.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request intercept request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 *
 * @return @c Eina_Hash mapping from header name string to header value string
 *            on success or NULL on failure
 */
EXPORT_API const Eina_Hash* ewk_intercept_request_headers_get(
    Ewk_Intercept_Request* intercept_request);

/**
 * @brief Ignores request so engine will handle it normally.
 *
 * @details When application doesn't have a response for intercepted request
 *          url, it calls this function, which notifies engine to proceed with
 *          normal resource loading.
 *
 * @remarks After this call, handling the request is done. Any further calls on
 *          the Ewk_Intercept_Request instance result in undefined behavior.
 *
 *          Only use this function from inside the
 *          #Ewk_Context_Intercept_Request_Callback.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request Intercept Request instance received in
 *            #Ewk_Context_Intercept_Request_Callback
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_intercept_request_ignore(Ewk_Intercept_Request* intercept_request);

/**
 * @brief Writes whole response with headers at once.
 *
 * @details This function can be used to write whole response at once. To call
 *          it, application should have full response headers and body ready
 *          for the intercepted request.
 *
 * @remarks It is allowed to use this function both inside and outside
 *          the #Ewk_Context_Intercept_Request_Callback.
 *
 *          After this call, handling the request is done. Any further calls on
 *          the Ewk_Intercept_Request instance result in undefined behavior.
 *
 *          You can't use this call after you started writing response in
 *          chunks with @a ewk_intercept_request_response_write_chunk.
 *
 *          Alternatively you can use following functions which are more
 *          convenient:
 *          - ewk_intercept_request_response_status_set()
 *          - ewk_intercept_request_response_header_add()
 *          - ewk_intercept_request_response_header_map_add()
 *          - ewk_intercept_request_response_body_set()
 *          - ewk_intercept_request_response_write_chunk()
 *
 *          Using this function overrides status and headers set with functions
 *          listed above.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request Intercept Request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 * @param[in] headers Null-terminated string representing response's headers
 *            for the intercept request.
 *            By HTTP spec, lines should end with a newline ('\\r\\n') and
 *            headers should end with an empty line ('\\r\\n\\r\\n').
 * @param[in] body Response body for the intercept request
 * @param[in] length Length of response body
 *
 * @return @c EINA_TRUE on success or EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_intercept_request_response_set(
    Ewk_Intercept_Request* intercept_request, const char* headers,
    const char* body, size_t length);

/**
 * @brief Sets status code and status text of response for intercepted request.
 *
 * @remarks It is allowed to use this function both inside and outside the
 *          #Ewk_Context_Intercept_Request_Callback.
 *
 *          In case of failure of this function finish writing for this
 *          intercept request.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request Intercept Request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 * @param[in] status_code HTTP response status code
 * @param[in] custom_status_text HTTP response reason phrase, pass NULL to use
 *            recommended reason phrase (example: "OK" for code 200)
 *
 * @return @c EINA_TRUE on success or EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_intercept_request_response_status_set(
    Ewk_Intercept_Request* intercept_request, int status_code,
    const char* custom_status_text);

/**
 * @brief Adds HTTP header to response for intercepted request.
 *
 * @remarks It is allowed to use this function both inside and outside
 *          the #Ewk_Context_Intercept_Request_Callback.
 *
 *          In case of failure of this function finish writing for this
 *          intercept request.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request Intercept Request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 * @param[in] field_name HTTP header field name
 * @param[in] field_value HTTP header field value
 *
 * @return @c EINA_TRUE on success or EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_intercept_request_response_header_add(
    Ewk_Intercept_Request* intercept_request, const char* field_name,
    const char* field_value);

/**
 * @brief Adds HTTP headers to response for intercepted request.
 *
 * @remarks It is allowed to use this function both inside and outside
 *          the #Ewk_Context_Intercept_Request_Callback.
 *
 *          In case of failure of this function finish writing for this
 *          intercept request.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request Intercept Request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback.
 * @param[in] headers Map from HTTP header field names to field values, both
 *            represented as null terminated strings
 *
 * @return @c EINA_TRUE on success or EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_intercept_request_response_header_map_add(
    Ewk_Intercept_Request* intercept_request, const Eina_Hash* headers);

/**
 * @brief Writes whole response body at once.
 *
 * @details This function can be used to write response body. To call it,
 *          application should have full response body ready for the
 *          intercepted request.
 *
 * @remarks It is allowed to use this function both inside and outside
 *          the #Ewk_Context_Intercept_Request_Callback.
 *
 *          After this call, handling the request is done. Any further calls on
 *          the Ewk_Intercept_Request instance result in undefined behavior.
 *
 *          You can't use this call after you started writing response
 *          in chunks with @a ewk_intercept_request_response_write_chunk.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request Intercept Request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 * @param[in] body Response body for the intercept request
 * @param[in] length Length of response body
 *
 * @return @c EINA_TRUE on success or EINA_FALSE on failure
 *
 * @pre Before writing response body, you should set response status and
 *      headers using the following functions:
 *      - ewk_intercept_request_response_status_set()
 *      - ewk_intercept_request_response_header_add()
 *      - ewk_intercept_request_response_header_map_add()
 *
 */
EXPORT_API Eina_Bool ewk_intercept_request_response_body_set(
    Ewk_Intercept_Request* intercept_request, const char* body, size_t length);

/**
 * @brief Writes a part of response body.
 *
 * @details This function can be used to write response body in chunks.
 *          Application doesn't have to prepare full response body before
 *          calling this function, it can be used as soon as a part of response
 *          is ready.
 *
 * @remarks If this function returns EINA_FALSE, handling the request is done.
 *          Any further calls on the Ewk_Intercept_Request instance result in
 *          undefined behavior. User should always check return value, because
 *          response to this request might not be needed anymore, and function
 *          can return EINA_FALSE even though user still has data to write.
 *
 *          It is only allowed to use this function *outside* of the
 *          #Ewk_Context_Intercept_Request_Callback.
 *
 *          If a part of response body has been written with this function, you
 *          can't use the following functions anymore:
 *          - ewk_intercept_request_response_set
 *          - ewk_intercept_request_response_body_set
 *
 *          Using this function with too big chunks of data defeats its
 *          purpose. Engine uses chunks of response written with this function
 *          without waiting for end of data, which is its main advantage over
 *          @a ewk_intercept_request_response_body_set.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request Intercept Request instance received from
 *            #Ewk_Context_Intercept_Request_Callback ewk_context callback
 * @param[in] chunk Part of response body for intercepted request
 * @param[in] length Length of response body part
 *
 * @return @c EINA_TRUE on success or EINA_FALSE on failure
 *
 * @pre Before writing response body, you should set response status
 *      and headers using the following functions:
 *      - ewk_intercept_request_response_status_set()
 *      - ewk_intercept_request_response_header_add()
 *      - ewk_intercept_request_response_header_map_add()
 *
 * @post After writing full response body in chunks using this function, call
 *       it again with NULL as @a chunk and 0 as @a length, to signal that
 *       response body is finished.
 *
 *       Use the same arguments if there is a failure with preparing the rest
 *       of response body on application side, and you don't want or can't
 *       continue writing parts.
 */
EXPORT_API Eina_Bool ewk_intercept_request_response_write_chunk(
    Ewk_Intercept_Request* intercept_request, const char* chunk, size_t length);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // EWK_EFL_INTEGRATION_PUBLIC_EWK_INTERCEPT_REQUEST_H_
