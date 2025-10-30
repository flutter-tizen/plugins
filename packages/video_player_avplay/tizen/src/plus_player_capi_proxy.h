// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PLUS_PLAYER_CAPI_PROXY_H_
#define FLUTTER_PLUGIN_PLUS_PLAYER_CAPI_PROXY_H_

#include "plusplayer_capi/plusplayer_capi.h"

typedef plusplayer_h (*FunPlusplayerCapiCreate)(void);
typedef int (*FunPlusplayerCapiOpen)(plusplayer_h handle, const char* uri);
typedef int (*FunPlusplayerCapiPrepare)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiStart)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiStop)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiClose)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiDestroy)(plusplayer_h handle);
typedef plusplayer_state_e (*FunPlusplayerCapiGetState)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiSetDisplay)(plusplayer_h handle,
                                           plusplayer_display_type_e type,
                                           void* window);
typedef int (*FunPlusplayerCapiSetDisplaySubsurface)(
    plusplayer_h handle, plusplayer_display_type_e type, uint32_t surface_id,
    plusplayer_geometry_s roi);
typedef int (*FunPlusplayerCapiSetPrepareAsyncDoneCb)(
    plusplayer_h handle, plusplayer_prepare_async_done_cb prepare_async_done_cb,
    void* userdata);
typedef int (*FunPlusplayerCapiSetResourceConflictedCb)(
    plusplayer_h handle,
    plusplayer_resource_conflicted_cb resource_conflicted_cb, void* userdata);
typedef int (*FunPlusplayerCapiSetEosCb)(plusplayer_h handle,
                                         plusplayer_eos_cb eos_cb,
                                         void* userdata);
typedef int (*FunPlusplayerCapiSetBufferStatusCb)(
    plusplayer_h handle, plusplayer_buffer_status_cb buffer_status_cb,
    void* userdata);
typedef int (*FunPlusplayerCapiSetErrorCb)(plusplayer_h handle,
                                           plusplayer_error_cb error_cb,
                                           void* userdata);
typedef int (*FunPlusplayerCapiSetErrorMsgCb)(
    plusplayer_h handle, plusplayer_error_msg_cb error_msg_cb, void* userdata);
typedef int (*FunPlusplayerCapiSetSeekDoneCb)(
    plusplayer_h handle, plusplayer_seek_done_cb seek_done_cb, void* userdata);
typedef int (*FunPlusplayerCapiSetSubtitleUpdatedCb)(
    plusplayer_h handle, plusplayer_subtitle_updated_cb subtitle_updated_cb,
    void* userdata);
typedef int (*FunPlusplayerCapiSetAdEventCb)(plusplayer_h handle,
                                             plusplayer_ad_event_cb ad_event_cb,
                                             void* userdata);
typedef int (*FunPlusplayerCapiPrepareAsync)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiPause)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiResume)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiSeek)(plusplayer_h handle, uint64_t time);
typedef void (*FunPlusplayerCapiSetPrebufferMode)(plusplayer_h handle,
                                                  bool prebuffer_mode);
typedef int (*FunPlusplayerCapiSetAppId)(plusplayer_h handle,
                                         const char* app_id);
typedef int (*FunPlusplayerCapiSuspend)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiRestore)(plusplayer_h handle,
                                        plusplayer_state_e target_state);
typedef int (*FunPlusplayerCapiGetPlayingTime)(plusplayer_h handle,
                                               uint64_t* cur_time_ms);
typedef int (*FunPlusplayerCapiSetDisplayMode)(plusplayer_h handle,
                                               plusplayer_display_mode_e mode);
typedef int (*FunPlusplayerCapiSetDisplayRoi)(plusplayer_h handle,
                                              plusplayer_geometry_s roi);
typedef int (*FunPlusplayerCapiSetDisplayRotation)(
    plusplayer_h handle, plusplayer_display_rotation_type_e rotation);
typedef int (*FunPlusplayerCapiSetBufferConfig)(plusplayer_h handle,
                                                const char* config, int amount);
typedef int (*FunPlusplayerCapiGetDuration)(plusplayer_h handle,
                                            int64_t* duration_ms);
typedef int (*FunPlusplayerCapiSetPlaybackRate)(plusplayer_h handle,
                                                const double playback_rate);
