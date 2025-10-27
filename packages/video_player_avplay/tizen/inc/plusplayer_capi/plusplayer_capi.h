/**
 * @file           plusplayer_capi.h
 * @brief          PlusPlayer api c version
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is plusplayer api header implemented as C style to
 *                 avoid binary compatibility issues.
 *
 * Copyright (c) 2025 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_PLUSPLAYER_CAPI_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_PLUSPLAYER_CAPI_H__

#include "plusplayer_capi/appinfo.h"
#include "plusplayer_capi/attribute.h"
#include "plusplayer_capi/display.h"
#include "plusplayer_capi/drm.h"
#include "plusplayer_capi/error.h"
#include "plusplayer_capi/property.h"
#include "plusplayer_capi/state.h"
#include "plusplayer_capi/streaming_message.h"
#include "plusplayer_capi/track.h"
#include "plusplayer_capi/track_capi.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Callback for error events
 * @param error_type Type of error occurred
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_error_cb)(plusplayer_error_type_e error_type,
                                    void* user_data);

/**
 * @brief Callback for asynchronous preparation completion
 * @param prepare_flag True if preparation succeeded, false otherwise
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_prepare_async_done_cb)(bool prepare_flag,
                                                 void* user_data);

/**
 * @brief Callback for resource conflict events
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_resource_conflicted_cb)(void* user_data);

/**
 * @brief Callback for end of stream event
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_eos_cb)(void* user_data);

/**
 * @brief Callback for seek completion
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_seek_done_cb)(void* user_data);

/**
 * @brief Callback for buffer status updates
 * @param buffer_level Current buffer level percentage (0-100)
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_buffer_status_cb)(int buffer_level, void*);

/**
 * @brief Callback for DRM initialization data reception
 * @param handle DRM session handle
 * @param data_size Size of the initialization data
 * @param data Pointer to the initialization data buffer
 * @param track_type Type of media track associated with the data
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_drm_init_data_cb)(Plusplayer_DrmHandle* handle,
                                            unsigned int data_size,
                                            unsigned char* data,
                                            plusplayer_track_type_e track_type,
                                            void* user_data);

/**
 * @brief Callback for adaptive streaming control events
 * @param message_type Type of streaming message/event
 * @param param Pointer to message-specific parameters
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_adaptive_streaming_control_event_cb)(
    plusplayer_streaming_message_type_e message_type,
    plusplayer_message_param_s* param, void* user_data);

/**
 * @brief Callback for error message details
 * @param error_type Type of error occurred
 * @param message Human-readable error message
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_error_msg_cb)(plusplayer_error_type_e error_type,
                                        const char* message, void* user_data);
/**
 * @brief Callback function for Subtitle Data updated information
 * @param type subtitle type (text or picture)
 * @param duration_in_ms subtitle duration (ms)
 * @param data Subtitle data
 * @param size subtitle data length
 * @param attr_list list of subtitle attribute
 * @param attr_size length of subtitle attribute list
 * @param user_data Application-specific user data
 */
typedef void (*plusplayer_subtitle_updated_cb)(
    const plusplayer_subtitle_type_e type, const uint64_t duration_in_ms,
    const char* data, const int size, plusplayer_subtitle_attr_s* attr_list,
    int attr_size, void* userdata);

/**
 * @brief Callback for AD Event from DASH.
 * @param ad_data Ad information c_str ends with '\0'
 * @param user_data Application-specific user data
 * @pre   On callback registration, it will not be invoked before player
 * prepare.
 */
typedef void (*plusplayer_ad_event_cb)(const char* ad_data, void* userdata);

/**
 * @brief Callback for track information.
 * @param track Track handle. Caller can use this handle to get information.
 * @param user_data Application-specific user data
 * @return @c true to continue with the next iteration of the loop,
 *         otherwise @c false to break out of the loop
 */
typedef bool (*plusplayer_track_cb)(const plusplayer_track_h track,
                                    void* userdata);

/**
 * @brief Handle for PlusPlayer CAPI
 */
typedef void* plusplayer_h;

/**
 * @brief Handle for PlusPlayer Track
 */
typedef void* plusplayer_track_h;

/**
 * @brief     Create a plusplayer handle.
 * @param     [in] void
 * @return    return plusplayer handle pointer.
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       None
 * @post      The player state will be #PLUSPLAYER_STATE_NONE.
 * @exception None
 * @remark    Caller must ensure all APIs are called from the same thread as
 * this API is invoked.
 */
plusplayer_h plusplayer_create(void);

/**
 * @brief     Open plusplayer handle.
 * @param     [in] handle : plusplayer handle
 * @param     [in] uri : content uri.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            // ... your codes ...
 *            plusplayer_close(player);
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_NONE.
 * @post      The player state will be #PLUSPLAYER_STATE_IDLE.
 * @exception None
 * @remark     Caller must provide a valid HTTP(S) URI. Local file sources
 * (e.g., file:///path) is not supported.
 * @see       plusplayer_close() \n
 *            plusplayer_prepare()
 */
int plusplayer_open(plusplayer_h handle, const char* uri);

/**
 * @brief     Prepare the player for playback, synchronously.
 * @param     [in] handle : plusplayer handle
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed.
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player); // SYNCHRONOUS operation
 *            // ... your codes ...
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_IDLE.
 * @post      The player state will be #PLUSPLAYER_STATE_READY
 *            or #PLUSPLAYER_STATE_TRACK_SOURCE_READY (only in case of prebuffer
 * mode)
 * @exception None
 * @see       plusplayer_open()
 */
int plusplayer_prepare(plusplayer_h handle);

/**
 * @brief     Start media playback.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed.
 * @code
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 *            // ... your codes ...
 * @endcode
 * @pre       The player state should be #PLUSPLAYER_STATE_READY.
 * @post      The player state will be #PLUSPLAYER_STATE_PLAYING.
 * @exception None
 * @see       plusplayer_open() \n
 *            plusplayer_prepare() \n
 *            plusplayer_start() \n
 *            plusplayer_stop() \n
 *            plusplayer_close()
 */
int plusplayer_start(plusplayer_h handle);

