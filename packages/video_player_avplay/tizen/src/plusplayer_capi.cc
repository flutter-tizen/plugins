// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plusplayer_capi/plusplayer_capi.h"

#include <dlfcn.h>
#include <stdlib.h>

#include "log.h"

// Function pointer types for all plusplayer functions
typedef plusplayer_h (*fun_plusplayer_create)(void);
typedef int (*fun_plusplayer_open)(plusplayer_h handle, const char* uri);
typedef int (*fun_plusplayer_prepare)(plusplayer_h handle);
typedef int (*fun_plusplayer_start)(plusplayer_h handle);
typedef int (*fun_plusplayer_stop)(plusplayer_h handle);
typedef int (*fun_plusplayer_close)(plusplayer_h handle);
typedef int (*fun_plusplayer_destroy)(plusplayer_h handle);
typedef plusplayer_state_e (*fun_plusplayer_get_state)(plusplayer_h handle);
typedef int (*fun_plusplayer_set_display)(plusplayer_h handle,
                                          plusplayer_display_type_e type,
                                          void* window);
typedef int (*fun_plusplayer_set_display_subsurface)(
    plusplayer_h handle, plusplayer_display_type_e type, uint32_t surface_id,
    plusplayer_geometry_s roi);
typedef int (*fun_plusplayer_set_prepare_async_done_cb)(
    plusplayer_h handle, plusplayer_prepare_async_done_cb prepare_async_done_cb,
    void* userdata);
typedef int (*fun_plusplayer_set_resource_conflicted_cb)(
    plusplayer_h handle,
    plusplayer_resource_conflicted_cb resource_conflicted_cb, void* userdata);
typedef int (*fun_plusplayer_set_eos_cb)(plusplayer_h handle,
                                         plusplayer_eos_cb eos_cb,
                                         void* userdata);
typedef int (*fun_plusplayer_set_buffer_status_cb)(
    plusplayer_h handle, plusplayer_buffer_status_cb buffer_status_cb,
    void* userdata);
typedef int (*fun_plusplayer_set_error_cb)(plusplayer_h handle,
                                           plusplayer_error_cb error_cb,
                                           void* userdata);
typedef int (*fun_plusplayer_set_error_msg_cb)(
    plusplayer_h handle, plusplayer_error_msg_cb error_msg_cb, void* userdata);
typedef int (*fun_plusplayer_set_seek_done_cb)(
    plusplayer_h handle, plusplayer_seek_done_cb seek_done_cb, void* userdata);
typedef int (*fun_plusplayer_set_subtitle_updated_cb)(
    plusplayer_h handle, plusplayer_subtitle_updated_cb subtitle_updated_cb,
    void* userdata);
typedef int (*fun_plusplayer_set_ad_event_cb)(
    plusplayer_h handle, plusplayer_ad_event_cb ad_event_cb, void* userdata);
typedef int (*fun_plusplayer_prepare_async)(plusplayer_h handle);
typedef int (*fun_plusplayer_pause)(plusplayer_h handle);
typedef int (*fun_plusplayer_resume)(plusplayer_h handle);
typedef int (*fun_plusplayer_seek)(plusplayer_h handle, uint64_t time);
typedef void (*fun_plusplayer_set_prebuffer_mode)(plusplayer_h handle,
                                                  bool prebuffer_mode);
typedef int (*fun_plusplayer_set_app_id)(plusplayer_h handle,
                                         const char* app_id);
typedef int (*fun_plusplayer_suspend)(plusplayer_h handle);
typedef int (*fun_plusplayer_restore)(plusplayer_h handle,
                                      plusplayer_state_e target_state);
typedef int (*fun_plusplayer_get_playing_time)(plusplayer_h handle,
                                               uint64_t* cur_time_ms);
typedef int (*fun_plusplayer_set_display_mode)(plusplayer_h handle,
                                               plusplayer_display_mode_e mode);
typedef int (*fun_plusplayer_set_display_roi)(plusplayer_h handle,
                                              plusplayer_geometry_s roi);
typedef int (*fun_plusplayer_set_display_rotation)(
    plusplayer_h handle, plusplayer_display_rotation_type_e rotation);
typedef int (*fun_plusplayer_set_buffer_config)(plusplayer_h handle,
                                                const char* config, int amount);
typedef int (*fun_plusplayer_get_duration)(plusplayer_h handle,
                                           int64_t* duration_ms);
typedef int (*fun_plusplayer_set_playback_rate)(plusplayer_h handle,
                                                const double playback_rate);
typedef int (*fun_plusplayer_deactivate_audio)(plusplayer_h handle);
typedef int (*fun_plusplayer_activate_audio)(plusplayer_h handle);
typedef int (*fun_plusplayer_set_property)(plusplayer_h handle,
                                           plusplayer_property_e property,
                                           const char* value);
typedef int (*fun_plusplayer_get_property)(plusplayer_h handle,
                                           plusplayer_property_e property,
                                           char** value);
typedef int (*fun_plusplayer_get_track_count)(
    plusplayer_h handle, plusplayer_track_type_e track_type, int* count);
typedef int (*fun_plusplayer_select_track)(plusplayer_h handle,
                                           plusplayer_track_type_e type,
                                           int index);
typedef const char* (*fun_plusplayer_get_track_language_code)(
    plusplayer_h handle, plusplayer_track_type_e type, int index);

// Function pointer types for track_capi.h functions
typedef int (*fun_plusplayer_get_track_index)(plusplayer_track_h track,
                                              int* track_index);
typedef int (*fun_plusplayer_get_track_id)(plusplayer_track_h track,
                                           int* track_id);
typedef int (*fun_plusplayer_get_track_mimetype)(plusplayer_track_h track,
                                                 const char** track_mimetype);
typedef int (*fun_plusplayer_get_track_streamtype)(
    plusplayer_track_h track, const char** track_streamtype);
typedef int (*fun_plusplayer_get_track_container_type)(
    plusplayer_track_h track, const char** track_containertype);
typedef int (*fun_plusplayer_get_track_type)(
    plusplayer_track_h track, plusplayer_track_type_e* track_type);
typedef int (*fun_plusplayer_get_track_codec_data)(
    plusplayer_track_h track, const char** track_codecdata);