typedef int (*FunPlusplayerCapiDeactivateAudio)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiActivateAudio)(plusplayer_h handle);
typedef int (*FunPlusplayerCapiSetProperty)(plusplayer_h handle,
                                            plusplayer_property_e property,
                                            const char* value);
typedef int (*FunPlusplayerCapiGetProperty)(plusplayer_h handle,
                                            plusplayer_property_e property,
                                            char** value);
typedef int (*FunPlusplayerCapiGetCurrentBandwidth)(
    plusplayer_h handle, uint32_t* curr_bandwidth_bps);
typedef int (*FunPlusplayerCapiGetTrackCount)(
    plusplayer_h handle, plusplayer_track_type_e track_type, int* count);
typedef int (*FunPlusplayerCapiSelectTrack)(plusplayer_h handle,
                                            plusplayer_track_type_e type,
                                            int index);
typedef const char* (*FunPlusplayerCapiGetTrackLanguageCode)(
    plusplayer_h handle, plusplayer_track_type_e type, int index);
typedef int (*FunPlusplayerCapiSetAppInfo)(
    plusplayer_h handle, const plusplayer_app_info_s* app_info);
typedef int (*FunPlusplayerCapiSetDrm)(plusplayer_h handle,
                                       plusplayer_drm_property_s drm_property);
typedef int (*FunPlusplayerCapiSetDrmInitDataCb)(
    plusplayer_h handle, plusplayer_drm_init_data_cb drm_init_data_callback,
    void* userdata);
typedef int (*FunPlusplayerCapiSetAdaptiveStreamingControlEventCb)(
    plusplayer_h handle,
    plusplayer_adaptive_streaming_control_event_cb
        adaptive_streaming_control_event_cb,
    void* userdata);
typedef int (*FunPlusplayerCapiDrmLicenseAcquiredDone)(
    plusplayer_h handle, plusplayer_track_type_e track_type);
typedef int (*FunPlusplayerCapiSetSubtitlePath)(plusplayer_h handle,
                                                const char* uri);
typedef int (*FunPlusplayerCapiSetVideoStillmode)(
    plusplayer_h handle, plusplayer_still_mode_e stillmode);
typedef int (*FunPlusplayerCapiSetAlternativeVideoResource)(
    plusplayer_h handle, unsigned int rsc_type);
typedef int (*FunPlusplayerCapiGetForeachTrack)(plusplayer_h handle,
                                                plusplayer_track_cb track_cb,
                                                void* userdata);
typedef int (*FunPlusplayerCapiGetForeachActiveTrack)(
    plusplayer_h handle, plusplayer_track_cb track_cb, void* userdata);
typedef int (*FunPlusplayerCapiSetCookie)(plusplayer_h handle,
                                          const char* cookie);
typedef int (*FunPlusplayerCapiSetUserAgent)(plusplayer_h handle,
                                             const char* user_agent);
typedef int (*FunPlusplayerCapiSetResumeTime)(plusplayer_h handle,
                                              uint64_t resume_time_ms);
typedef int (*FunPlusplayerCapiIsLiveStreaming)(plusplayer_h handle,
                                                bool* is_live);
typedef int (*FunPlusplayerCapiGetDvrSeekableRange)(plusplayer_h handle,
                                                    uint64_t* start_time_ms,
                                                    uint64_t* end_time_ms);
typedef int (*FunPlusplayerCapiGetTrackIndex)(plusplayer_track_h track,
                                              int* track_index);
typedef int (*FunPlusplayerCapiGetTrackId)(plusplayer_track_h track,
                                           int* track_id);
typedef int (*FunPlusplayerCapiGetTrackMimetype)(plusplayer_track_h track,
                                                 const char** track_mimetype);
typedef int (*FunPlusplayerCapiGetTrackStreamtype)(
    plusplayer_track_h track, const char** track_streamtype);
typedef int (*FunPlusplayerCapiGetTrackContainerType)(
    plusplayer_track_h track, const char** track_containertype);
typedef int (*FunPlusplayerCapiGetTrackType)(
    plusplayer_track_h track, plusplayer_track_type_e* track_type);
typedef int (*FunPlusplayerCapiGetTrackCodecData)(plusplayer_track_h track,
                                                  const char** track_codecdata);