/**
 * @brief     Stop playing media content.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed.
 * @code
 *            plusplayer_open();
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 *             // ... your codes ...
 *            plusplayer_close(player);
 * @endcode
 * @pre       The player state must be all of #plusplayer_state_e except
 * #PLUSPLAYER_STATE_NONE.
 * @post      The player state will be #PLUSPLAYER_STATE_IDLE.
 * @exception None
 * @remark    plusplayer_close() must be called once after player is stopped
 * @see       plusplayer_open() \n
 *            plusplayer_prepare() \n
 *            plusplayer_start() \n
 *            plusplayer_close()
 */
int plusplayer_stop(plusplayer_h handle);

/**
 * @brief     Release all the player resources except callback functions.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #plusplayer_ERROR_TYPE_NONE Successful
 * @retval    #plusplayer_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #plusplayer_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed.
 * @pre       The player state must be all of #plusplayer_state_e except
 *            #PLUSPLAYER_STATE_NONE.
 * @post      The player state will be #PLUSPLAYER_STATE_NONE.
 * @exception None
 * @see       plusplayer_open()
 */
int plusplayer_close(plusplayer_h handle);

/**
 * @brief     Release a player handle.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed.
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_NONE
 * @post      player handle will be removed.
 * @exception None
 * @see       plusplayer_create()
 */
int plusplayer_destroy(plusplayer_h handle);

/**
 * @brief     Get current state of player.
 * @param     [in] handle : plusplayer handle.
 * @return    current #plusplayer_state_e of player.
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            // ... your codes ...
 *            plusplayer_state_e ret = plusplayer_get_state(player);
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE.
 * @post      The player state will be same as @pre.
 * @exception None
 */
plusplayer_state_e plusplayer_get_state(plusplayer_h handle);

/**
 * @brief     Set the video display type and window pointer.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] type : display type.
 * @param     [in] window : the handle to display window.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed.
 * @code
 *            plusplayer_open(player,uri);
 *            plusplayer_set_display(player,plusplayer_display_type_e_OVERLAY,window);
 *            // ... your codes ...
 *            plusplayer_close(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE.
 * @post      None
 * @exception None
 * @remark    We are not supporting changing display. \n
 *            This API have to be called before calling
 *            plusplayer_prepare() or plusplayer_prepare_async() to reflect the
 * display type.
 * @see       plusplayer_open() \n
 *            plusplayer_set_display_mode() \n
 *            plusplayer_set_display_roi()
 */
int plusplayer_set_display(plusplayer_h handle, plusplayer_display_type_e type,
                           void* window);

/**
 * @brief     Set the video display type, surface id and display size.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] type : display type.
 * @param     [in] surface_id : resource id of window.
 * @param     [in] roi : plusplayer_geometry_s.
 * @return    @c one of plusplayer_error_type_e values will be returned.
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE.
 * @post      None
 * @exception None
 * @see       plusplayer_set_display_mode() \n
 *            plusplayer_set_display_roi()
 */
int plusplayer_set_display_subsurface(plusplayer_h handle,
                                      plusplayer_display_type_e type,
                                      uint32_t surface_id,
                                      plusplayer_geometry_s roi);

/**
 * @brief     Set a callback function to be invoked when player is prepared
 * completed.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] prepare_async_done_cb : the prepare async done callback
 * function to register.
 * @param     [in] userdata : pointer to caller object of
 * prepare_async_done_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            refer to the sample code of plusplayer_prepare_async();
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_prepare_async_done_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_prepare_async_done_cb callback
 * @remark    plusplayer_prepare_async_done_cb() \n
 *            if prepare_async_done_cb is set to null,
 *            plusplayer_prepare_async_done_cb() will not be
 *            invoked anymore.
 * @see       plusplayer_prepare_async().
 */
int plusplayer_set_prepare_async_done_cb(
    plusplayer_h handle, plusplayer_prepare_async_done_cb prepare_async_done_cb,
    void* userdata);

/**
 * @brief     Set a callback function to be invoked when resource conflicted.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] resource_conflicted_cb : the resource conflict callback
 * function to be called.
 * @param     [in] userdata : pointer to caller object of
 * resource_conflicted_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void resource_conflicted_cb(void* userdata){
 *                //Something you want to do when Resource Conflict occur
 *            }
 *            plusplayer_h player1 = plusplayer_create();
 *            plusplayer_h player2 = plusplayer_create();
 *            plusplayer_set_resource_conflicted_cb(player1,
 * resource_conflicted_cb, NULL);
 *            // ... your codes ...
 *            plusplayer_prepare(player1);
 *            plusplayer_prepare(player2);
 *            // ... your codes ...
 *            plusplayer_close(player1);
 *            plusplayer_destroy(player1);
 *            plusplayer_close(player2);
 *            plusplayer_destroy(player2);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_resource_conflicted_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_resource_conflicted_cb callback
 * @remark    plusplayer_resource_conflicted_cb() \n
 *            if resource_conflicted_cb is set to null,
 *            plusplayer_resource_conflicted_cb() will not be
 *            invoked anymore.
 */
int plusplayer_set_resource_conflicted_cb(
    plusplayer_h handle,
    plusplayer_resource_conflicted_cb resource_conflicted_cb, void* userdata);

/**
 * @brief     Set a callback function to be invoked when end of stream (EOS) is
 * reached.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] eos_cb : the EOS callback function to be called.
 * @param     [in] userdata : pointer to caller object of eos_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void eos_cb(void* userdata){
 *                //Something you want to do when EOS received
 *            }
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_set_eos_cb(player, eos_cb, NULL);
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_eos_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_eos_cb callback
 * @remark    plusplayer_eos_cb()
 *            if eos_cb is set to null,
 *            plusplayer_eos_cb() will not be
 *            invoked anymore.
 */
int plusplayer_set_eos_cb(plusplayer_h handle, plusplayer_eos_cb eos_cb,
                          void* userdata);

/**
 * @brief     Set a callback function to be invoked for buffer status.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] buffer_status_cb : the buffer status callback function to be
 * called.
 * @param     [in] userdata : pointer to caller object of buffer_status_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void buffer_status_cb(int percent, void* userdata){
 *                //Something you want to do with buffer percent
 *            }
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_set_buffer_status_cb(player, buffer_status_cb, NULL);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_buffer_status_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_buffer_status_cb callback
 * @remark    plusplayer_buffer_status_cb() \n
 *            if buffer_status_cb is set to null,
 *            plusplayer_buffer_status_cb() will not be
 *            invoked anymore.
 */