typedef int (*fun_plusplayer_get_track_codec_tag)(plusplayer_track_h track,
                                                  unsigned int* track_codectag);
typedef int (*fun_plusplayer_get_track_codec_data_len)(plusplayer_track_h track,
                                                       int* track_codecdatalen);
typedef int (*fun_plusplayer_get_track_width)(plusplayer_track_h track,
                                              int* track_width);
typedef int (*fun_plusplayer_get_track_height)(plusplayer_track_h track,
                                               int* track_height);
typedef int (*fun_plusplayer_get_track_maxwidth)(plusplayer_track_h track,
                                                 int* track_maxwidth);
typedef int (*fun_plusplayer_get_track_maxheight)(plusplayer_track_h track,
                                                  int* track_maxheight);
typedef int (*fun_plusplayer_get_track_framerate_num)(plusplayer_track_h track,
                                                      int* track_framerate_num);
typedef int (*fun_plusplayer_get_track_framerate_den)(plusplayer_track_h track,
                                                      int* track_framerate_den);
typedef int (*fun_plusplayer_get_track_sample_rate)(plusplayer_track_h track,
                                                    int* track_sample_rate);
typedef int (*fun_plusplayer_get_track_sample_format)(plusplayer_track_h track,
                                                      int* track_sample_format);
typedef int (*fun_plusplayer_get_track_channels)(plusplayer_track_h track,
                                                 int* track_channels);
typedef int (*fun_plusplayer_get_track_version)(plusplayer_track_h track,
                                                int* track_version);
typedef int (*fun_plusplayer_get_track_layer)(plusplayer_track_h track,
                                              int* track_layer);
typedef int (*fun_plusplayer_get_track_bits_per_sample)(
    plusplayer_track_h track, int* track_bits_per_sample);
typedef int (*fun_plusplayer_get_track_block_align)(plusplayer_track_h track,
                                                    int* track_block_align);
typedef int (*fun_plusplayer_get_track_bitrate)(plusplayer_track_h track,
                                                int* track_bitrate);
typedef int (*fun_plusplayer_get_track_endianness)(plusplayer_track_h track,
                                                   int* track_endianness);
typedef int (*fun_plusplayer_get_track_is_signed)(plusplayer_track_h track,
                                                  bool* track_is_signed);
typedef int (*fun_plusplayer_get_track_active)(plusplayer_track_h track,
                                               bool* track_active);
typedef int (*fun_plusplayer_get_track_lang_code)(plusplayer_track_h track,
                                                  const char** track_lang_code);
typedef int (*fun_plusplayer_get_track_subtitle_format)(
    plusplayer_track_h track, const char** track_subtitle_format);
typedef int (*fun_plusplayer_set_app_info)(
    plusplayer_h handle, const plusplayer_app_info_s* app_info);
typedef int (*fun_plusplayer_set_drm)(plusplayer_h handle,
                                      plusplayer_drm_property_s drm_property);
typedef int (*fun_plusplayer_set_drm_init_data_cb)(
    plusplayer_h handle, plusplayer_drm_init_data_cb drm_init_data_callback,
    void* userdata);
typedef int (*fun_plusplayer_set_adaptive_streaming_control_event_cb)(
    plusplayer_h handle,
    plusplayer_adaptive_streaming_control_event_cb
        adaptive_streaming_control_event_cb,
    void* userdata);
typedef int (*fun_plusplayer_drm_license_acquired_done)(
    plusplayer_h handle, plusplayer_track_type_e track_type);
typedef int (*fun_plusplayer_set_subtitle_path)(plusplayer_h handle,
                                                const char* uri);
typedef int (*fun_plusplayer_set_video_stillmode)(
    plusplayer_h handle, plusplayer_still_mode_e stillmode);
typedef int (*fun_plusplayer_set_alternative_video_resource)(
    plusplayer_h handle, unsigned int rsc_type);
typedef int (*fun_plusplayer_get_foreach_track)(plusplayer_h handle,
                                                plusplayer_track_cb track_cb,
                                                void* userdata);
typedef int (*fun_plusplayer_get_foreach_active_track)(
    plusplayer_h handle, plusplayer_track_cb track_cb, void* userdata);
typedef int (*fun_plusplayer_set_cookie)(plusplayer_h handle,
                                         const char* cookie);
typedef int (*fun_plusplayer_set_user_agent)(plusplayer_h handle,
                                             const char* user_agent);
typedef int (*fun_plusplayer_set_resume_time)(plusplayer_h handle,
                                              uint64_t resume_time_ms);
typedef int (*fun_plusplayer_is_live_streaming)(plusplayer_h handle,
                                                bool* is_live);
typedef int (*fun_plusplayer_get_dvr_seekable_range)(plusplayer_h handle,
                                                     uint64_t* start_time_ms,
                                                     uint64_t* end_time_ms);
typedef int (*fun_plusplayer_get_current_bandwidth)(
    plusplayer_h handle, uint32_t* curr_bandwidth_bps);

