// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plusplayer_capi/plusplayer_capi.h"

#include <dlfcn.h>
#include <pthread.h>
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
typedef int (*fun_plusplayer_deactivate_track)(
    plusplayer_h handle, plusplayer_track_type_e track_type);
typedef int (*fun_plusplayer_activate_track)(
    plusplayer_h handle, plusplayer_track_type_e track_type);
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
typedef int (*fun_plusplayer_set_volume)(plusplayer_h handle, int volume);
typedef int (*fun_plusplayer_set_display_visible)(plusplayer_h handle,
                                                  bool is_visible);

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
  fun_plusplayer_deactivate_track deactivate_track;
  fun_plusplayer_activate_track activate_track;
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
  fun_plusplayer_set_volume set_volume;
  fun_plusplayer_set_display_visible set_display_visible;
} plusplayer_capi_t;

class PlusPlayerLoader {
 public:
  static PlusPlayerLoader& GetInstance() {
    static PlusPlayerLoader instance;
    return instance;
  }

  int Initialize() {
    pthread_mutex_lock(&mutex_);

    if (initialized_) {
      pthread_mutex_unlock(&mutex_);
      return PLUSPLAYER_ERROR_TYPE_NONE;
    }

    library_handle_ = dlopen("libplusplayer.so", RTLD_LAZY);
    if (library_handle_ == nullptr) {
      LOG_ERROR("Failed to open libplusplayer.so");
      pthread_mutex_unlock(&mutex_);
      return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
    }

    if (!LoadAllFunctionPointers()) {
      LOG_ERROR("Failed to load function pointers");
      if (library_handle_) {
        dlclose(library_handle_);
        library_handle_ = nullptr;
      }
      pthread_mutex_unlock(&mutex_);
      return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
    }

    initialized_ = true;
    pthread_mutex_unlock(&mutex_);
    return PLUSPLAYER_ERROR_TYPE_NONE;
  }

  bool IsInitialized() const { return initialized_; }

  void Cleanup() {
    pthread_mutex_lock(&mutex_);

    if (library_handle_) {
      dlclose(library_handle_);
      library_handle_ = nullptr;
    }

    memset(&function_pointers_, 0, sizeof(function_pointers_));
    initialized_ = false;

    pthread_mutex_unlock(&mutex_);
  }

  const plusplayer_capi_t* GetFunctionPointers() const {
    return &function_pointers_;
  }

 private:
  PlusPlayerLoader() : initialized_(false), library_handle_(nullptr) {
    pthread_mutex_init(&mutex_, nullptr);
    memset(&function_pointers_, 0, sizeof(function_pointers_));
  }

  ~PlusPlayerLoader() {
    Cleanup();
    pthread_mutex_destroy(&mutex_);
  }

  PlusPlayerLoader(const PlusPlayerLoader&) = delete;
  PlusPlayerLoader& operator=(const PlusPlayerLoader&) = delete;

  void* LoadFunction(const char* func_name) {
    if (!library_handle_) {
      LOG_ERROR("library_handle is invalid.");
      return nullptr;
    }

    void* func = dlsym(library_handle_, func_name);
    if (!func) {
      LOG_ERROR("Failed to find %s function.", func_name);
      return nullptr;
    }

    return func;
  }