int plusplayer_set_buffer_status_cb(
    plusplayer_h handle, plusplayer_buffer_status_cb buffer_status_cb,
    void* userdata);

/**
 * @brief     Sets a callback function to be invoked when an error occurs.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] error_cb : the error callback function to register.
 * @param     [in] userdata : pointer to caller object of plusplayer_error_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void error_cb(const plusplayer_error_type_e err_code, void*
 *                userdata) {
 *                //Something you want to do when error occur
 *            }
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_set_error_cb(player, error_cb, NULL);
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_error_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_error_cb callback
 * @remark    plusplayer_error_cb() \n
 *            if plusplayer_error_cb is set to null,
 *            plusplayer_error_cb() will not be
 *            invoked anymore.
 */
int plusplayer_set_error_cb(plusplayer_h handle, plusplayer_error_cb error_cb,
                            void* userdata);

/**
 * @brief     Sets a callback function to be invoked when an error occurs.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] error_msg_cb : the error callback function to register.
 * @param     [in] userdata : pointer to caller object of
 * plusplayer_error_msg_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void error_msg_cb(const plusplayer_error_type_e err_code,
 * const char* error_msg, void* userdata) {
 *                //Something you want to do when error occur
 *            }
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_set_error_msg_cb(player, error_msg_cb, NULL);
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_error_msg_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_error_msg_cb callback
 * @remark    plusplayer_error_msg_cb() \n
 *            if plusplayer_error_msg_cb is set to null,
 *            plusplayer_error_msg_cb() will not be
 *            invoked anymore.
 */
int plusplayer_set_error_msg_cb(plusplayer_h handle,
                                plusplayer_error_msg_cb error_msg_cb,
                                void* userdata);

/**
 * @brief     Set a callback function to be invoked when player seek is
 * completed.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] seek_done_cb : the seek done callback function to register.
 * @param     [in] userdata : pointer to caller object of
 * plusplayer_seek_done_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            refer to the sample code of plusplayer_seek();
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_seek_done_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_seek_done_cb callback
 * @remark    plusplayer_seek_done_cb() \n
 *            if seek_done_cb is set to null,
 *            plusplayer_seek_done_cb() will
 *            not be invoked anymore.
 */
int plusplayer_set_seek_done_cb(plusplayer_h handle,
                                plusplayer_seek_done_cb seek_done_cb,
                                void* userdata);

/**
 * @brief     Set a callback function to be invoked when there is any subtitle
 information to display.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] subtitle_updated_cb : the subtitle data callback function to
 register.
 * @param     [in] userdata : pointer to caller object of
 * plusplayer_subtitle_updated_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void subtitle_data_cb(const plusplayer_subtitle_type_e
 type, const uint64_t duration_in_ms, const char* data, const int size,
 plusplayer_subtitle_attr_s* attr_list, int attr_size, void* userdata) {
 *            //Something you want to do on when subtitle data is updated
 *            }
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_set_subtitle_updated_cb(player, subtitle_data_cb,
 NULL);
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_subtitle_updated_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_subtitle_updated_cb callback
 * @remark    plusplayer_subtitle_updated_cb() \n
 *            if subtitle_updated_cb is set to null,
 *            plusplayer_subtitle_updated_cb() will
 *            not be invoked anymore.
 */
int plusplayer_set_subtitle_updated_cb(
    plusplayer_h handle, plusplayer_subtitle_updated_cb subtitle_updated_cb,
    void* userdata);

/**
 * @brief     Sets a callback function to be invoked when dash ad event is
 * parsed.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] ad_event_cb : the error callback function to register.
 * @param     [in] userdata : pointer to caller object of plusplayer_error_cb().
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void ad_event_cb(const char* ad_data, void* userdata) {
 *                //Something you want to do when dash scte:35 cue event is
 * parsed
 *            }
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_set_ad_event_cb(player, ad_event_cb, NULL);
 *            // ... your codes ...
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_ad_event_cb() will be invoked.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_ad_event_cb callback
 * @remark    callback function @c ad_event_cb will be invoked after player
 * prepare, when dash scte:35 cue event is parsed.
 * @remark    plusplayer_ad_event_cb() \n
 *            if plusplayer_ad_event_cb is set to null,
 *            plusplayer_ad_event_cb() will not be
 *            invoked anymore.
 */
int plusplayer_set_ad_event_cb(plusplayer_h handle,
                               plusplayer_ad_event_cb ad_event_cb,
                               void* userdata);

/**
 * @brief     Prepare the player for playback, asynchronously.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            static void prepare_done_cb(bool ret, void* userdata) {
 *                //Something you want to do when prepare done, but, we strongly
 *                //recommend DO NOT CALL PLAYER APIs in this callbck
 *            }
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_set_prepare_async_done_cb(player,
 * prepare_done_cb, NULL);
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare_async(player); // ASYNC operation
 *            // ... your codes ...
 *            plusplayer_close(player);
 *            plusplayer_destroy(player);
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_IDLE.
 * @post      The player state will be #PLUSPLAYER_STATE_READY
 *            or #PLUSPLAYER_STATE_TRACK_SOURCE_READY (only in case of prebuffer
 * mode)
 * @exception None
 * @remark    plusplayer_prepare_async_done_cb() is invoked.
 * @see       plusplayer_open() \n
 *            plusplayer_stop() \n
 *            plusplayer_close()
 */
int plusplayer_prepare_async(plusplayer_h handle);

/**
 * @brief     Pause playing media content.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            prepare player done
 *            // ... your codes ...
 *            plusplayer_pause(player);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player state must be one of #PLUSPLAYER_STATE_READY or
 *            #PLUSPLAYER_STATE_PAUSED or #PLUSPLAYER_STATE_PLAYING.
 * @post      The player state will be #PLUSPLAYER_STATE_PAUSE.
 * @exception None
 * @see       plusplayer_start() \n
 *            plusplayer_resume() \n
 *            plusplayer_prepare_async()
 */
int plusplayer_pause(plusplayer_h handle);