// Structure to hold library handle and function pointers
typedef struct {
  void* library_handle;

  // Function pointers
  fun_plusplayer_create create;
  fun_plusplayer_open open;
  fun_plusplayer_prepare prepare;
  fun_plusplayer_start start;
  fun_plusplayer_stop stop;
  fun_plusplayer_close close;
  fun_plusplayer_destroy destroy;
  fun_plusplayer_get_state get_state;
  fun_plusplayer_set_display set_display;
  fun_plusplayer_set_display_subsurface set_display_subsurface;
  fun_plusplayer_set_prepare_async_done_cb set_prepare_async_done_cb;
  fun_plusplayer_set_resource_conflicted_cb set_resource_conflicted_cb;
  fun_plusplayer_set_eos_cb set_eos_cb;
  fun_plusplayer_set_buffer_status_cb set_buffer_status_cb;
  fun_plusplayer_set_error_cb set_error_cb;
  fun_plusplayer_set_error_msg_cb set_error_msg_cb;
  fun_plusplayer_set_seek_done_cb set_seek_done_cb;
  fun_plusplayer_set_subtitle_updated_cb set_subtitle_updated_cb;
  fun_plusplayer_set_ad_event_cb set_ad_event_cb;
  fun_plusplayer_prepare_async prepare_async;
  fun_plusplayer_pause pause;
  fun_plusplayer_resume resume;
  fun_plusplayer_seek seek;
  fun_plusplayer_set_prebuffer_mode set_prebuffer_mode;
  fun_plusplayer_set_app_id set_app_id;
  fun_plusplayer_suspend suspend;
  fun_plusplayer_restore restore;
  fun_plusplayer_get_playing_time get_playing_time;
  fun_plusplayer_set_display_mode set_display_mode;
  fun_plusplayer_set_display_roi set_display_roi;
  fun_plusplayer_set_display_rotation set_display_rotation;
  fun_plusplayer_set_buffer_config set_buffer_config;
  fun_plusplayer_get_duration get_duration;
  fun_plusplayer_set_playback_rate set_playback_rate;
  fun_plusplayer_deactivate_audio deactivate_audio;
  fun_plusplayer_activate_audio activate_audio;
  fun_plusplayer_set_property set_property;
  fun_plusplayer_get_property get_property;
  fun_plusplayer_get_track_count get_track_count;
  fun_plusplayer_select_track select_track;
  fun_plusplayer_get_track_language_code get_track_language_code;
  // Track CAPI function pointers
  fun_plusplayer_get_track_index get_track_index;
  fun_plusplayer_get_track_id get_track_id;
  fun_plusplayer_get_track_mimetype get_track_mimetype;
  fun_plusplayer_get_track_streamtype get_track_streamtype;
  fun_plusplayer_get_track_container_type get_track_container_type;
  fun_plusplayer_get_track_type get_track_type;
  fun_plusplayer_get_track_codec_data get_track_codec_data;
  fun_plusplayer_get_track_codec_tag get_track_codec_tag;
  fun_plusplayer_get_track_codec_data_len get_track_codec_data_len;
  fun_plusplayer_get_track_width get_track_width;
  fun_plusplayer_get_track_height get_track_height;
  fun_plusplayer_get_track_maxwidth get_track_maxwidth;
  fun_plusplayer_get_track_maxheight get_track_maxheight;
  fun_plusplayer_get_track_framerate_num get_track_framerate_num;
  fun_plusplayer_get_track_framerate_den get_track_framerate_den;
  fun_plusplayer_get_track_sample_rate get_track_sample_rate;
  fun_plusplayer_get_track_sample_format get_track_sample_format;
  fun_plusplayer_get_track_channels get_track_channels;
  fun_plusplayer_get_track_version get_track_version;
  fun_plusplayer_get_track_layer get_track_layer;
  fun_plusplayer_get_track_bits_per_sample get_track_bits_per_sample;
  fun_plusplayer_get_track_block_align get_track_block_align;
  fun_plusplayer_get_track_bitrate get_track_bitrate;
  fun_plusplayer_get_track_endianness get_track_endianness;
  fun_plusplayer_get_track_is_signed get_track_is_signed;
  fun_plusplayer_get_track_active get_track_active;
  fun_plusplayer_get_track_lang_code get_track_lang_code;
  fun_plusplayer_get_track_subtitle_format get_track_subtitle_format;
  fun_plusplayer_set_app_info set_app_info;
  fun_plusplayer_set_drm set_drm;
  fun_plusplayer_set_drm_init_data_cb set_drm_init_data_cb;
  fun_plusplayer_set_adaptive_streaming_control_event_cb
      set_adaptive_streaming_control_event_cb;
  fun_plusplayer_drm_license_acquired_done drm_license_acquired_done;
  fun_plusplayer_set_subtitle_path set_subtitle_path;
  fun_plusplayer_set_video_stillmode set_video_stillmode;
  fun_plusplayer_set_alternative_video_resource set_alternative_video_resource;
  fun_plusplayer_get_foreach_track get_foreach_track;
  fun_plusplayer_get_foreach_active_track get_foreach_active_track;
  fun_plusplayer_set_cookie set_cookie;
  fun_plusplayer_set_user_agent set_user_agent;
  fun_plusplayer_set_resume_time set_resume_time;
  fun_plusplayer_is_live_streaming is_live_streaming;
  fun_plusplayer_get_dvr_seekable_range get_dvr_seekable_range;
  fun_plusplayer_get_current_bandwidth get_current_bandwidth;
} plusplayer_capi_t;

// Global instance to hold the library handle and function pointers
static plusplayer_capi_t g_plusplayer_capi = {0};

// Template function for dynamic library function calls
static void* load_plusplayer_function(const char* func_name) {
  if (!g_plusplayer_capi.library_handle) {
    LOG_ERROR("plusplayer_capi library_handle is invalid.");
    return NULL;
  }

  void* func = dlsym(g_plusplayer_capi.library_handle, func_name);
  if (!func) {
    LOG_ERROR("Failed to find %s function.", func_name);
    return NULL;
  }

  return func;
}