  bool LoadAllFunctionPointers() {
    function_pointers_.create =
        (fun_plusplayer_create)LoadFunction("plusplayer_create");
    function_pointers_.open =
        (fun_plusplayer_open)LoadFunction("plusplayer_open");
    function_pointers_.prepare =
        (fun_plusplayer_prepare)LoadFunction("plusplayer_prepare");
    function_pointers_.start =
        (fun_plusplayer_start)LoadFunction("plusplayer_start");
    function_pointers_.stop =
        (fun_plusplayer_stop)LoadFunction("plusplayer_stop");
    function_pointers_.close =
        (fun_plusplayer_close)LoadFunction("plusplayer_close");
    function_pointers_.destroy =
        (fun_plusplayer_destroy)LoadFunction("plusplayer_destroy");
    function_pointers_.get_state =
        (fun_plusplayer_get_state)LoadFunction("plusplayer_get_state");
    function_pointers_.set_display =
        (fun_plusplayer_set_display)LoadFunction("plusplayer_set_display");
    function_pointers_.set_display_subsurface =
        (fun_plusplayer_set_display_subsurface)LoadFunction(
            "plusplayer_set_display_subsurface");
    function_pointers_.set_prepare_async_done_cb =
        (fun_plusplayer_set_prepare_async_done_cb)LoadFunction(
            "plusplayer_set_prepare_async_done_cb");
    function_pointers_.set_resource_conflicted_cb =
        (fun_plusplayer_set_resource_conflicted_cb)LoadFunction(
            "plusplayer_set_resource_conflicted_cb");
    function_pointers_.set_eos_cb =
        (fun_plusplayer_set_eos_cb)LoadFunction("plusplayer_set_eos_cb");
    function_pointers_.set_buffer_status_cb =
        (fun_plusplayer_set_buffer_status_cb)LoadFunction(
            "plusplayer_set_buffer_status_cb");
    function_pointers_.set_error_cb =
        (fun_plusplayer_set_error_cb)LoadFunction("plusplayer_set_error_cb");
    function_pointers_.set_error_msg_cb =
        (fun_plusplayer_set_error_msg_cb)LoadFunction(
            "plusplayer_set_error_msg_cb");
    function_pointers_.set_seek_done_cb =
        (fun_plusplayer_set_seek_done_cb)LoadFunction(
            "plusplayer_set_seek_done_cb");
    function_pointers_.set_subtitle_updated_cb =
        (fun_plusplayer_set_subtitle_updated_cb)LoadFunction(
            "plusplayer_set_subtitle_updated_cb");
    function_pointers_.set_ad_event_cb =
        (fun_plusplayer_set_ad_event_cb)LoadFunction(
            "plusplayer_set_ad_event_cb");
    function_pointers_.prepare_async =
        (fun_plusplayer_prepare_async)LoadFunction("plusplayer_prepare_async");
    function_pointers_.pause =
        (fun_plusplayer_pause)LoadFunction("plusplayer_pause");
    function_pointers_.resume =
        (fun_plusplayer_resume)LoadFunction("plusplayer_resume");
    function_pointers_.seek =
        (fun_plusplayer_seek)LoadFunction("plusplayer_seek");
    function_pointers_.set_prebuffer_mode =
        (fun_plusplayer_set_prebuffer_mode)LoadFunction(
            "plusplayer_set_prebuffer_mode");
    function_pointers_.set_app_id =
        (fun_plusplayer_set_app_id)LoadFunction("plusplayer_set_app_id");
    function_pointers_.suspend =
        (fun_plusplayer_suspend)LoadFunction("plusplayer_suspend");
    function_pointers_.restore =
        (fun_plusplayer_restore)LoadFunction("plusplayer_restore");
    function_pointers_.get_playing_time =
        (fun_plusplayer_get_playing_time)LoadFunction(
            "plusplayer_get_playing_time");
    function_pointers_.set_display_mode =
        (fun_plusplayer_set_display_mode)LoadFunction(
            "plusplayer_set_display_mode");
    function_pointers_.set_display_roi =
        (fun_plusplayer_set_display_roi)LoadFunction(
            "plusplayer_set_display_roi");
    function_pointers_.set_display_rotation =
        (fun_plusplayer_set_display_rotation)LoadFunction(
            "plusplayer_set_display_rotation");
    function_pointers_.set_buffer_config =
        (fun_plusplayer_set_buffer_config)LoadFunction(
            "plusplayer_set_buffer_config");
    function_pointers_.get_duration =
        (fun_plusplayer_get_duration)LoadFunction("plusplayer_get_duration");
    function_pointers_.set_playback_rate =
        (fun_plusplayer_set_playback_rate)LoadFunction(
            "plusplayer_set_playback_rate");
    function_pointers_.deactivate_track =
        (fun_plusplayer_deactivate_track)LoadFunction(
            "plusplayer_deactivate_track");
    function_pointers_.activate_track =
        (fun_plusplayer_activate_track)LoadFunction(
            "plusplayer_activate_track");
    function_pointers_.set_property =
        (fun_plusplayer_set_property)LoadFunction("plusplayer_set_property");
    function_pointers_.get_property =
        (fun_plusplayer_get_property)LoadFunction("plusplayer_get_property");
    function_pointers_.get_track_count =
        (fun_plusplayer_get_track_count)LoadFunction(
            "plusplayer_get_track_count");
    function_pointers_.select_track =
        (fun_plusplayer_select_track)LoadFunction("plusplayer_select_track");
    function_pointers_.get_track_language_code =
        (fun_plusplayer_get_track_language_code)LoadFunction(
            "plusplayer_get_track_language_code");
    function_pointers_.get_track_index =
        (fun_plusplayer_get_track_index)LoadFunction(
            "plusplayer_get_track_index");
    function_pointers_.get_track_id =
        (fun_plusplayer_get_track_id)LoadFunction("plusplayer_get_track_id");
    function_pointers_.get_track_mimetype =
        (fun_plusplayer_get_track_mimetype)LoadFunction(
            "plusplayer_get_track_mimetype");
    function_pointers_.get_track_streamtype =
        (fun_plusplayer_get_track_streamtype)LoadFunction(
            "plusplayer_get_track_streamtype");
    function_pointers_.get_track_container_type =
        (fun_plusplayer_get_track_container_type)LoadFunction(
            "plusplayer_get_track_container_type");
    function_pointers_.get_track_type =
        (fun_plusplayer_get_track_type)LoadFunction(
            "plusplayer_get_track_type");
    function_pointers_.get_track_codec_data =
        (fun_plusplayer_get_track_codec_data)LoadFunction(
            "plusplayer_get_track_codec_data");
    function_pointers_.get_track_codec_tag =
        (fun_plusplayer_get_track_codec_tag)LoadFunction(
            "plusplayer_get_track_codec_tag");
    function_pointers_.get_track_codec_data_len =
        (fun_plusplayer_get_track_codec_data_len)LoadFunction(
            "plusplayer_get_track_codec_data_len");
    function_pointers_.get_track_width =
        (fun_plusplayer_get_track_width)LoadFunction(
            "plusplayer_get_track_width");
    function_pointers_.get_track_height =
        (fun_plusplayer_get_track_height)LoadFunction(
            "plusplayer_get_track_height");
    function_pointers_.get_track_maxwidth =
        (fun_plusplayer_get_track_maxwidth)LoadFunction(
            "plusplayer_get_track_maxwidth");
    function_pointers_.get_track_maxheight =
        (fun_plusplayer_get_track_maxheight)LoadFunction(
            "plusplayer_get_track_maxheight");
    function_pointers_.get_track_framerate_num =
        (fun_plusplayer_get_track_framerate_num)LoadFunction(
            "plusplayer_get_track_framerate_num");
    function_pointers_.get_track_framerate_den =
        (fun_plusplayer_get_track_framerate_den)LoadFunction(
            "plusplayer_get_track_framerate_den");
    function_pointers_.get_track_sample_rate =
        (fun_plusplayer_get_track_sample_rate)LoadFunction(
            "plusplayer_get_track_sample_rate");
    function_pointers_.get_track_sample_format =
        (fun_plusplayer_get_track_sample_format)LoadFunction(
            "plusplayer_get_track_sample_format");
    function_pointers_.get_track_channels =
        (fun_plusplayer_get_track_channels)LoadFunction(
            "plusplayer_get_track_channels");
    function_pointers_.get_track_version =
        (fun_plusplayer_get_track_version)LoadFunction(
            "plusplayer_get_track_version");
    function_pointers_.get_track_layer =
        (fun_plusplayer_get_track_layer)LoadFunction(
            "plusplayer_get_track_layer");
    function_pointers_.get_track_bits_per_sample =
        (fun_plusplayer_get_track_bits_per_sample)LoadFunction(
            "plusplayer_get_track_bits_per_sample");
    function_pointers_.get_track_block_align =
        (fun_plusplayer_get_track_block_align)LoadFunction(
            "plusplayer_get_track_block_align");
    function_pointers_.get_track_bitrate =
        (fun_plusplayer_get_track_bitrate)LoadFunction(
            "plusplayer_get_track_bitrate");
    function_pointers_.get_track_endianness =
        (fun_plusplayer_get_track_endianness)LoadFunction(
            "plusplayer_get_track_endianness");
    function_pointers_.get_track_is_signed =
        (fun_plusplayer_get_track_is_signed)LoadFunction(
            "plusplayer_get_track_is_signed");
    function_pointers_.get_track_active =
        (fun_plusplayer_get_track_active)LoadFunction(
            "plusplayer_get_track_active");
    function_pointers_.get_track_lang_code =
        (fun_plusplayer_get_track_lang_code)LoadFunction(
            "plusplayer_get_track_lang_code");
    function_pointers_.get_track_subtitle_format =
        (fun_plusplayer_get_track_subtitle_format)LoadFunction(
            "plusplayer_get_track_subtitle_format");
    function_pointers_.set_app_info =
        (fun_plusplayer_set_app_info)LoadFunction("plusplayer_set_app_info");
    function_pointers_.set_drm =
        (fun_plusplayer_set_drm)LoadFunction("plusplayer_set_drm");
    function_pointers_.set_drm_init_data_cb =
        (fun_plusplayer_set_drm_init_data_cb)LoadFunction(
            "plusplayer_set_drm_init_data_cb");
    function_pointers_.set_adaptive_streaming_control_event_cb =
        (fun_plusplayer_set_adaptive_streaming_control_event_cb)LoadFunction(
            "plusplayer_set_adaptive_streaming_control_event_cb");
    function_pointers_.drm_license_acquired_done =
        (fun_plusplayer_drm_license_acquired_done)LoadFunction(
            "plusplayer_drm_license_acquired_done");
    function_pointers_.set_subtitle_path =
        (fun_plusplayer_set_subtitle_path)LoadFunction(
            "plusplayer_set_subtitle_path");
    function_pointers_.set_video_stillmode =
        (fun_plusplayer_set_video_stillmode)LoadFunction(
            "plusplayer_set_video_stillmode");
    function_pointers_.set_alternative_video_resource =
        (fun_plusplayer_set_alternative_video_resource)LoadFunction(
            "plusplayer_set_alternative_video_resource");
    function_pointers_.get_foreach_track =
        (fun_plusplayer_get_foreach_track)LoadFunction(
            "plusplayer_get_foreach_track");
    function_pointers_.get_foreach_active_track =
        (fun_plusplayer_get_foreach_active_track)LoadFunction(
            "plusplayer_get_foreach_active_track");
    function_pointers_.set_cookie =
        (fun_plusplayer_set_cookie)LoadFunction("plusplayer_set_cookie");
    function_pointers_.set_user_agent =
        (fun_plusplayer_set_user_agent)LoadFunction(
            "plusplayer_set_user_agent");
    function_pointers_.set_resume_time =
        (fun_plusplayer_set_resume_time)LoadFunction(
            "plusplayer_set_resume_time");
    function_pointers_.is_live_streaming =
        (fun_plusplayer_is_live_streaming)LoadFunction(
            "plusplayer_is_live_streaming");
    function_pointers_.get_dvr_seekable_range =
        (fun_plusplayer_get_dvr_seekable_range)LoadFunction(
            "plusplayer_get_dvr_seekable_range");
    function_pointers_.get_current_bandwidth =
        (fun_plusplayer_get_current_bandwidth)LoadFunction(
            "plusplayer_get_current_bandwidth");
    function_pointers_.set_volume =
        (fun_plusplayer_set_volume)LoadFunction("plusplayer_set_volume");
    function_pointers_.set_display_visible =
        (fun_plusplayer_set_display_visible)LoadFunction(
            "plusplayer_set_display_visible");

    return true;
  }