/**
 * @brief     Resume playing media content.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed
 * @code
 *            prepare player done
 *            // ... your codes ...
 *            plusplayer_pause(player);
 *            // ... your codes ...
 *            plusplayer_resume(player);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player state must be one of #PLUSPLAYER_STATE_PAUSED or
 *            #PLUSPLAYER_STATE_PLAYING.
 * @post      The player state will be #PLUSPLAYER_STATE_PLAYING.
 * @exception None
 * @see       plusplayer_start() \n
 *            plusplayer_pause() \n
 *            plusplayer_prepare()
 */
int plusplayer_resume(plusplayer_h handle);

/**
 * @brief     Seek/Jump playback to position passed in parameter,
 * asynchronously.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] time : the absolute position(playingtime) of
 *            the stream in milliseconds
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @remark    In case of non-seekable content, it will return @c
 * PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION. \n If application ignore this error,
 * player will keep playing without changing play position.
 * @code
 *            static void seek_done_cb(void* userdata) {
 *                //Something you want to do when seek is completed
 *            }
 *            plusplayer_set_seek_done_cb(handle_, seek_done_cb, this);
 *            // ... your codes ...
 *            const uint64_t ms_to_seek = 0;
 *            plusplayer_seek(player,ms_to_seek);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player state must be one of #PLUSPLAYER_STATE_READY or
 *            #PLUSPLAYER_STATE_PAUSED or #PLUSPLAYER_STATE_PLAYING.
 *            In PLUSPLAYER_STATE_IDLE, this api can be called exceptionally
 *            between plusplayer_open() and plusplayer_prepare_async().
 *            the start time of plyabak can be set explicitly when starting
 *            first playback. In this case, plusplayer_set_seek_done_cb is not
 *            called.
 * @post      None
 * @exception None
 * @remark    plusplayer_set_seek_done_cb() will be invoked when seek operation
 *            is finished.
 */
int plusplayer_seek(plusplayer_h handle, uint64_t time);

/**
 * @brief     Enable prebuffer mode.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] prebuffer_mode : Boolean to set prebuffer mode
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            // ... your codes ...
 *            plusplayer_set_prebuffer_mode(player,true);
 *            uint64_t resume_time_ms = 5000;
 *            plusplayer_set_resume_time(player, resume_time_ms);
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *            plusplayer_state_e curr_state = plusplayer_get_state(player); //
 * curr_state will be #PLUSPLAYER_STATE_TRACK_SOURCE_READY
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be #PLUSPLAYER_STATE_IDLE.
 * @exception None
 * @see       plusplayer_set_property()
 */
void plusplayer_set_prebuffer_mode(plusplayer_h handle, bool prebuffer_mode);

/**
 * @brief     Sets application id to resource manager to allocate
 *            hardware resources like decoder.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] app_id : app id
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_open(player,uri);
 *            plusplayer_set_app_id(player,app_id);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_IDLE.
 * @post      The player state will be same as @pre.
 * @exception None
 * @see       plusplayer_start() \n
 */
int plusplayer_set_app_id(plusplayer_h handle, const char* app_id);

/**
 * @brief     Suspend playing media content. Player can be suspended and it will
 * be muted. \n The current playback status will be maintained internally \n App
 * can call suspend() when the app is switching to background.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            prepare player done
 *            // ... your codes ...
 *            plusplayer_suspend(player);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player state must be one of #PLUSPLAYER_STATE_READY or
 * #PLUSPLAYER_STATE_PAUSED or #PLUSPLAYER_STATE_PLAYING.
 * @post      In case of success the player state will be
 * #PLUSPLAYER_STATE_PAUSED otherwise state will be not changed.
 * @remark    In case of power off/on, caller needs to close and open player
 * again.
 * @exception None
 * @see       plusplayer_start() \n
 *            plusplayer_pause() \n
 *            plusplayer_prepare() \n
 *            plusplayer_close()
 */
int plusplayer_suspend(plusplayer_h handle);

/**
 * @brief     Restore the suspended player.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] target_state : State of player after restore. \n
 *              Playback will be started if #PLUSPLAYER_STATE_PLAYING is set.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            prepare player done
 *            // ... your codes ...
 *            plusplayer_suspend(player);
 *            // ... your codes ...
 *            plusplayer_restore(player,PLUSPLAYER_STATE_PLAYING);
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player must be suspended.
 * @post      The player state will be one of #PLUSPLAYER_STATE_PLAYING or
 * #PLUSPLAYER_STATE_PAUSED or #PLUSPLAYER_STATE_READY
 * @exception None
 * @see       plusplayer_suspend() \n
 */
int plusplayer_restore(plusplayer_h handle, plusplayer_state_e target_state);

/**
 * @brief     Get the current playing time of the associated media.
 * @param     [in] handle : plusplayer handle.
 * @param     [out] cur_time_ms : current playing time default in milliseconds
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            prepare player done
 *            plusplayer_start(player);
 *            // ... your codes ...
 *            uint64_t cur_time_ms = 0;
 *            plusplayer_get_playing_time(player, &cur_time_ms);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player must be one of #PLUSPLAYER_STATE_PAUSE or
 *            #PLUSPLAYER_STATE_PLAYING.
 * @post      None
 * @exception None
 * @see       plusplayer_prepare_async()
 */
int plusplayer_get_playing_time(plusplayer_h handle, uint64_t* cur_time_ms);

/**
 * @brief     Set the video display mode.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] mode : display mode.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_open(player,uri);
 *            plusplayer_set_display_mode(player,PLUSPLAYER_DISPLAY_MODE_DST_ROI);
 *            // ... your codes ...
 *            plusplayer_close(player);
 * @endcode
 * @pre       The player state can be all of #plusplayer_state_e except
 *            #PLUSPLAYER_STATE_NONE.
 * @post      None
 * @exception None
 * @see       plusplayer_open() \n
 *            plusplayer_set_display_mode() \n
 *            plusplayer_set_display_roi()
 */
int plusplayer_set_display_mode(plusplayer_h handle,
                                plusplayer_display_mode_e mode);