// Initialize the plusplayer C API
static int initialize_plusplayer_capi(void) {
  if (g_plusplayer_capi.library_handle) {
    // Already initialized
    return PLUSPLAYER_ERROR_TYPE_NONE;
  }

  g_plusplayer_capi.library_handle = dlopen("libplusplayer.so", RTLD_LAZY);
  if (g_plusplayer_capi.library_handle == NULL) {
    LOG_ERROR("Failed to open libplusplayer.so");
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }

  // Load all function pointers
  g_plusplayer_capi.create =
      (fun_plusplayer_create)load_plusplayer_function("plusplayer_create");
  g_plusplayer_capi.open =
      (fun_plusplayer_open)load_plusplayer_function("plusplayer_open");
  g_plusplayer_capi.prepare =
      (fun_plusplayer_prepare)load_plusplayer_function("plusplayer_prepare");
  g_plusplayer_capi.start =
      (fun_plusplayer_start)load_plusplayer_function("plusplayer_start");
  g_plusplayer_capi.stop =
      (fun_plusplayer_stop)load_plusplayer_function("plusplayer_stop");
  g_plusplayer_capi.close =
      (fun_plusplayer_close)load_plusplayer_function("plusplayer_close");
  g_plusplayer_capi.destroy =
      (fun_plusplayer_destroy)load_plusplayer_function("plusplayer_destroy");
  g_plusplayer_capi.get_state =
      (fun_plusplayer_get_state)load_plusplayer_function(
          "plusplayer_get_state");
  g_plusplayer_capi.set_display =
      (fun_plusplayer_set_display)load_plusplayer_function(
          "plusplayer_set_display");
  g_plusplayer_capi.set_display_subsurface =
      (fun_plusplayer_set_display_subsurface)load_plusplayer_function(
          "plusplayer_set_display_subsurface");
  g_plusplayer_capi.set_prepare_async_done_cb =
      (fun_plusplayer_set_prepare_async_done_cb)load_plusplayer_function(
          "plusplayer_set_prepare_async_done_cb");
  g_plusplayer_capi.set_resource_conflicted_cb =
      (fun_plusplayer_set_resource_conflicted_cb)load_plusplayer_function(
          "plusplayer_set_resource_conflicted_cb");
  g_plusplayer_capi.set_eos_cb =
      (fun_plusplayer_set_eos_cb)load_plusplayer_function(
          "plusplayer_set_eos_cb");
  g_plusplayer_capi.set_buffer_status_cb =
      (fun_plusplayer_set_buffer_status_cb)load_plusplayer_function(
          "plusplayer_set_buffer_status_cb");
  g_plusplayer_capi.set_error_cb =
      (fun_plusplayer_set_error_cb)load_plusplayer_function(
          "plusplayer_set_error_cb");
  g_plusplayer_capi.set_error_msg_cb =
      (fun_plusplayer_set_error_msg_cb)load_plusplayer_function(
          "plusplayer_set_error_msg_cb");
  g_plusplayer_capi.set_seek_done_cb =
      (fun_plusplayer_set_seek_done_cb)load_plusplayer_function(
          "plusplayer_set_seek_done_cb");
  g_plusplayer_capi.set_subtitle_updated_cb =
      (fun_plusplayer_set_subtitle_updated_cb)load_plusplayer_function(
          "plusplayer_set_subtitle_updated_cb");
  g_plusplayer_capi.set_ad_event_cb =
      (fun_plusplayer_set_ad_event_cb)load_plusplayer_function(
          "plusplayer_set_ad_event_cb");
  g_plusplayer_capi.prepare_async =
      (fun_plusplayer_prepare_async)load_plusplayer_function(
          "plusplayer_prepare_async");
  g_plusplayer_capi.pause =
      (fun_plusplayer_pause)load_plusplayer_function("plusplayer_pause");
  g_plusplayer_capi.resume =
      (fun_plusplayer_resume)load_plusplayer_function("plusplayer_resume");
  g_plusplayer_capi.seek =
      (fun_plusplayer_seek)load_plusplayer_function("plusplayer_seek");
  g_plusplayer_capi.set_prebuffer_mode =
      (fun_plusplayer_set_prebuffer_mode)load_plusplayer_function(
          "plusplayer_set_prebuffer_mode");
  g_plusplayer_capi.set_app_id =
      (fun_plusplayer_set_app_id)load_plusplayer_function(
          "plusplayer_set_app_id");
  g_plusplayer_capi.suspend =
      (fun_plusplayer_suspend)load_plusplayer_function("plusplayer_suspend");
  g_plusplayer_capi.restore =
      (fun_plusplayer_restore)load_plusplayer_function("plusplayer_restore");
  g_plusplayer_capi.get_playing_time =
      (fun_plusplayer_get_playing_time)load_plusplayer_function(
          "plusplayer_get_playing_time");
  g_plusplayer_capi.set_display_mode =
      (fun_plusplayer_set_display_mode)load_plusplayer_function(
          "plusplayer_set_display_mode");
  g_plusplayer_capi.set_display_roi =
      (fun_plusplayer_set_display_roi)load_plusplayer_function(
          "plusplayer_set_display_roi");
  g_plusplayer_capi.set_display_rotation =
      (fun_plusplayer_set_display_rotation)load_plusplayer_function(
          "plusplayer_set_display_rotation");
  g_plusplayer_capi.set_buffer_config =
      (fun_plusplayer_set_buffer_config)load_plusplayer_function(
          "plusplayer_set_buffer_config");
  g_plusplayer_capi.get_duration =
      (fun_plusplayer_get_duration)load_plusplayer_function(
          "plusplayer_get_duration");
  g_plusplayer_capi.set_playback_rate =
      (fun_plusplayer_set_playback_rate)load_plusplayer_function(
          "plusplayer_set_playback_rate");
  g_plusplayer_capi.deactivate_audio =
      (fun_plusplayer_deactivate_audio)load_plusplayer_function(
          "plusplayer_deactivate_audio");
  g_plusplayer_capi.activate_audio =
      (fun_plusplayer_activate_audio)load_plusplayer_function(
          "plusplayer_activate_audio");
  g_plusplayer_capi.set_property =
      (fun_plusplayer_set_property)load_plusplayer_function(
          "plusplayer_set_property");
  g_plusplayer_capi.get_property =
      (fun_plusplayer_get_property)load_plusplayer_function(
          "plusplayer_get_property");
  g_plusplayer_capi.get_track_count =
      (fun_plusplayer_get_track_count)load_plusplayer_function(
          "plusplayer_get_track_count");
  g_plusplayer_capi.select_track =
      (fun_plusplayer_select_track)load_plusplayer_function(
          "plusplayer_select_track");
  g_plusplayer_capi.get_track_language_code =
      (fun_plusplayer_get_track_language_code)load_plusplayer_function(
          "plusplayer_get_track_language_code");
  // Initialize track CAPI function pointers
  g_plusplayer_capi.get_track_index =
      (fun_plusplayer_get_track_index)load_plusplayer_function(
          "plusplayer_get_track_index");
  g_plusplayer_capi.get_track_id =
      (fun_plusplayer_get_track_id)load_plusplayer_function(
          "plusplayer_get_track_id");
  g_plusplayer_capi.get_track_mimetype =
      (fun_plusplayer_get_track_mimetype)load_plusplayer_function(
          "plusplayer_get_track_mimetype");
  g_plusplayer_capi.get_track_streamtype =
      (fun_plusplayer_get_track_streamtype)load_plusplayer_function(
          "plusplayer_get_track_streamtype");
  g_plusplayer_capi.get_track_container_type =
      (fun_plusplayer_get_track_container_type)load_plusplayer_function(
          "plusplayer_get_track_container_type");
  g_plusplayer_capi.get_track_type =
      (fun_plusplayer_get_track_type)load_plusplayer_function(
          "plusplayer_get_track_type");
  g_plusplayer_capi.get_track_codec_data =
      (fun_plusplayer_get_track_codec_data)load_plusplayer_function(
          "plusplayer_get_track_codec_data");
  g_plusplayer_capi.get_track_codec_tag =
      (fun_plusplayer_get_track_codec_tag)load_plusplayer_function(
          "plusplayer_get_track_codec_tag");
  g_plusplayer_capi.get_track_codec_data_len =
      (fun_plusplayer_get_track_codec_data_len)load_plusplayer_function(
          "plusplayer_get_track_codec_data_len");
  g_plusplayer_capi.get_track_width =
      (fun_plusplayer_get_track_width)load_plusplayer_function(
          "plusplayer_get_track_width");
  g_plusplayer_capi.get_track_height =
      (fun_plusplayer_get_track_height)load_plusplayer_function(
          "plusplayer_get_track_height");
  g_plusplayer_capi.get_track_maxwidth =
      (fun_plusplayer_get_track_maxwidth)load_plusplayer_function(
          "plusplayer_get_track_maxwidth");
  g_plusplayer_capi.get_track_maxheight =
      (fun_plusplayer_get_track_maxheight)load_plusplayer_function(
          "plusplayer_get_track_maxheight");
  g_plusplayer_capi.get_track_framerate_num =
      (fun_plusplayer_get_track_framerate_num)load_plusplayer_function(
          "plusplayer_get_track_framerate_num");
  g_plusplayer_capi.get_track_framerate_den =
      (fun_plusplayer_get_track_framerate_den)load_plusplayer_function(
          "plusplayer_get_track_framerate_den");
  g_plusplayer_capi.get_track_sample_rate =
      (fun_plusplayer_get_track_sample_rate)load_plusplayer_function(
          "plusplayer_get_track_sample_rate");
  g_plusplayer_capi.get_track_sample_format =
      (fun_plusplayer_get_track_sample_format)load_plusplayer_function(
          "plusplayer_get_track_sample_format");
  g_plusplayer_capi.get_track_channels =
      (fun_plusplayer_get_track_channels)load_plusplayer_function(
          "plusplayer_get_track_channels");
  g_plusplayer_capi.get_track_version =
      (fun_plusplayer_get_track_version)load_plusplayer_function(
          "plusplayer_get_track_version");
  g_plusplayer_capi.get_track_layer =
      (fun_plusplayer_get_track_layer)load_plusplayer_function(
          "plusplayer_get_track_layer");
  g_plusplayer_capi.get_track_bits_per_sample =
      (fun_plusplayer_get_track_bits_per_sample)load_plusplayer_function(
          "plusplayer_get_track_bits_per_sample");
  g_plusplayer_capi.get_track_block_align =
      (fun_plusplayer_get_track_block_align)load_plusplayer_function(
          "plusplayer_get_track_block_align");
  g_plusplayer_capi.get_track_bitrate =
      (fun_plusplayer_get_track_bitrate)load_plusplayer_function(
          "plusplayer_get_track_bitrate");
  g_plusplayer_capi.get_track_endianness =
      (fun_plusplayer_get_track_endianness)load_plusplayer_function(
          "plusplayer_get_track_endianness");
  g_plusplayer_capi.get_track_is_signed =
      (fun_plusplayer_get_track_is_signed)load_plusplayer_function(
          "plusplayer_get_track_is_signed");
  g_plusplayer_capi.get_track_active =
      (fun_plusplayer_get_track_active)load_plusplayer_function(
          "plusplayer_get_track_active");
  g_plusplayer_capi.get_track_lang_code =
      (fun_plusplayer_get_track_lang_code)load_plusplayer_function(
          "plusplayer_get_track_lang_code");
  g_plusplayer_capi.get_track_subtitle_format =
      (fun_plusplayer_get_track_subtitle_format)load_plusplayer_function(
          "plusplayer_get_track_subtitle_format");
  g_plusplayer_capi.set_app_info =
      (fun_plusplayer_set_app_info)load_plusplayer_function(
          "plusplayer_set_app_info");
  g_plusplayer_capi.set_drm =
      (fun_plusplayer_set_drm)load_plusplayer_function("plusplayer_set_drm");
  g_plusplayer_capi.set_drm_init_data_cb =
      (fun_plusplayer_set_drm_init_data_cb)load_plusplayer_function(
          "plusplayer_set_drm_init_data_cb");
  g_plusplayer_capi.set_adaptive_streaming_control_event_cb =
      (fun_plusplayer_set_adaptive_streaming_control_event_cb)
          load_plusplayer_function(
              "plusplayer_set_adaptive_streaming_control_event_cb");
  g_plusplayer_capi.drm_license_acquired_done =
      (fun_plusplayer_drm_license_acquired_done)load_plusplayer_function(
          "plusplayer_drm_license_acquired_done");
  g_plusplayer_capi.set_subtitle_path =
      (fun_plusplayer_set_subtitle_path)load_plusplayer_function(
          "plusplayer_set_subtitle_path");
  g_plusplayer_capi.set_video_stillmode =
      (fun_plusplayer_set_video_stillmode)load_plusplayer_function(
          "plusplayer_set_video_stillmode");
  g_plusplayer_capi.set_alternative_video_resource =
      (fun_plusplayer_set_alternative_video_resource)load_plusplayer_function(
          "plusplayer_set_alternative_video_resource");
  g_plusplayer_capi.get_foreach_track =
      (fun_plusplayer_get_foreach_track)load_plusplayer_function(
          "plusplayer_get_foreach_track");
  g_plusplayer_capi.get_foreach_active_track =
      (fun_plusplayer_get_foreach_active_track)load_plusplayer_function(
          "plusplayer_get_foreach_active_track");
  g_plusplayer_capi.set_cookie =
      (fun_plusplayer_set_cookie)load_plusplayer_function(
          "plusplayer_set_cookie");
  g_plusplayer_capi.set_user_agent =
      (fun_plusplayer_set_user_agent)load_plusplayer_function(
          "plusplayer_set_user_agent");
  g_plusplayer_capi.set_resume_time =
      (fun_plusplayer_set_resume_time)load_plusplayer_function(
          "plusplayer_set_resume_time");
  g_plusplayer_capi.is_live_streaming =
      (fun_plusplayer_is_live_streaming)load_plusplayer_function(
          "plusplayer_is_live_streaming");
  g_plusplayer_capi.get_dvr_seekable_range =
      (fun_plusplayer_get_dvr_seekable_range)load_plusplayer_function(
          "plusplayer_get_dvr_seekable_range");
  g_plusplayer_capi.get_current_bandwidth =
      (fun_plusplayer_get_current_bandwidth)load_plusplayer_function(
          "plusplayer_get_current_bandwidth");

  return PLUSPLAYER_ERROR_TYPE_NONE;
}