 private:
  bool initialized_;
  void* library_handle_;
  pthread_mutex_t mutex_;
  plusplayer_capi_t function_pointers_;
};

static const plusplayer_capi_t* get_plusplayer_functions() {
  PlusPlayerLoader& loader = PlusPlayerLoader::GetInstance();
  if (!loader.IsInitialized() &&
      loader.Initialize() != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("PlusPlayer API not available. Returning nullptr.");
    return nullptr;
  }
  return loader.GetFunctionPointers();
}

// PlusPlayer API implementation functions
plusplayer_h plusplayer_create(void) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->create) {
    LOG_ERROR("plusplayer_create API loading failed. Returning nullptr.");
    return nullptr;
  }
  return functions->create();
}

int plusplayer_open(plusplayer_h handle, const char* uri) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->open) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->open(handle, uri);
}

int plusplayer_prepare(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->prepare) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->prepare(handle);
}

int plusplayer_start(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->start) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->start(handle);
}

int plusplayer_stop(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->stop) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->stop(handle);
}

int plusplayer_close(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->close) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->close(handle);
}

int plusplayer_destroy(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->destroy) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->destroy(handle);
}

plusplayer_state_e plusplayer_get_state(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_state) {
    return (plusplayer_state_e)(-1);
  }
  return functions->get_state(handle);
}