/**
 * @brief     Set the ROI(Region Of Interest) area of display.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] roi : plusplayer_geometry_s.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_open(player,uri);
 *            plusplayer_set_display(player,plusplayer_display_type_e_OVERLAY,window);
 *            plusplayer_set_display_mode(player,PLUSPLAYER_DISPLAY_MODE_DST_ROI);
 *            plusplayer_geometry_s roi;
 *            roi.x = 0;
 *            roi.y = 0;
 *            roi.w = 600;
 *            roi.h = 500;
 *            plusplayer_set_display_roi(player,roi);
 *            // ... your codes ...
 *            plusplayer_close(player);
 * @endcode
 * @pre       The player state can be all of #plusplayer_state_e except
 *            #PLUSPLAYER_STATE_NONE. \n
 *            Before set display ROI, #PLUSPLAYER_DISPLAY_MODE_DST_ROI
 *            must be set with plusplayer_set_display_mode().
 * @post      None
 * @exception None
 * @remark    The minimum value of width and height are 1.
 * @see       plusplayer_open() \n
 *            plusplayer_set_display() \n
 *            plusplayer_set_display_mode()
 */
int plusplayer_set_display_roi(plusplayer_h handle, plusplayer_geometry_s roi);
/**
 * @brief     Set the rotate angle of the video display.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] rotation : the rotate angle of the display.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_open(player,uri);
 *            plusplayer_set_display(player,plusplayer_display_type_e_OVERLAY,window);
 *            plusplayer_set_display_rotation(player_,plusplayer_display_rotation_type_e_90);
 *            // ... your codes ...
 *            plusplayer_close(player);
 * @endcode
 * @pre       The player state can be all of #plusplayer_state_e except
 *            #PLUSPLAYER_STATE_NONE.
 * @post      The player state will be same as @pre.
 * @remark    The default value is 0.
 * @exception None
 * @see       plusplayer_open() \n
 *            plusplayer_set_display()
 */
int plusplayer_set_display_rotation(
    plusplayer_h handle, plusplayer_display_rotation_type_e rotation);

/**
 * @brief     Set buffer config parameters to buffer media contents
 *            before starting playback.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] config : config of buffer.\n
 *             config can be \n
 *               "total_buffer_size_in_byte" \n
 *               "total_buffer_size_in_time" \n
 *               "buffer_size_in_byte_for_play" \n
 *               "buffer_size_in_sec_for_play" \n
 *               "buffer_size_in_byte_for_resume" \n
 *               "buffer_size_in_sec_for_resume" \n
 *               "buffering_timeout_in_sec_for_play"
 * @param     [in] amount : data amount to be buffered.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @pre       The player state can be all of #plusplayer_state_e except
 * #State::PLUSPLAYER_STATE_NONE
 * @post      The player state will be same as @pre.
 * @exception None
 */
int plusplayer_set_buffer_config(plusplayer_h handle, const char* config,
                                 int amount);

/**
 * @brief     Get the duration of the stream.
 * @param     [in] handle : plusplayer handle.
 * @param     [out] duration_ms : duration in milliseconds.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation
 * failed.
 * @pre       The player state must be one of
 * PLUSPLAYER_STATE_TRACK_SOURCE_READY, PLUSPLAYER_STATE_READY,
 * PLUSPLAYER_STATE_PLAYING or PLUSPLAYER_STATE_PAUSED.
 * @post      The player state will be same as @pre.
 * @exception None
 */
int plusplayer_get_duration(plusplayer_h handle, int64_t* duration_ms);

/**
 * @brief     Set the media playback rate. This api is used to slow/fast
 * playback speed.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] playback_rate :  media playback rate.
 * @remark    Supported playback rates include: -16.0, -8.0, -4.0, -2.0, 0.25,
 * 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 4.0, 8.0, 16.0.
 * @remark    Positive parameter values play the media forwards, while negative
 * values cause the media to play in reverse.
 * @remark    Playback speeds -16x and 16x are not supported for general HTTP
 * and HTTPS streams.
 * @remark    For HLS streaming, trick play is available for content with
 * EXT-X-I-FRAME-STREAM-INF tag, So content must have this in playlist.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_STATE Invalid state
 * @code
 *            prepare player done
 *            // ... your codes ...
 *            plusplayer_set_playback_rate(player,1.5);
 *            // ... your codes ...
 *            plusplayer_stop(player);
 * @endcode
 * @pre       The player state can be all of #plusplayer_state_e except
 *            #PLUSPLAYER_STATE_NONE or #PLUSPLAYER_STATE_IDLE.
 * @post      The player state will be same as @pre.
 * @exception None
 * @see       plusplayer_prepare_async()
 */
int plusplayer_set_playback_rate(plusplayer_h handle,
                                 const double playback_rate);

/**
 * @brief     Deactivate Audio Stream.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *             refer to the sample code of plusplayer_activate_audio()
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre.
 * @exception None
 * @see       plusplayer_activate_audio
 */
int plusplayer_deactivate_audio(plusplayer_h handle);

/**
 * @brief     Activate Audio Stream.
 * @param     [in] handle : plusplayer handle.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            // play one player in normal
 *            plusplayer_h player1 = plusplayer_create();
 *            plusplayer_open(player1,uri_1);
 *            plusplayer_prepare(player1);
 *            plusplayer_start(player1);
 *
 *            // play one more player started in deactivate
 *            plusplayer_h player2 = plusplayer_create();
 *            plusplayer_open(player2,uri_2);
 *            // ... your codes ...
 *            plusplayer_deactivate_audio(player2);
 *            plusplayer_prepare(player2);
 *            plusplayer_start(player2);
 *
 *            // if you want to play player2, deactivate player1 first
 *            // and then activate player2
 *            plusplayer_deactivate_audio(player1);
 *            plusplayer_activate_audio(player2);
 *            // ... your codes ...
 *            plusplayer_close(player1);
 *            plusplayer_destroy(player1);
 *            plusplayer_close(player2);
 *            plusplayer_destroy(player2);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE.
 * @post      The player state will be same as @pre.
 *            plusplayer_activate_audio().
 * @exception None
 * @see       plusplayer_deactivate_audio
 */
int plusplayer_activate_audio(plusplayer_h handle);