typedef int (*FunPlusplayerCapiGetTrackCodecTag)(plusplayer_track_h track,
                                                 unsigned int* track_codectag);
typedef int (*FunPlusplayerCapiGetTrackCodecDataLen)(plusplayer_track_h track,
                                                     int* track_codecdatalen);
typedef int (*FunPlusplayerCapiGetTrackWidth)(plusplayer_track_h track,
                                              int* track_width);
typedef int (*FunPlusplayerCapiGetTrackHeight)(plusplayer_track_h track,
                                               int* track_height);
typedef int (*FunPlusplayerCapiGetTrackMaxwidth)(plusplayer_track_h track,
                                                 int* track_maxwidth);
typedef int (*FunPlusplayerCapiGetTrackMaxheight)(plusplayer_track_h track,
                                                  int* track_maxheight);
typedef int (*FunPlusplayerCapiGetTrackFramerateNum)(plusplayer_track_h track,
                                                     int* track_framerate_num);
typedef int (*FunPlusplayerCapiGetTrackFramerateDen)(plusplayer_track_h track,
                                                     int* track_framerate_den);
typedef int (*FunPlusplayerCapiGetTrackSampleRate)(plusplayer_track_h track,
                                                   int* track_sample_rate);
typedef int (*FunPlusplayerCapiGetTrackSampleFormat)(plusplayer_track_h track,
                                                     int* track_sample_format);
typedef int (*FunPlusplayerCapiGetTrackChannels)(plusplayer_track_h track,
                                                 int* track_channels);
typedef int (*FunPlusplayerCapiGetTrackVersion)(plusplayer_track_h track,
                                                int* track_version);
typedef int (*FunPlusplayerCapiGetTrackLayer)(plusplayer_track_h track,
                                              int* track_layer);
typedef int (*FunPlusplayerCapiGetTrackBitsPerSample)(
    plusplayer_track_h track, int* track_bits_per_sample);
typedef int (*FunPlusplayerCapiGetTrackBlockAlign)(plusplayer_track_h track,
                                                   int* track_block_align);
typedef int (*FunPlusplayerCapiGetTrackBitrate)(plusplayer_track_h track,
                                                int* track_bitrate);
typedef int (*FunPlusplayerCapiGetTrackEndianness)(plusplayer_track_h track,
                                                   int* track_endianness);
typedef int (*FunPlusplayerCapiGetTrackIsSigned)(plusplayer_track_h track,
                                                 bool* track_is_signed);
typedef int (*FunPlusplayerCapiGetTrackActive)(plusplayer_track_h track,
                                               bool* track_active);
typedef int (*FunPlusplayerCapiGetTrackLangCode)(plusplayer_track_h track,
                                                 const char** track_lang_code);
typedef int (*FunPlusplayerCapiGetTrackSubtitleFormat)(
    plusplayer_track_h track, const char** track_subtitle_format);

class PlusPlayerCapiProxy {
 public:
  PlusPlayerCapiProxy();
  ~PlusPlayerCapiProxy();
  plusplayer_h plusplayer_capi_create(void);
  int plusplayer_capi_open(plusplayer_h handle, const char* uri);
  int plusplayer_capi_prepare(plusplayer_h handle);
  int plusplayer_capi_start(plusplayer_h handle);
  int plusplayer_capi_stop(plusplayer_h handle);
  int plusplayer_capi_close(plusplayer_h handle);
  int plusplayer_capi_destroy(plusplayer_h handle);
  plusplayer_state_e plusplayer_capi_get_state(plusplayer_h handle);
  int plusplayer_capi_set_display(plusplayer_h handle,
                                  plusplayer_display_type_e type, void* window);
  int plusplayer_capi_set_display_subsurface(plusplayer_h handle,
                                             plusplayer_display_type_e type,
                                             uint32_t surface_id,
                                             plusplayer_geometry_s roi);
  int plusplayer_capi_set_prepare_async_done_cb(
      plusplayer_h handle,
      plusplayer_prepare_async_done_cb prepare_async_done_cb, void* userdata);
  int plusplayer_capi_set_resource_conflicted_cb(
      plusplayer_h handle,
      plusplayer_resource_conflicted_cb resource_conflicted_cb, void* userdata);
  int plusplayer_capi_set_eos_cb(plusplayer_h handle, plusplayer_eos_cb eos_cb,
                                 void* userdata);
  int plusplayer_capi_set_buffer_status_cb(
      plusplayer_h handle, plusplayer_buffer_status_cb buffer_status_cb,
      void* userdata);
  int plusplayer_capi_set_error_cb(plusplayer_h handle,
                                   plusplayer_error_cb error_cb,
                                   void* userdata);
  int plusplayer_capi_set_error_msg_cb(plusplayer_h handle,
                                       plusplayer_error_msg_cb error_msg_cb,
                                       void* userdata);
  int plusplayer_capi_set_seek_done_cb(plusplayer_h handle,
                                       plusplayer_seek_done_cb seek_done_cb,
                                       void* userdata);

