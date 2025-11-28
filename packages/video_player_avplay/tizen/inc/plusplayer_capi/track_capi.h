/**
 * @file           track_capi.h
 * @brief          PlusPlayer Track apis c version
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is plusplayer track apis header implemented as C style
 * to avoid binary compatibility issues.
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

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_PLUSPLAYER_TRACK_CAPI_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_PLUSPLAYER_TRACK_CAPI_H__

#include "plusplayer_capi/track.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Handle for PlusPlayer Track
 */
typedef void* plusplayer_track_h;

/**
 * @brief     Get the index of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_index : index of track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_index;
 *            plusplayer_get_track_index(track, &track_index);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_index(plusplayer_track_h track, int* track_index);

/**
 * @brief     Get the ID of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_id : id of track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_id;
 *            plusplayer_get_track_id(track, &track_id);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_id(plusplayer_track_h track, int* track_id);

/**
 * @brief     Get the MIME type of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_mimetype : mimetype of track. (e.g "video/x-h264",
 * "audio/mpeg")
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            const char* track_mimetype;
 *            plusplayer_get_track_mimetype(track, &track_mimetype);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @remark    Caller MUST NOT attempt to free track_mimetype resource.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_mimetype(plusplayer_track_h track,
                                  const char** track_mimetype);

/**
 * @brief     Get the stream type of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_streamtype : streamtype of track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            const char* track_streamtype;
 *            plusplayer_get_track_streamtype(track, &track_streamtype);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @remark    Caller MUST NOT attempt to free track_streamtype resource.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_streamtype(plusplayer_track_h track,
                                    const char** track_streamtype);

/**
 * @brief     Get the media container type of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_containertype : container type of track. (e.g
 * "tsdemux", "dash_mov")
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            const char* track_containertype;
 *            plusplayer_get_track_container_type(track, &track_containertype);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @remark    Caller MUST NOT attempt to free track_containertype resource.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_container_type(plusplayer_track_h track,
                                        const char** track_containertype);

/**
 * @brief     Get the type of the track (audio/video/subtitle).
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_type : type of the track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            plusplayer_track_type_e track_type;
 *            plusplayer_get_track_type(track, &track_type);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_type(plusplayer_track_h track,
                              plusplayer_track_type_e* track_type);

/**
 * @brief     Get the codec data of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_codecdata : codec data of the track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION Data is invalid.
 * @code
 *            plusplayer_track_h track = ...;
 *            const char* track_codecdata;
 *            plusplayer_get_track_codec_data(track, &track_codecdata);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @remark    Caller MUST NOT attempt to free track_codecdata resource.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_codec_data(plusplayer_track_h track,
                                    const char** track_codecdata);

/**
 * @brief     Get the codec tag of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_codectag : codec tag of the track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            unsigned int track_codectag;
 *            plusplayer_get_track_codec_tag(track, &track_codectag);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_codec_tag(plusplayer_track_h track,
                                   unsigned int* track_codectag);

/**
 * @brief     Get the length of the codec data of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_codecdatalen : length of the codec data of the track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_codecdatalen;
 *            plusplayer_get_track_codec_data_len(track, &track_codecdatalen);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_codec_data_len(plusplayer_track_h track,
                                        int* track_codecdatalen);

/**
 * @brief     Get the width of video resolution.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_width : width of video track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_width;
 *            plusplayer_get_track_width(track, &track_width);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_width(plusplayer_track_h track, int* track_width);

/**
 * @brief     Get the height of video resolution.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_height : height of video track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_height;
 *            plusplayer_get_track_height(track, &track_height);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_height(plusplayer_track_h track, int* track_height);

/**
 * @brief     Get the maximum width of video in adaptive stream
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_maxwidth : maximum width of video track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_maxwidth;
 *            plusplayer_get_track_maxwidth(track, &track_maxwidth);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_maxwidth(plusplayer_track_h track,
                                  int* track_maxwidth);

/**
 * @brief     Get the maximum height of video in adaptive stream.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_maxheight : maximum height of video track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_maxheight;
 *            plusplayer_get_track_maxheight(track, &track_maxheight);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_maxheight(plusplayer_track_h track,
                                   int* track_maxheight);

/**
 * @brief     Get the numerator of the frame rate of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_framerate_num : framerate numerator.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_framerate_num;
 *            plusplayer_get_track_framerate_num(track, &track_framerate_num);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_framerate_num(plusplayer_track_h track,
                                       int* track_framerate_num);

/**
 * @brief     Get the denominator of the frame rate of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_framerate_den : framerate denumerator.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_framerate_den;
 *            plusplayer_get_track_framerate_den(track, &track_framerate_den);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_framerate_den(plusplayer_track_h track,
                                       int* track_framerate_den);

/**
 * @brief     Get the sample rate for audio track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_sample_rate : sample rate for audio track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_sample_rate;
 *            plusplayer_get_track_sample_rate(track, &track_sample_rate);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_sample_rate(plusplayer_track_h track,
                                     int* track_sample_rate);

/**
 * @brief     Get the sample format for audio track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_sample_format : sample format for audio track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_sample_format;
 *            plusplayer_get_track_sample_format(track, &track_sample_format);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_sample_format(plusplayer_track_h track,
                                       int* track_sample_format);

/**
 * @brief     Get the number of channels of audio track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_channels : number of channels of audio track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_channels;
 *            plusplayer_get_track_channels(track, &track_channels);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_channels(plusplayer_track_h track,
                                  int* track_channels);

/**
 * @brief     Get the mpeg version of audio codec.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_version : mpeg version of audio codec.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_version;
 *            plusplayer_get_track_version(track, &track_version);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_version(plusplayer_track_h track, int* track_version);

/**
 * @brief     Get the mpeg layer of audio codec.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_layer : mpeg layer of audio codec.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_layer;
 *            plusplayer_get_track_layer(track, &track_layer);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_layer(plusplayer_track_h track, int* track_layer);

/**
 * @brief     Get the bits per sample of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_bits_per_sample : bits per sample of the track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_bits_per_sample;
 *            plusplayer_get_track_bits_per_sample(track,
 * &track_bits_per_sample);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_bits_per_sample(plusplayer_track_h track,
                                         int* track_bits_per_sample);

/**
 * @brief     Get the data block alignment of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_block_align : data block alignment.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_block_align;
 *            plusplayer_get_track_block_align(track, &track_block_align);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_block_align(plusplayer_track_h track,
                                     int* track_block_align);

/**
 * @brief     Get the data bitrate of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_bitrate : data bitrate of the track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_bitrate;
 *            plusplayer_get_track_bitrate(track, &track_bitrate);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_bitrate(plusplayer_track_h track, int* track_bitrate);

/**
 * @brief     Get the endianness of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_endianness : endianness of the track. (e.g little
 * endian : 1234)
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            int track_endianness;
 *            plusplayer_get_track_endianness(track, &track_endianness);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_endianness(plusplayer_track_h track,
                                    int* track_endianness);

/**
 * @brief     Check if this track is signed or not.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_is_signed : @c True if track is signed, @c False
 * otherwise.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            bool track_is_signed;
 *            plusplayer_get_track_is_signed(track, &track_is_signed);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_is_signed(plusplayer_track_h track,
                                   bool* track_is_signed);

/**
 * @brief     Check if the track is active.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_active : @c True if track is active, @c False
 * otherwise.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            bool track_active;
 *            plusplayer_get_track_active(track, &track_active);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_active(plusplayer_track_h track, bool* track_active);

/**
 * @brief     Get the language code of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_lang_code : language code of track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            const char* track_lang_code;
 *            plusplayer_get_track_lang_code(track, &track_lang_code);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @remark    Caller MUST NOT attempt to free track_lang_code resource.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_lang_code(plusplayer_track_h track,
                                   const char** track_lang_code);

/**
 * @brief     Get the subtitle format of the track.
 * @param     [in] track : Handle to the track info.
 * @param     [out] track_subtitle_format : subtitle format of track.
 * @return    @c PLUSPLAYER_ERROR_TYPE_NONE on success,otherwise @c one of
 *            plusplayer_error_type values will be returned.
 * @retval    #PLUSPLAYER_ERROR_TYPE_NONE Successful
 * @retval    #PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER Invalid parameter
 * @code
 *            plusplayer_track_h track = ...;
 *            const char* track_subtitle_format;
 *            plusplayer_get_track_subtitle_format(track,
 * &track_subtitle_format);
 *            // ... your codes ...
 * @endcode
 * @pre       The track handle must be valid.
 * @post      The track info remains unchanged.
 * @remark    Caller MUST NOT attempt to free track_subtitle_format resource.
 * @exception None
 * @see       plusplayer_get_foreach_track() \n
 *            plusplayer_get_foreach_active_track()
 */
int plusplayer_get_track_subtitle_format(plusplayer_track_h track,
                                         const char** track_subtitle_format);

#ifdef __cplusplus
}
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_PLUSPLAYER_TRACK_CAPI_H__