int plusplayer_set_display(plusplayer_h handle, plusplayer_display_type_e type,
                           void* window) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_display) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_display(handle, type, window);
}

int plusplayer_set_display_subsurface(plusplayer_h handle,
                                      plusplayer_display_type_e type,
                                      uint32_t surface_id,
                                      plusplayer_geometry_s roi) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_display_subsurface) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_display_subsurface(handle, type, surface_id, roi);
}

int plusplayer_set_prepare_async_done_cb(
    plusplayer_h handle, plusplayer_prepare_async_done_cb prepare_async_done_cb,
    void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_prepare_async_done_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_prepare_async_done_cb(handle, prepare_async_done_cb,
                                              userdata);
}

int plusplayer_set_resource_conflicted_cb(
    plusplayer_h handle,
    plusplayer_resource_conflicted_cb resource_conflicted_cb, void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_resource_conflicted_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_resource_conflicted_cb(handle, resource_conflicted_cb,
                                               userdata);
}

int plusplayer_set_eos_cb(plusplayer_h handle, plusplayer_eos_cb eos_cb,
                          void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_eos_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_eos_cb(handle, eos_cb, userdata);
}

int plusplayer_set_buffer_status_cb(
    plusplayer_h handle, plusplayer_buffer_status_cb buffer_status_cb,
    void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_buffer_status_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_buffer_status_cb(handle, buffer_status_cb, userdata);
}