/**
 * @brief     Set Property.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] property : property type enum.
 * @param     [in] value : property value in string.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            // Example for setting resolution information for HLS/DASH
 * streaming.
 *
 *            plusplayer_open(player,uri);
 *            // ... your codes ...
 *            plusplayer_set_property(player,
 * PLUSPLAYER_PROPERTY_ADAPTIVE_INFO, "FIXED_MAX_RESOLUTION=7680X4320");
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *
 *
 *            // Example for setting adaptive bitrate information.
 *
 *            plusplayer_open(player,uri);
 *            // ... your codes ...
 *            plusplayer_set_property(player,
 * PLUSPLAYER_PROPERTY_ADAPTIVE_INFO,
 * "BITRATES=5000~10000|STARTBITRATE=HIGHEST|SKIPBITRATE=LOWEST");
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *
 *
 *            // Example for setting Max Bandwidth (Only valid for Dash
 * Streaming).
 *
 *            plusplayer_open(player, dash_uri);
 *            plusplayer_prepare(player);
 *            plusplayer_set_property(player,
 * PLUSPLAYER_PROPERTY_MAX_BANDWIDTH, "100000");
 *            // ... your codes ...
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre.
 * @remark    Valid Properties to Set are [PLUSPLAYER_PROPERTY_ADAPTIVE_INFO,
 * PLUSPLAYER_PROPERTY_LISTEN_SPARSE_TRACK,
 * PLUSPLAYER_PROPERTY_CONFIG_LOW_LATENCY,
 * PLUSPLAYER_PROPERTY_ATSC3_L1_SERVER_TIME,
 * PLUSPLAYER_PROPERTY_AUDIO_DESCRIPTION, PLUSPLAYER_PROPERTY_PRESELECTION_TAG,
 * PLUSPLAYER_PROPERTY_USE_MAIN_OUT_SHARE, PLUSPLAYER_PROPERTY_URL_AUTH_TOKEN,
 * PLUSPLAYER_PROPERTY_USER_LOW_LATENCY, PLUSPLAYER_PROPERTY_OPEN_HTTP_HEADER,
 * PLUSPLAYER_PROPERTY_MAX_BANDWIDTH, PLUSPLAYER_PROPERTY_MPEGH_METADATA,
 * PLUSPLAYER_PROPERTY_OPEN_MANIFEST]
 * @exception None
 */
int plusplayer_set_property(plusplayer_h handle, plusplayer_property_e property,
                            const char* value);

/**
 * @brief     Get Property.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] property : get property type enum.
 * @param     [out] value   : Value of @c property.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_STATE Invalid State
 * @code
 *            // Example for getting string listing of available bit-rates for
 * the currently-playing stream.
 *
 *            plusplayer_prepare(player);
 *            char* value;
 *            plusplayer_get_property(player,
 * PLUSPLAYER_PROPERTY_AVAILABLE_BITRATE, &value);
 *            // ... your codes ...
 *            plusplayer_start(player);
 *            // ... your codes ...
 *            free(value);
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_READY or
 *            #PLUSPLAYER_STATE_PLAYING or #PLUSPLAYER_STATE_PAUSED
 * @post      None
 * @remark    It is caller's responsibility to free @c value memory resources
 * ONLY on api return success.
 * @remark    Valid Properties to Get are
 * [PLUSPLAYER_PROPERTY_AVAILABLE_BITRATE, PLUSPLAYER_PROPERTY_PRESELECTION_TAG,
 * PLUSPLAYER_PROPERTY_CURRENT_LATENCY, PLUSPLAYER_PROPERTY_IS_DVB_DASH,
 * PLUSPLAYER_PROPERTY_LIVE_PLAYER_START, PLUSPLAYER_PROPERTY_HTTP_HEADER,
 * PLUSPLAYER_PROPERTY_START_DATE,
 * PLUSPLAYER_PROPERTY_MPEGH_METADATA, PLUSPLAYER_PROPERTY_DASH_STREAM_INFO]
 * @exception None
 */
int plusplayer_get_property(plusplayer_h handle, plusplayer_property_e property,
                            char** value);

/**
 * @brief     Get track count of a track type.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] track_type : track type Audio, Video or Subtitle.
 * @param     [out] count : track count.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            int track_count = 0;
 *            plusplayer_get_track_count(player,PLUSPLAYER_TRACK_TYPE_AUDIO,&track_count);
 *            // ... your codes ...
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player can be one of PLUSPLAYER_STATE_TRACK_SOURCE_READY,
 * PLUSPLAYER_STATE_READY, PLUSPLAYER_STATE_PLAYING or PLUSPLAYER_STATE_PAUSED
 * @post      The player state is same as before calling
 * @exception None
 */
int plusplayer_get_track_count(plusplayer_h handle,
                               plusplayer_track_type_e track_type, int* count);

/**
 * @brief     Select track to be played.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] type : track type (only supports #PLUSPLAYER_TRACK_TYPE_AUDIO
 * and #PLUSPLAYER_TRACK_TYPE_VIDEO)
 * @param     [in] index : index of track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            plusplayer_select_track(player_, PLUSPLAYER_TRACK_TYPE_AUDIO, 0);
 *            // ... your codes ...
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player can be one of PLUSPLAYER_STATE_READY,
 * PLUSPLAYER_STATE_PLAYING or PLUSPLAYER_STATE_PAUSED
 * @post      The player state will be same as @pre.
 * @exception None
 */
int plusplayer_select_track(plusplayer_h handle, plusplayer_track_type_e type,
                            int index);

/**
 * @brief     Get language code of the selected track.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] type : track type.
 * @param     [in] index : index of track.
 * @return    @c Pointer to heap allocated char array on success, @c NULL
 * otherwise
 * @retval    String value of property type
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            auto lang_code =
 * plusplayer_get_track_language_code(player_,PLUSPLAYER_TRACK_TYPE_AUDIO,1);
 *            // ... your codes ...
 *            plusplayer_start(player);
 *            free(lang_code);
 * @endcode
 * @pre       The player can be one of PLUSPLAYER_STATE_READY,
 * PLUSPLAYER_STATE_PLAYING or PLUSPLAYER_STATE_PAUSED
 * @post      The player state will be same as @pre.
 * @remark    It is caller's responsibility to free the returned pointer memory
 * resources.
 * @exception None
 */