// PlusPlayer API implementation functions
plusplayer_h plusplayer_create(void) {
  if (initialize_plusplayer_capi() != PLUSPLAYER_ERROR_TYPE_NONE) {
    return NULL;
  }

  if (!g_plusplayer_capi.create) {
    return NULL;
  }

  return g_plusplayer_capi.create();
}

int plusplayer_open(plusplayer_h handle, const char* uri) {
  if (!g_plusplayer_capi.open) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.open(handle, uri);
}

int plusplayer_prepare(plusplayer_h handle) {
  if (!g_plusplayer_capi.prepare) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.prepare(handle);
}

int plusplayer_start(plusplayer_h handle) {
  if (!g_plusplayer_capi.start) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.start(handle);
}

int plusplayer_stop(plusplayer_h handle) {
  if (!g_plusplayer_capi.stop) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.stop(handle);
}

int plusplayer_close(plusplayer_h handle) {
  if (!g_plusplayer_capi.close) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.close(handle);
}

int plusplayer_destroy(plusplayer_h handle) {
  if (!g_plusplayer_capi.destroy) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.destroy(handle);
}

plusplayer_state_e plusplayer_get_state(plusplayer_h handle) {
  if (!g_plusplayer_capi.get_state) {
    return (plusplayer_state_e)(-1);
  }
  return g_plusplayer_capi.get_state(handle);
}