int plusplayer_set_error_cb(plusplayer_h handle, plusplayer_error_cb error_cb,
                            void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_error_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_error_cb(handle, error_cb, userdata);
}

int plusplayer_set_error_msg_cb(plusplayer_h handle,
                                plusplayer_error_msg_cb error_msg_cb,
                                void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_error_msg_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_error_msg_cb(handle, error_msg_cb, userdata);
}

int plusplayer_set_seek_done_cb(plusplayer_h handle,
                                plusplayer_seek_done_cb seek_done_cb,
                                void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_seek_done_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_seek_done_cb(handle, seek_done_cb, userdata);
}

int plusplayer_set_subtitle_updated_cb(
    plusplayer_h handle, plusplayer_subtitle_updated_cb subtitle_updated_cb,
    void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_subtitle_updated_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_subtitle_updated_cb(handle, subtitle_updated_cb,
                                            userdata);
}

int plusplayer_set_ad_event_cb(plusplayer_h handle,
                               plusplayer_ad_event_cb ad_event_cb,
                               void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_ad_event_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_ad_event_cb(handle, ad_event_cb, userdata);
}

int plusplayer_prepare_async(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->prepare_async) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->prepare_async(handle);
}

int plusplayer_pause(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->pause) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->pause(handle);
}

int plusplayer_resume(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->resume) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->resume(handle);
}