const char* plusplayer_get_track_language_code(plusplayer_h handle,
                                               plusplayer_track_type_e type,
                                               int index);

/**
 * @brief     Set Application Information like id, version and type to
 * plusplayer.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] app_info : plusplayer_app_info_s struct.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            std::string id , version , type.
 *            // ... your codes ...
 *            plusplayer_app_info_s app_info;
 *            app_info.id = id;
 *            app_info.version = version;
 *            app_info.type = type;
 *            plusplayer_set_app_info(player,app_info);
 *            // ... your codes ...
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player must be PLUSPLAYER_STATE_IDLE.
 * @post      The player state will be same as @pre.
 * @exception None
 */
int plusplayer_set_app_info(plusplayer_h handle,
                            const plusplayer_app_info_s* app_info);

/**
 * @brief     Set DRM Property.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] drm_property : DRM Property
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            plusplayer_drm_property_s drm_property;
 *            plusplayer_set_drm(player,drm_property);
 *            // ... your codes ...
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state can be #PLUSPLAYER_STATE_IDLE.
 * @post      The player state will be same as @pre
 * @exception None
 */
int plusplayer_set_drm(plusplayer_h handle,
                       plusplayer_drm_property_s drm_property);

/**
 * @brief     Set a callback function to be invoked when drm init data is set.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] drm_init_data_callback : the drm init callback function to
 * register.
 * @param     [in] userdata : pointer to caller object of
 * plusplayer_set_drm_init_data_cb()
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_set_drm_init_data_cb() will be invoked.
 *            if drm_init_data_cb is set to null,
 * plusplayer_set_drm_init_data_cb() will not be invoked anymore.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_drm_init_data_cb callback
 */
int plusplayer_set_drm_init_data_cb(
    plusplayer_h handle, plusplayer_drm_init_data_cb drm_init_data_callback,
    void* userdata);

/**
 * @brief     Set a callback function to  be invoked when streamingengine
 * invokes adaptive streaming control event.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] adaptive_streaming_control_event_cb : the adative streaming
 * event callback function to register.
 * @param     [in] userdata : pointer to caller object of
 * plusplayer_set_drm_init_data_cb()
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @pre       The player state must be at least #PLUSPLAYER_STATE_NONE. \n
 *            But recommended to be called in either #PLUSPLAYER_STATE_NONE or
 * #PLUSPLAYER_STATE_IDLE.
 * @post      plusplayer_set_adaptive_streaming_control_event_cb() will be
 * invoked. if adaptive_streaming_control_event_cb is set to null,
 * adaptive_streaming_control_event_cb() will not be invoked anymore.
 * @exception It is prohibited to call any player APIs within context of
 * plusplayer_adaptive_streaming_control_event_cb callback
 * @remark    @c data inside plusplayer_message_param_s struct is allocated
 * using malloc operator. It is caller's responsibility to free the returned
 * pointer memory resources using @ free operator.
 */
int plusplayer_set_adaptive_streaming_control_event_cb(
    plusplayer_h handle,
    plusplayer_adaptive_streaming_control_event_cb
        adaptive_streaming_control_event_cb,
    void* userdata);

/**
 * @brief     Notify to player DRM license is acquired for a specified track
 * type.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] track_type : @c for which track is that drm license was
 * acquired.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @pre       The player state must be one of
 * #PLUSPLAYER_STATE_TRACK_SOURCE_READY or #PLUSPLAYER_STATE_READY,
 * @post      The player state will be same as @pre
 * @exception None
 */
int plusplayer_drm_license_acquired_done(plusplayer_h handle,
                                         plusplayer_track_type_e track_type);

/**
 * @brief     Set External Subtitle Path.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] uri : external subtitle path uri.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,content_uri);
 *            plusplayer_set_subtitle(player,subtitle_uri);
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre
 * @exception None
 */
int plusplayer_set_subtitle_path(plusplayer_h handle, const char* uri);

/**
 * @brief     Set Video Stillmode.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] stillmode : enum for stillmode.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_set_video_stillmode(player,PLUSPLAYER_STILL_MODE_ON);
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre
 * @exception None
 */
int plusplayer_set_video_stillmode(plusplayer_h handle,
                                   plusplayer_still_mode_e stillmode);

/**
 * @brief     Set Alternate Video Resource(Sub decoder and Sub Scaler).
 * @param     [in] handle : plusplayer handle.
 * @param     [in] rsc_type : set alternative video resource.
 *            (@c 0 [default] = set all video resources(decoder/scaler) to
 *                              main resources,
 *             @c 1 = set all video resources(decoder/scaler) to sub
 *                    resources,
 *             @c 2 = set only decoder to sub resource,
 *             @c 3 = set only scaler to sub resource)
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_set_alternate_video_resource(player,1);
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre
 * @remark    Using sub resource (either decoder or scaler) is subject to
 * availability on platform. \n No error is returned if unable to set resource.
 * @exception None
 */
int plusplayer_set_alternative_video_resource(plusplayer_h handle,
                                              unsigned int rsc_type);

/**
 * @brief     Get track info.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] track_cb : plusplayer track callback function to register.
 * @param     [in] userdata : userdata of caller.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *      bool printTrackInfo(const plusplayer_track_h track, void* userdata) {
 *          int index, id;
 *          const char* mimetype;
 *          plusplayer_get_track_index(track, &index);
 *          plusplayer_get_track_id(track, &id);
 *          plusplayer_get_track_mimetype(track, &mimetype);
 *          // ... your codes ...
 *          return true;
 *      }
 *
 *      // ... your codes ...
 *      plusplayer_prepare(player_);
 *      plusplayer_get_foreach_track(player_, printTrackInfo, nullptr);
 *
 * @endcode
 * @pre       The player can be one of PLUSPLAYER_STATE_TRACK_SOURCE_READY,
 * PLUSPLAYER_STATE_READY, PLUSPLAYER_STATE_PLAYING or PLUSPLAYER_STATE_PAUSED
 * @post      The player state is same as before calling
 * @remark    The caller will receive track information through track_cb.
 * Further caller can use get api's for track info members.
 * @exception None
 * @see       plusplayer_get_track_index() \n
 *            plusplayer_get_track_id() \n
 *            plusplayer_get_track_type() \n
 *            plusplayer_get_track_bitrate()
 */