  int plusplayer_capi_set_subtitle_updated_cb(
      plusplayer_h handle, plusplayer_subtitle_updated_cb subtitle_updated_cb,
      void* userdata);
  int plusplayer_capi_set_ad_event_cb(plusplayer_h handle,
                                      plusplayer_ad_event_cb ad_event_cb,
                                      void* userdata);
  int plusplayer_capi_prepare_async(plusplayer_h handle);
  int plusplayer_capi_pause(plusplayer_h handle);
  int plusplayer_capi_resume(plusplayer_h handle);
  int plusplayer_capi_seek(plusplayer_h handle, uint64_t time);
  void plusplayer_capi_set_prebuffer_mode(plusplayer_h handle,
                                          bool prebuffer_mode);
  int plusplayer_capi_set_app_id(plusplayer_h handle, const char* app_id);
  int plusplayer_capi_suspend(plusplayer_h handle);
  int plusplayer_capi_restore(plusplayer_h handle,
                              plusplayer_state_e target_state);
  int plusplayer_capi_get_playing_time(plusplayer_h handle,
                                       uint64_t* cur_time_ms);
  int plusplayer_capi_set_display_mode(plusplayer_h handle,
                                       plusplayer_display_mode_e mode);
  int plusplayer_capi_set_display_roi(plusplayer_h handle,
                                      plusplayer_geometry_s roi);
  int plusplayer_capi_set_display_rotation(
      plusplayer_h handle, plusplayer_display_rotation_type_e rotation);
  int plusplayer_capi_set_buffer_config(plusplayer_h handle, const char* config,
                                        int amount);
  int plusplayer_capi_get_duration(plusplayer_h handle, int64_t* duration_ms);
  int plusplayer_capi_set_playback_rate(plusplayer_h handle,
                                        const double playback_rate);
  int plusplayer_capi_deactivate_audio(plusplayer_h handle);
  int plusplayer_capi_activate_audio(plusplayer_h handle);
  int plusplayer_capi_set_property(plusplayer_h handle,
                                   plusplayer_property_e property,
                                   const char* value);
  int plusplayer_capi_get_property(plusplayer_h handle,
                                   plusplayer_property_e property,
                                   char** value);
  int plusplayer_capi_get_track_count(plusplayer_h handle,
                                      plusplayer_track_type_e track_type,
                                      int* count);
  int plusplayer_capi_select_track(plusplayer_h handle,
                                   plusplayer_track_type_e type, int index);
  const char* plusplayer_capi_get_track_language_code(
      plusplayer_h handle, plusplayer_track_type_e type, int index);
  int plusplayer_capi_set_app_info(plusplayer_h handle,
                                   const plusplayer_app_info_s* app_info);
  int plusplayer_capi_set_drm(plusplayer_h handle,
                              plusplayer_drm_property_s drm_property);
  int plusplayer_capi_set_drm_init_data_cb(
      plusplayer_h handle, plusplayer_drm_init_data_cb drm_init_data_callback,
      void* userdata);
  int plusplayer_capi_set_adaptive_streaming_control_event_cb(
      plusplayer_h handle,
      plusplayer_adaptive_streaming_control_event_cb
          adaptive_streaming_control_event_cb,
      void* userdata);
  int plusplayer_capi_drm_license_acquired_done(
      plusplayer_h handle, plusplayer_track_type_e track_type);
  int plusplayer_capi_set_subtitle_path(plusplayer_h handle, const char* uri);
  int plusplayer_capi_set_video_stillmode(plusplayer_h handle,
                                          plusplayer_still_mode_e stillmode);
  int plusplayer_capi_set_alternative_video_resource(plusplayer_h handle,
                                                     unsigned int rsc_type);
  int plusplayer_capi_get_foreach_track(plusplayer_h handle,
                                        plusplayer_track_cb track_cb,
                                        void* userdata);
  int plusplayer_capi_get_foreach_active_track(plusplayer_h handle,
                                               plusplayer_track_cb track_cb,
                                               void* userdata);
  int plusplayer_capi_set_cookie(plusplayer_h handle, const char* cookie);
  int plusplayer_capi_set_user_agent(plusplayer_h handle,
                                     const char* user_agent);
  int plusplayer_capi_set_resume_time(plusplayer_h handle,
                                      uint64_t resume_time_ms);
  int plusplayer_capi_is_live_streaming(plusplayer_h handle, bool* is_live);
  int plusplayer_capi_get_dvr_seekable_range(plusplayer_h handle,
                                             uint64_t* start_time_ms,
                                             uint64_t* end_time_ms);
  int plusplayer_capi_get_current_bandwidth(plusplayer_h handle,
                                            uint32_t* curr_bandwidth_bps);
  int plusplayer_capi_get_track_index(plusplayer_track_h track,
                                      int* track_index);
  int plusplayer_capi_get_track_id(plusplayer_track_h track, int* track_id);
  int plusplayer_capi_get_track_mimetype(plusplayer_track_h track,
                                         const char** track_mimetype);
  int plusplayer_capi_get_track_streamtype(plusplayer_track_h track,
                                           const char** track_streamtype);
  int plusplayer_capi_get_track_container_type(
      plusplayer_track_h track, const char** track_containertype);
  int plusplayer_capi_get_track_type(plusplayer_track_h track,
                                     plusplayer_track_type_e* track_type);
  int plusplayer_capi_get_track_codec_data(plusplayer_track_h track,
                                           const char** track_codecdata);
  int plusplayer_capi_get_track_codec_tag(plusplayer_track_h track,
                                          unsigned int* track_codectag);
  int plusplayer_capi_get_track_codec_data_len(plusplayer_track_h track,
                                               int* track_codecdatalen);
  int plusplayer_capi_get_track_width(plusplayer_track_h track,
                                      int* track_width);
  int plusplayer_capi_get_track_height(plusplayer_track_h track,
                                       int* track_height);
  int plusplayer_capi_get_track_maxwidth(plusplayer_track_h track,
                                         int* track_maxwidth);
  int plusplayer_capi_get_track_maxheight(plusplayer_track_h track,
                                          int* track_maxheight);
  int plusplayer_capi_get_track_framerate_num(plusplayer_track_h track,
                                              int* track_framerate_num);
  int plusplayer_capi_get_track_framerate_den(plusplayer_track_h track,
                                              int* track_framerate_den);
  int plusplayer_capi_get_track_sample_rate(plusplayer_track_h track,
                                            int* track_sample_rate);
  int plusplayer_capi_get_track_sample_format(plusplayer_track_h track,
                                              int* track_sample_format);
  int plusplayer_capi_get_track_channels(plusplayer_track_h track,
                                         int* track_channels);
  int plusplayer_capi_get_track_version(plusplayer_track_h track,
                                        int* track_version);
  int plusplayer_capi_get_track_layer(plusplayer_track_h track,
                                      int* track_layer);
  int plusplayer_capi_get_track_bits_per_sample(plusplayer_track_h track,
                                                int* track_bits_per_sample);
  int plusplayer_capi_get_track_block_align(plusplayer_track_h track,
                                            int* track_block_align);
  int plusplayer_capi_get_track_bitrate(plusplayer_track_h track,
                                        int* track_bitrate);
  int plusplayer_capi_get_track_endianness(plusplayer_track_h track,
                                           int* track_endianness);
  int plusplayer_capi_get_track_is_signed(plusplayer_track_h track,
                                          bool* track_is_signed);
  int plusplayer_capi_get_track_active(plusplayer_track_h track,
                                       bool* track_active);
  int plusplayer_capi_get_track_lang_code(plusplayer_track_h track,
                                          const char** track_lang_code);
  int plusplayer_capi_get_track_subtitle_format(
      plusplayer_track_h track, const char** track_subtitle_format);

 private:
  void* plusplayer_capi_handle_ = nullptr;
};

#endif