int plusplayer_seek(plusplayer_h handle, uint64_t time) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->seek) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->seek(handle, time);
}

void plusplayer_set_prebuffer_mode(plusplayer_h handle, bool prebuffer_mode) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (functions && functions->set_prebuffer_mode) {
    functions->set_prebuffer_mode(handle, prebuffer_mode);
  }
}

int plusplayer_set_app_id(plusplayer_h handle, const char* app_id) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_app_id) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_app_id(handle, app_id);
}

int plusplayer_suspend(plusplayer_h handle) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->suspend) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->suspend(handle);
}

int plusplayer_restore(plusplayer_h handle, plusplayer_state_e target_state) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->restore) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->restore(handle, target_state);
}

int plusplayer_get_playing_time(plusplayer_h handle, uint64_t* cur_time_ms) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_playing_time) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_playing_time(handle, cur_time_ms);
}

int plusplayer_set_display_mode(plusplayer_h handle,
                                plusplayer_display_mode_e mode) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_display_mode) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_display_mode(handle, mode);
}

int plusplayer_set_display_roi(plusplayer_h handle, plusplayer_geometry_s roi) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_display_roi) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_display_roi(handle, roi);
}

int plusplayer_set_display_rotation(
    plusplayer_h handle, plusplayer_display_rotation_type_e rotation) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_display_rotation) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_display_rotation(handle, rotation);
}

int plusplayer_set_buffer_config(plusplayer_h handle, const char* config,
                                 int amount) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_buffer_config) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_buffer_config(handle, config, amount);
}

int plusplayer_get_duration(plusplayer_h handle, int64_t* duration_ms) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_duration) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_duration(handle, duration_ms);
}

int plusplayer_set_playback_rate(plusplayer_h handle,
                                 const double playback_rate) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_playback_rate) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_playback_rate(handle, playback_rate);
}

int plusplayer_deactivate_track(plusplayer_h handle,
                                plusplayer_track_type_e track_type) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->deactivate_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->deactivate_track(handle, track_type);
}

int plusplayer_activate_track(plusplayer_h handle,
                              plusplayer_track_type_e track_type) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->activate_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->activate_track(handle, track_type);
}

int plusplayer_set_property(plusplayer_h handle, plusplayer_property_e property,
                            const char* value) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_property) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_property(handle, property, value);
}

int plusplayer_get_property(plusplayer_h handle, plusplayer_property_e property,
                            char** value) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_property) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_property(handle, property, value);
}

int plusplayer_get_track_count(plusplayer_h handle,
                               plusplayer_track_type_e track_type, int* count) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_count) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_count(handle, track_type, count);
}

int plusplayer_select_track(plusplayer_h handle, plusplayer_track_type_e type,
                            int index) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->select_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->select_track(handle, type, index);
}

const char* plusplayer_get_track_language_code(plusplayer_h handle,
                                               plusplayer_track_type_e type,
                                               int index) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_language_code) {
    return nullptr;
  }
  return functions->get_track_language_code(handle, type, index);
}

int plusplayer_set_app_info(plusplayer_h handle,
                            const plusplayer_app_info_s* app_info) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_app_info) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_app_info(handle, app_info);
}