int plusplayer_get_foreach_track(plusplayer_h handle,
                                 plusplayer_track_cb track_cb, void* userdata);

/**
 * @brief     Get Active track info.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] track_cb : plusplayer track callback function to register.
 * @param     [in] userdata : userdata of caller.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *       bool getTrackAudio(const plusplayer_track_h track, void* user_data) {
 *           plusplayer_track_h* found_track =
 * (plusplayer_track_h*)user_data;
 *           plusplayer_track_type_e track_type;
 *
 *           if (plusplayer_get_track_type(track, &track_type) !=
 * PLUSPLAYER_ERROR_TYPE_NONE)
 *           {
 *              return false;
 *           }
 *           if (track_type == PLUSPLAYER_TRACK_TYPE_AUDIO) {
 *               *found_track = track;
 *               return false;
 *           }
 *           return true;
 *       }
 *
 *       bool GetActiveTrack(plusplayer_h player, const
 * plusplayer_track_type_e type, plusplayer_track_h* active_track) {
 *          plusplayer_track_h found_track = nullptr;
 *          plusplayer_get_foreach_active_track(player, getTrackAudio,
 * &found_track);
 *          if (found_track == nullptr) {
 *              return false;
 *          }
 *          *active_track = found_track;
 *          return true;
 *       }
 * @endcode
 * @pre       The player can be one of PLUSPLAYER_STATE_TRACK_SOURCE_READY,
 * PLUSPLAYER_STATE_READY, PLUSPLAYER_STATE_PLAYING or PLUSPLAYER_STATE_PAUSED
 * @post      The player state is same as before calling
 * @remark    The caller will receive track information through track_cb.
 * Further caller can use get api's for track info members.
 * @exception None
 * @see       plusplayer_get_track_index() \n
 *            plusplayer_get_track_id() \n
 *            plusplayer_get_track_type() \n
 *            plusplayer_get_track_bitrate()
 */
int plusplayer_get_foreach_active_track(plusplayer_h handle,
                                        plusplayer_track_cb track_cb,
                                        void* userdata);

/**
 * @brief     Set HTTP Request Cookie.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] cookie : String containing HTTP request cookie.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_set_cookie(player, );
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre
 * @exception None
 */
int plusplayer_set_cookie(plusplayer_h handle, const char* cookie);

/**
 * @brief     Set HTTP User Agent.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] user_agent : String containing HTTP user agent.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_set_user_agent(player,user_agent);
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre
 * @exception None
 */
int plusplayer_set_user_agent(plusplayer_h handle, const char* user_agent);

/**
 * @brief     Set resume time to start the content playback.
 * @param     [in] handle : plusplayer handle.
 * @param     [in] resume_time_ms : value to start the content playback at
 * resume time.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_set_resume_time(player, resume_time_ms);
 *            // ... your codes ...
 *            plusplayer_prepare(player);
 *            plusplayer_start(player);
 * @endcode
 * @pre       The player state must be at least #PLUSPLAYER_STATE_IDLE
 * @post      The player state will be same as @pre
 * @remark    For general non-adaptive streaming content, this api will return
 * error @c PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION
 * @exception None
 */
int plusplayer_set_resume_time(plusplayer_h handle, uint64_t resume_time_ms);

/**
 * @brief     To check whether stream is live.
 * @param     [in] handle : plusplayer handle.
 * @param     [out] is_live : live streaming or not @c true = live streaming
 * @c false = VOD
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_STATE Invalid state
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            plusplayer_is_live_streaming(player, &is_live);
 *            // ... your codes ...
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_READY or
 *            #PLUSPLAYER_STATE_PLAYING or #PLUSPLAYER_STATE_PAUSED
 * @post      The player state will be same as @pre
 * @remark    For general non-adaptive streaming content, this api will return
 * error @c PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION
 * @exception None
 */
int plusplayer_is_live_streaming(plusplayer_h handle, bool* is_live);

/**
 * @brief     Get live stream duration of current DVR window.
 * @param     [in] handle : plusplayer handle.
 * @param     [out] start_time_ms : start time in milliseconds.
 * @param     [out] end_time_ms : end time in milliseconds.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_STATE Invalid state
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            plusplayer_get_dvr_seekable_range(player, &start_time_ms,
 * &end_time_ms);
 *            // ... your codes ...
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_READY or
 *            #PLUSPLAYER_STATE_PLAYING or #PLUSPLAYER_STATE_PAUSED
 * @post      The player state will be same as @pre
 * @remark    Live streams do not have fixed duration. But server generally
 * maintains a finite buffer (called DVR) of live encoded data, whose right edge
 * @c end_time_ms indicates live point and @c start_time_ms indicates oldest
 * segment buffered by the server. DVR is sliding rightward with time to keep
 * with live point.
 * @remark    For general non-adaptive streaming content, this api will return
 * error @c PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION
 * @exception None
 */
int plusplayer_get_dvr_seekable_range(plusplayer_h handle,
                                      uint64_t* start_time_ms,
                                      uint64_t* end_time_ms);

/**
 * @brief     Get the current streaming bandwidth.
 * @param     [in] handle : plusplayer handle.
 * @param     [out] curr_bandwidth_bps : value of current bandwidth in bits/s
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type_e values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Internal operation failed
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_STATE Invalid state
 * @code
 *            plusplayer_h player = plusplayer_create();
 *            plusplayer_open(player,uri);
 *            plusplayer_prepare(player);
 *            // ... your codes ...
 *            plusplayer_get_current_bandwidth(player, &curr_bandwidth_bps);
 *            // ... your codes ...
 * @endcode
 * @pre       The player state must be #PLUSPLAYER_STATE_READY or
 *            #PLUSPLAYER_STATE_PLAYING or #PLUSPLAYER_STATE_PAUSED
 * @post      The player state will be same as @pre
 * @remark    For general non-adaptive streaming content, this api will return
 * error @c PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION
 * @exception None
 */
int plusplayer_get_current_bandwidth(plusplayer_h handle,
                                     uint32_t* curr_bandwidth_bps);

#ifdef __cplusplus
}
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_PLUSPLAYER_CAPI_H__