int plusplayer_set_display(plusplayer_h handle, plusplayer_display_type_e type,
                           void* window) {
  if (!g_plusplayer_capi.set_display) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_display(handle, type, window);
}

int plusplayer_set_display_subsurface(plusplayer_h handle,
                                      plusplayer_display_type_e type,
                                      uint32_t surface_id,
                                      plusplayer_geometry_s roi) {
  if (!g_plusplayer_capi.set_display_subsurface) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_display_subsurface(handle, type, surface_id,
                                                  roi);
}

int plusplayer_set_prepare_async_done_cb(
    plusplayer_h handle, plusplayer_prepare_async_done_cb prepare_async_done_cb,
    void* userdata) {
  if (!g_plusplayer_capi.set_prepare_async_done_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_prepare_async_done_cb(
      handle, prepare_async_done_cb, userdata);
}

int plusplayer_set_resource_conflicted_cb(
    plusplayer_h handle,
    plusplayer_resource_conflicted_cb resource_conflicted_cb, void* userdata) {
  if (!g_plusplayer_capi.set_resource_conflicted_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_resource_conflicted_cb(
      handle, resource_conflicted_cb, userdata);
}

int plusplayer_set_eos_cb(plusplayer_h handle, plusplayer_eos_cb eos_cb,
                          void* userdata) {
  if (!g_plusplayer_capi.set_eos_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_eos_cb(handle, eos_cb, userdata);
}

int plusplayer_set_buffer_status_cb(
    plusplayer_h handle, plusplayer_buffer_status_cb buffer_status_cb,
    void* userdata) {
  if (!g_plusplayer_capi.set_buffer_status_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_buffer_status_cb(handle, buffer_status_cb,
                                                userdata);
}

int plusplayer_set_error_cb(plusplayer_h handle, plusplayer_error_cb error_cb,
                            void* userdata) {
  if (!g_plusplayer_capi.set_error_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_error_cb(handle, error_cb, userdata);
}

int plusplayer_set_error_msg_cb(plusplayer_h handle,
                                plusplayer_error_msg_cb error_msg_cb,
                                void* userdata) {
  if (!g_plusplayer_capi.set_error_msg_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_error_msg_cb(handle, error_msg_cb, userdata);
}

int plusplayer_set_seek_done_cb(plusplayer_h handle,
                                plusplayer_seek_done_cb seek_done_cb,
                                void* userdata) {
  if (!g_plusplayer_capi.set_seek_done_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_seek_done_cb(handle, seek_done_cb, userdata);
}

int plusplayer_set_subtitle_updated_cb(
    plusplayer_h handle, plusplayer_subtitle_updated_cb subtitle_updated_cb,
    void* userdata) {
  if (!g_plusplayer_capi.set_subtitle_updated_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_subtitle_updated_cb(handle, subtitle_updated_cb,
                                                   userdata);
}

int plusplayer_set_ad_event_cb(plusplayer_h handle,
                               plusplayer_ad_event_cb ad_event_cb,
                               void* userdata) {
  if (!g_plusplayer_capi.set_ad_event_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_ad_event_cb(handle, ad_event_cb, userdata);
}

int plusplayer_prepare_async(plusplayer_h handle) {
  if (!g_plusplayer_capi.prepare_async) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.prepare_async(handle);
}

int plusplayer_pause(plusplayer_h handle) {
  if (!g_plusplayer_capi.pause) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.pause(handle);
}

int plusplayer_resume(plusplayer_h handle) {
  if (!g_plusplayer_capi.resume) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.resume(handle);
}

int plusplayer_seek(plusplayer_h handle, uint64_t time) {
  if (!g_plusplayer_capi.seek) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.seek(handle, time);
}

void plusplayer_set_prebuffer_mode(plusplayer_h handle, bool prebuffer_mode) {
  if (g_plusplayer_capi.set_prebuffer_mode) {
    g_plusplayer_capi.set_prebuffer_mode(handle, prebuffer_mode);
  }
}

int plusplayer_set_app_id(plusplayer_h handle, const char* app_id) {
  if (!g_plusplayer_capi.set_app_id) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_app_id(handle, app_id);
}

int plusplayer_suspend(plusplayer_h handle) {
  if (!g_plusplayer_capi.suspend) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.suspend(handle);
}

int plusplayer_restore(plusplayer_h handle, plusplayer_state_e target_state) {
  if (!g_plusplayer_capi.restore) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.restore(handle, target_state);
}

int plusplayer_get_playing_time(plusplayer_h handle, uint64_t* cur_time_ms) {
  if (!g_plusplayer_capi.get_playing_time) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_playing_time(handle, cur_time_ms);
}

int plusplayer_set_display_mode(plusplayer_h handle,
                                plusplayer_display_mode_e mode) {
  if (!g_plusplayer_capi.set_display_mode) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_display_mode(handle, mode);
}

int plusplayer_set_display_roi(plusplayer_h handle, plusplayer_geometry_s roi) {
  if (!g_plusplayer_capi.set_display_roi) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_display_roi(handle, roi);
}

int plusplayer_set_display_rotation(
    plusplayer_h handle, plusplayer_display_rotation_type_e rotation) {
  if (!g_plusplayer_capi.set_display_rotation) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_display_rotation(handle, rotation);
}

int plusplayer_set_buffer_config(plusplayer_h handle, const char* config,
                                 int amount) {
  if (!g_plusplayer_capi.set_buffer_config) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_buffer_config(handle, config, amount);
}

int plusplayer_get_duration(plusplayer_h handle, int64_t* duration_ms) {
  if (!g_plusplayer_capi.get_duration) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_duration(handle, duration_ms);
}

int plusplayer_set_playback_rate(plusplayer_h handle,
                                 const double playback_rate) {
  if (!g_plusplayer_capi.set_playback_rate) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_playback_rate(handle, playback_rate);
}

int plusplayer_deactivate_audio(plusplayer_h handle) {
  if (!g_plusplayer_capi.deactivate_audio) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.deactivate_audio(handle);
}

int plusplayer_activate_audio(plusplayer_h handle) {
  if (!g_plusplayer_capi.activate_audio) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.activate_audio(handle);
}

int plusplayer_set_property(plusplayer_h handle, plusplayer_property_e property,
                            const char* value) {
  if (!g_plusplayer_capi.set_property) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_property(handle, property, value);
}

int plusplayer_get_property(plusplayer_h handle, plusplayer_property_e property,
                            char** value) {
  if (!g_plusplayer_capi.get_property) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_property(handle, property, value);
}

int plusplayer_get_track_count(plusplayer_h handle,
                               plusplayer_track_type_e track_type, int* count) {
  if (!g_plusplayer_capi.get_track_count) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_count(handle, track_type, count);
}

int plusplayer_select_track(plusplayer_h handle, plusplayer_track_type_e type,
                            int index) {
  if (!g_plusplayer_capi.select_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.select_track(handle, type, index);
}

const char* plusplayer_get_track_language_code(plusplayer_h handle,
                                               plusplayer_track_type_e type,
                                               int index) {
  if (!g_plusplayer_capi.get_track_language_code) {
    return NULL;
  }
  return g_plusplayer_capi.get_track_language_code(handle, type, index);
}

int plusplayer_set_app_info(plusplayer_h handle,
                            const plusplayer_app_info_s* app_info) {
  if (!g_plusplayer_capi.set_app_info) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_app_info(handle, app_info);
}

int plusplayer_set_drm(plusplayer_h handle,
                       plusplayer_drm_property_s drm_property) {
  if (!g_plusplayer_capi.set_drm) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_drm(handle, drm_property);
}

int plusplayer_set_drm_init_data_cb(
    plusplayer_h handle, plusplayer_drm_init_data_cb drm_init_data_callback,
    void* userdata) {
  if (!g_plusplayer_capi.set_drm_init_data_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_drm_init_data_cb(handle, drm_init_data_callback,
                                                userdata);
}

int plusplayer_set_adaptive_streaming_control_event_cb(
    plusplayer_h handle,
    plusplayer_adaptive_streaming_control_event_cb
        adaptive_streaming_control_event_cb,
    void* userdata) {
  if (!g_plusplayer_capi.set_adaptive_streaming_control_event_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_adaptive_streaming_control_event_cb(
      handle, adaptive_streaming_control_event_cb, userdata);
}

int plusplayer_drm_license_acquired_done(plusplayer_h handle,
                                         plusplayer_track_type_e track_type) {
  if (!g_plusplayer_capi.drm_license_acquired_done) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.drm_license_acquired_done(handle, track_type);
}

int plusplayer_set_subtitle_path(plusplayer_h handle, const char* uri) {
  if (!g_plusplayer_capi.set_subtitle_path) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_subtitle_path(handle, uri);
}

int plusplayer_set_video_stillmode(plusplayer_h handle,
                                   plusplayer_still_mode_e stillmode) {
  if (!g_plusplayer_capi.set_video_stillmode) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_video_stillmode(handle, stillmode);
}

int plusplayer_set_alternative_video_resource(plusplayer_h handle,
                                              unsigned int rsc_type) {
  if (!g_plusplayer_capi.set_alternative_video_resource) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_alternative_video_resource(handle, rsc_type);
}

int plusplayer_get_foreach_track(plusplayer_h handle,
                                 plusplayer_track_cb track_cb, void* userdata) {
  if (!g_plusplayer_capi.get_foreach_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_foreach_track(handle, track_cb, userdata);
}

int plusplayer_get_foreach_active_track(plusplayer_h handle,
                                        plusplayer_track_cb track_cb,
                                        void* userdata) {
  if (!g_plusplayer_capi.get_foreach_active_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_foreach_active_track(handle, track_cb, userdata);
}

int plusplayer_set_cookie(plusplayer_h handle, const char* cookie) {
  if (!g_plusplayer_capi.set_cookie) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_cookie(handle, cookie);
}

int plusplayer_set_user_agent(plusplayer_h handle, const char* user_agent) {
  if (!g_plusplayer_capi.set_user_agent) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_user_agent(handle, user_agent);
}

int plusplayer_set_resume_time(plusplayer_h handle, uint64_t resume_time_ms) {
  if (!g_plusplayer_capi.set_resume_time) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.set_resume_time(handle, resume_time_ms);
}

int plusplayer_is_live_streaming(plusplayer_h handle, bool* is_live) {
  if (!g_plusplayer_capi.is_live_streaming) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.is_live_streaming(handle, is_live);
}

int plusplayer_get_dvr_seekable_range(plusplayer_h handle,
                                      uint64_t* start_time_ms,
                                      uint64_t* end_time_ms) {
  if (!g_plusplayer_capi.get_dvr_seekable_range) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_dvr_seekable_range(handle, start_time_ms,
                                                  end_time_ms);
}

int plusplayer_get_current_bandwidth(plusplayer_h handle,
                                     uint32_t* curr_bandwidth_bps) {
  if (!g_plusplayer_capi.get_current_bandwidth) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_current_bandwidth(handle, curr_bandwidth_bps);
}

// Track CAPI implementation functions
int plusplayer_get_track_index(plusplayer_track_h track, int* track_index) {
  if (!g_plusplayer_capi.get_track_index) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_index(track, track_index);
}

int plusplayer_get_track_id(plusplayer_track_h track, int* track_id) {
  if (!g_plusplayer_capi.get_track_id) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_id(track, track_id);
}

int plusplayer_get_track_mimetype(plusplayer_track_h track,
                                  const char** track_mimetype) {
  if (!g_plusplayer_capi.get_track_mimetype) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_mimetype(track, track_mimetype);
}

int plusplayer_get_track_streamtype(plusplayer_track_h track,
                                    const char** track_streamtype) {
  if (!g_plusplayer_capi.get_track_streamtype) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_streamtype(track, track_streamtype);
}

int plusplayer_get_track_container_type(plusplayer_track_h track,
                                        const char** track_containertype) {
  if (!g_plusplayer_capi.get_track_container_type) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_container_type(track, track_containertype);
}

int plusplayer_get_track_type(plusplayer_track_h track,
                              plusplayer_track_type_e* track_type) {
  if (!g_plusplayer_capi.get_track_type) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_type(track, track_type);
}

int plusplayer_get_track_codec_data(plusplayer_track_h track,
                                    const char** track_codecdata) {
  if (!g_plusplayer_capi.get_track_codec_data) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_codec_data(track, track_codecdata);
}

int plusplayer_get_track_codec_tag(plusplayer_track_h track,
                                   unsigned int* track_codectag) {
  if (!g_plusplayer_capi.get_track_codec_tag) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_codec_tag(track, track_codectag);
}

int plusplayer_get_track_codec_data_len(plusplayer_track_h track,
                                        int* track_codecdatalen) {
  if (!g_plusplayer_capi.get_track_codec_data_len) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_codec_data_len(track, track_codecdatalen);
}

int plusplayer_get_track_width(plusplayer_track_h track, int* track_width) {
  if (!g_plusplayer_capi.get_track_width) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_width(track, track_width);
}

int plusplayer_get_track_height(plusplayer_track_h track, int* track_height) {
  if (!g_plusplayer_capi.get_track_height) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_height(track, track_height);
}

int plusplayer_get_track_maxwidth(plusplayer_track_h track,
                                  int* track_maxwidth) {
  if (!g_plusplayer_capi.get_track_maxwidth) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_maxwidth(track, track_maxwidth);
}

int plusplayer_get_track_maxheight(plusplayer_track_h track,
                                   int* track_maxheight) {
  if (!g_plusplayer_capi.get_track_maxheight) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_maxheight(track, track_maxheight);
}

int plusplayer_get_track_framerate_num(plusplayer_track_h track,
                                       int* track_framerate_num) {
  if (!g_plusplayer_capi.get_track_framerate_num) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_framerate_num(track, track_framerate_num);
}

int plusplayer_get_track_framerate_den(plusplayer_track_h track,
                                       int* track_framerate_den) {
  if (!g_plusplayer_capi.get_track_framerate_den) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_framerate_den(track, track_framerate_den);
}

int plusplayer_get_track_sample_rate(plusplayer_track_h track,
                                     int* track_sample_rate) {
  if (!g_plusplayer_capi.get_track_sample_rate) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_sample_rate(track, track_sample_rate);
}

int plusplayer_get_track_sample_format(plusplayer_track_h track,
                                       int* track_sample_format) {
  if (!g_plusplayer_capi.get_track_sample_format) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_sample_format(track, track_sample_format);
}

int plusplayer_get_track_channels(plusplayer_track_h track,
                                  int* track_channels) {
  if (!g_plusplayer_capi.get_track_channels) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_channels(track, track_channels);
}

int plusplayer_get_track_version(plusplayer_track_h track, int* track_version) {
  if (!g_plusplayer_capi.get_track_version) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_version(track, track_version);
}

int plusplayer_get_track_layer(plusplayer_track_h track, int* track_layer) {
  if (!g_plusplayer_capi.get_track_layer) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_layer(track, track_layer);
}

int plusplayer_get_track_bits_per_sample(plusplayer_track_h track,
                                         int* track_bits_per_sample) {
  if (!g_plusplayer_capi.get_track_bits_per_sample) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_bits_per_sample(track,
                                                     track_bits_per_sample);
}

int plusplayer_get_track_block_align(plusplayer_track_h track,
                                     int* track_block_align) {
  if (!g_plusplayer_capi.get_track_block_align) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_block_align(track, track_block_align);
}

int plusplayer_get_track_bitrate(plusplayer_track_h track, int* track_bitrate) {
  if (!g_plusplayer_capi.get_track_bitrate) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_bitrate(track, track_bitrate);
}

int plusplayer_get_track_endianness(plusplayer_track_h track,
                                    int* track_endianness) {
  if (!g_plusplayer_capi.get_track_endianness) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_endianness(track, track_endianness);
}

int plusplayer_get_track_is_signed(plusplayer_track_h track,
                                   bool* track_is_signed) {
  if (!g_plusplayer_capi.get_track_is_signed) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_is_signed(track, track_is_signed);
}

int plusplayer_get_track_active(plusplayer_track_h track, bool* track_active) {
  if (!g_plusplayer_capi.get_track_active) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_active(track, track_active);
}

int plusplayer_get_track_lang_code(plusplayer_track_h track,
                                   const char** track_lang_code) {
  if (!g_plusplayer_capi.get_track_lang_code) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_lang_code(track, track_lang_code);
}

int plusplayer_get_track_subtitle_format(plusplayer_track_h track,
                                         const char** track_subtitle_format) {
  if (!g_plusplayer_capi.get_track_subtitle_format) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return g_plusplayer_capi.get_track_subtitle_format(track,
                                                     track_subtitle_format);
}