int plusplayer_set_drm(plusplayer_h handle,
                       plusplayer_drm_property_s drm_property) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_drm) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_drm(handle, drm_property);
}

int plusplayer_set_drm_init_data_cb(
    plusplayer_h handle, plusplayer_drm_init_data_cb drm_init_data_callback,
    void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_drm_init_data_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_drm_init_data_cb(handle, drm_init_data_callback,
                                         userdata);
}

int plusplayer_set_adaptive_streaming_control_event_cb(
    plusplayer_h handle,
    plusplayer_adaptive_streaming_control_event_cb
        adaptive_streaming_control_event_cb,
    void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_adaptive_streaming_control_event_cb) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_adaptive_streaming_control_event_cb(
      handle, adaptive_streaming_control_event_cb, userdata);
}

int plusplayer_drm_license_acquired_done(plusplayer_h handle,
                                         plusplayer_track_type_e track_type) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->drm_license_acquired_done) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->drm_license_acquired_done(handle, track_type);
}

int plusplayer_set_subtitle_path(plusplayer_h handle, const char* uri) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_subtitle_path) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_subtitle_path(handle, uri);
}

int plusplayer_set_video_stillmode(plusplayer_h handle,
                                   plusplayer_still_mode_e stillmode) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_video_stillmode) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_video_stillmode(handle, stillmode);
}

int plusplayer_set_alternative_video_resource(plusplayer_h handle,
                                              unsigned int rsc_type) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_alternative_video_resource) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_alternative_video_resource(handle, rsc_type);
}

int plusplayer_get_foreach_track(plusplayer_h handle,
                                 plusplayer_track_cb track_cb, void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_foreach_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_foreach_track(handle, track_cb, userdata);
}

int plusplayer_get_foreach_active_track(plusplayer_h handle,
                                        plusplayer_track_cb track_cb,
                                        void* userdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_foreach_active_track) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_foreach_active_track(handle, track_cb, userdata);
}

int plusplayer_set_cookie(plusplayer_h handle, const char* cookie) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_cookie) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_cookie(handle, cookie);
}

int plusplayer_set_user_agent(plusplayer_h handle, const char* user_agent) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_user_agent) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_user_agent(handle, user_agent);
}

int plusplayer_set_resume_time(plusplayer_h handle, uint64_t resume_time_ms) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_resume_time) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_resume_time(handle, resume_time_ms);
}

int plusplayer_is_live_streaming(plusplayer_h handle, bool* is_live) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->is_live_streaming) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->is_live_streaming(handle, is_live);
}

int plusplayer_get_dvr_seekable_range(plusplayer_h handle,
                                      uint64_t* start_time_ms,
                                      uint64_t* end_time_ms) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_dvr_seekable_range) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_dvr_seekable_range(handle, start_time_ms, end_time_ms);
}

int plusplayer_get_current_bandwidth(plusplayer_h handle,
                                     uint32_t* curr_bandwidth_bps) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_current_bandwidth) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_current_bandwidth(handle, curr_bandwidth_bps);
}

// Track CAPI implementation functions
int plusplayer_get_track_index(plusplayer_track_h track, int* track_index) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_index) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_index(track, track_index);
}

int plusplayer_get_track_id(plusplayer_track_h track, int* track_id) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_id) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_id(track, track_id);
}

int plusplayer_get_track_mimetype(plusplayer_track_h track,
                                  const char** track_mimetype) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_mimetype) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_mimetype(track, track_mimetype);
}

int plusplayer_get_track_streamtype(plusplayer_track_h track,
                                    const char** track_streamtype) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_streamtype) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_streamtype(track, track_streamtype);
}

int plusplayer_get_track_container_type(plusplayer_track_h track,
                                        const char** track_containertype) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_container_type) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_container_type(track, track_containertype);
}

int plusplayer_get_track_type(plusplayer_track_h track,
                              plusplayer_track_type_e* track_type) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_type) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_type(track, track_type);
}

int plusplayer_get_track_codec_data(plusplayer_track_h track,
                                    const char** track_codecdata) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_codec_data) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_codec_data(track, track_codecdata);
}

int plusplayer_get_track_codec_tag(plusplayer_track_h track,
                                   unsigned int* track_codectag) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_codec_tag) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_codec_tag(track, track_codectag);
}

int plusplayer_get_track_codec_data_len(plusplayer_track_h track,
                                        int* track_codecdatalen) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_codec_data_len) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_codec_data_len(track, track_codecdatalen);
}

int plusplayer_get_track_width(plusplayer_track_h track, int* track_width) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_width) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_width(track, track_width);
}

int plusplayer_get_track_height(plusplayer_track_h track, int* track_height) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_height) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_height(track, track_height);
}

int plusplayer_get_track_maxwidth(plusplayer_track_h track,
                                  int* track_maxwidth) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_maxwidth) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_maxwidth(track, track_maxwidth);
}

int plusplayer_get_track_maxheight(plusplayer_track_h track,
                                   int* track_maxheight) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_maxheight) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_maxheight(track, track_maxheight);
}

int plusplayer_get_track_framerate_num(plusplayer_track_h track,
                                       int* track_framerate_num) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_framerate_num) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_framerate_num(track, track_framerate_num);
}

int plusplayer_get_track_framerate_den(plusplayer_track_h track,
                                       int* track_framerate_den) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_framerate_den) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_framerate_den(track, track_framerate_den);
}

int plusplayer_get_track_sample_rate(plusplayer_track_h track,
                                     int* track_sample_rate) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_sample_rate) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_sample_rate(track, track_sample_rate);
}

int plusplayer_get_track_sample_format(plusplayer_track_h track,
                                       int* track_sample_format) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_sample_format) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_sample_format(track, track_sample_format);
}

int plusplayer_get_track_channels(plusplayer_track_h track,
                                  int* track_channels) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_channels) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_channels(track, track_channels);
}

int plusplayer_get_track_version(plusplayer_track_h track, int* track_version) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_version) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_version(track, track_version);
}

int plusplayer_get_track_layer(plusplayer_track_h track, int* track_layer) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_layer) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_layer(track, track_layer);
}

int plusplayer_get_track_bits_per_sample(plusplayer_track_h track,
                                         int* track_bits_per_sample) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_bits_per_sample) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_bits_per_sample(track, track_bits_per_sample);
}

int plusplayer_get_track_block_align(plusplayer_track_h track,
                                     int* track_block_align) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_block_align) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_block_align(track, track_block_align);
}

int plusplayer_get_track_bitrate(plusplayer_track_h track, int* track_bitrate) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_bitrate) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_bitrate(track, track_bitrate);
}

int plusplayer_get_track_endianness(plusplayer_track_h track,
                                    int* track_endianness) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_endianness) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_endianness(track, track_endianness);
}

int plusplayer_get_track_is_signed(plusplayer_track_h track,
                                   bool* track_is_signed) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_is_signed) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_is_signed(track, track_is_signed);
}

int plusplayer_get_track_active(plusplayer_track_h track, bool* track_active) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_active) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_active(track, track_active);
}

int plusplayer_get_track_lang_code(plusplayer_track_h track,
                                   const char** track_lang_code) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_lang_code) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_lang_code(track, track_lang_code);
}

int plusplayer_get_track_subtitle_format(plusplayer_track_h track,
                                         const char** track_subtitle_format) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->get_track_subtitle_format) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->get_track_subtitle_format(track, track_subtitle_format);
}

int plusplayer_set_volume(plusplayer_h handle, int volume) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_volume) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_volume(handle, volume);
}

int plusplayer_set_display_visible(plusplayer_h handle, bool is_visible) {
  const plusplayer_capi_t* functions = get_plusplayer_functions();
  if (!functions || !functions->set_display_visible) {
    return PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION;
  }
  return functions->set_display_visible(handle, is_visible);
}
