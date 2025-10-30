// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plus_player_capi_proxy.h"

#include <dlfcn.h>

#include <utility>

#include "log.h"

// Template function for dynamic library function calls with custom error values
template <typename FuncType>
FuncType CallPlusplayerFunction(void* handle, const char* func_name) {
  if (!handle) {
    LOG_ERROR("plusplayer_capi_handle_ is invalid.");
    return nullptr;
  }

  FuncType func = reinterpret_cast<FuncType>(dlsym(handle, func_name));
  if (!func) {
    LOG_ERROR("Failed to find %s function.", func_name);
    return nullptr;
  }

  return func;
}

PlusPlayerCapiProxy::PlusPlayerCapiProxy() {
  plusplayer_capi_handle_ = dlopen("libplusplayer.so", RTLD_LAZY);
  if (plusplayer_capi_handle_ == nullptr) {
    LOG_ERROR("Failed to open libplusplayer.so");
  }
}

PlusPlayerCapiProxy::~PlusPlayerCapiProxy() {
  if (plusplayer_capi_handle_) {
    dlclose(plusplayer_capi_handle_);
    plusplayer_capi_handle_ = nullptr;
  }
}

plusplayer_h PlusPlayerCapiProxy::plusplayer_capi_create(void) {
  FunPlusplayerCapiCreate plusplayer_capi_create =
      CallPlusplayerFunction<FunPlusplayerCapiCreate>(plusplayer_capi_handle_,
                                                      "plusplayer_create");
  if (plusplayer_capi_create) {
    return plusplayer_capi_create();
  }
  return nullptr;
}

int PlusPlayerCapiProxy::plusplayer_capi_open(plusplayer_h handle,
                                              const char* uri) {
  FunPlusplayerCapiOpen plusplayer_capi_open =
      CallPlusplayerFunction<FunPlusplayerCapiOpen>(plusplayer_capi_handle_,
                                                    "plusplayer_open");
  if (plusplayer_capi_open) {
    return plusplayer_capi_open(handle, uri);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
  // if (!plusplayer_capi_handle_) {
  //   LOG_ERROR("power_state_handle_ is invalid.");
  //   return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
  // }

  // FunPlusplayerCapiOpen plusplayer_capi_open =
  //     reinterpret_cast<FunPlusplayerCapiOpen>(
  //         dlsym(plusplayer_capi_handle_, "plusplayer_open"));

  // if (!plusplayer_capi_open) {
  //   LOG_ERROR("Failed to find plusplayer_open function.");
  //   return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
  // }

  // return plusplayer_capi_open(handle, uri);
}

plusplayer_state_e PlusPlayerCapiProxy::plusplayer_capi_get_state(
    plusplayer_h handle) {
  FunPlusplayerCapiGetState plusplayer_capi_get_state =
      CallPlusplayerFunction<FunPlusplayerCapiGetState>(plusplayer_capi_handle_,
                                                        "plusplayer_get_state");
  if (plusplayer_capi_get_state) {
    return plusplayer_capi_get_state(handle);
  }
  return static_cast<plusplayer_state_e>(-1);
}

int PlusPlayerCapiProxy::plusplayer_capi_set_property(
    plusplayer_h handle, plusplayer_property_e property, const char* value) {
  FunPlusplayerCapiSetProperty plusplayer_capi_set_property =
      CallPlusplayerFunction<FunPlusplayerCapiSetProperty>(
          plusplayer_capi_handle_, "plusplayer_set_property");
  if (plusplayer_capi_set_property) {
    return plusplayer_capi_set_property(handle, property, value);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_app_id(plusplayer_h handle,
                                                    const char* app_id) {
  FunPlusplayerCapiSetAppId plusplayer_capi_set_app_id =
      CallPlusplayerFunction<FunPlusplayerCapiSetAppId>(
          plusplayer_capi_handle_, "plusplayer_set_app_id");
  if (plusplayer_capi_set_app_id) {
    return plusplayer_capi_set_app_id(handle, app_id);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

void PlusPlayerCapiProxy::plusplayer_capi_set_prebuffer_mode(
    plusplayer_h handle, bool prebuffer_mode) {
  FunPlusplayerCapiSetPrebufferMode plusplayer_capi_set_prebuffer_mode =
      CallPlusplayerFunction<FunPlusplayerCapiSetPrebufferMode>(
          plusplayer_capi_handle_, "plusplayer_set_prebuffer_mode");
  if (plusplayer_capi_set_prebuffer_mode) {
    return plusplayer_capi_set_prebuffer_mode(handle, prebuffer_mode);
  }
}

int PlusPlayerCapiProxy::plusplayer_capi_prepare(plusplayer_h handle) {
  FunPlusplayerCapiPrepare plusplayer_capi_prepare =
      CallPlusplayerFunction<FunPlusplayerCapiPrepare>(plusplayer_capi_handle_,
                                                       "plusplayer_prepare");
  if (plusplayer_capi_prepare) {
    return plusplayer_capi_prepare(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_start(plusplayer_h handle) {
  FunPlusplayerCapiStart plusplayer_capi_start =
      CallPlusplayerFunction<FunPlusplayerCapiStart>(plusplayer_capi_handle_,
                                                     "plusplayer_start");
  if (plusplayer_capi_start) {
    return plusplayer_capi_start(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_stop(plusplayer_h handle) {
  FunPlusplayerCapiStop plusplayer_capi_stop =
      CallPlusplayerFunction<FunPlusplayerCapiStop>(plusplayer_capi_handle_,
                                                    "plusplayer_stop");
  if (plusplayer_capi_stop) {
    return plusplayer_capi_stop(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_close(plusplayer_h handle) {
  FunPlusplayerCapiClose plusplayer_capi_close =
      CallPlusplayerFunction<FunPlusplayerCapiClose>(plusplayer_capi_handle_,
                                                     "plusplayer_close");
  if (plusplayer_capi_close) {
    return plusplayer_capi_close(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_destroy(plusplayer_h handle) {
  FunPlusplayerCapiDestroy plusplayer_capi_destroy =
      CallPlusplayerFunction<FunPlusplayerCapiDestroy>(plusplayer_capi_handle_,
                                                       "plusplayer_destroy");
  if (plusplayer_capi_destroy) {
    return plusplayer_capi_destroy(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_display(
    plusplayer_h handle, plusplayer_display_type_e type, void* window) {
  FunPlusplayerCapiSetDisplay plusplayer_capi_set_display =
      CallPlusplayerFunction<FunPlusplayerCapiSetDisplay>(
          plusplayer_capi_handle_, "plusplayer_set_display");
  if (plusplayer_capi_set_display) {
    return plusplayer_capi_set_display(handle, type, window);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_display_subsurface(
    plusplayer_h handle, plusplayer_display_type_e type, uint32_t surface_id,
    plusplayer_geometry_s roi) {
  FunPlusplayerCapiSetDisplaySubsurface plusplayer_capi_set_display_subsurface =
      CallPlusplayerFunction<FunPlusplayerCapiSetDisplaySubsurface>(
          plusplayer_capi_handle_, "plusplayer_set_display_subsurface");
  if (plusplayer_capi_set_display_subsurface) {
    return plusplayer_capi_set_display_subsurface(handle, type, surface_id,
                                                  roi);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_prepare_async_done_cb(
    plusplayer_h handle, plusplayer_prepare_async_done_cb prepare_async_done_cb,
    void* userdata) {
  FunPlusplayerCapiSetPrepareAsyncDoneCb
      plusplayer_capi_set_prepare_async_done_cb =
          CallPlusplayerFunction<FunPlusplayerCapiSetPrepareAsyncDoneCb>(
              plusplayer_capi_handle_, "plusplayer_set_prepare_async_done_cb");
  if (plusplayer_capi_set_prepare_async_done_cb) {
    return plusplayer_capi_set_prepare_async_done_cb(
        handle, prepare_async_done_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_resource_conflicted_cb(
    plusplayer_h handle,
    plusplayer_resource_conflicted_cb resource_conflicted_cb, void* userdata) {
  FunPlusplayerCapiSetResourceConflictedCb
      plusplayer_capi_set_resource_conflicted_cb =
          CallPlusplayerFunction<FunPlusplayerCapiSetResourceConflictedCb>(
              plusplayer_capi_handle_, "plusplayer_set_resource_conflicted_cb");
  if (plusplayer_capi_set_resource_conflicted_cb) {
    return plusplayer_capi_set_resource_conflicted_cb(
        handle, resource_conflicted_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_eos_cb(plusplayer_h handle,
                                                    plusplayer_eos_cb eos_cb,
                                                    void* userdata) {
  FunPlusplayerCapiSetEosCb plusplayer_capi_set_eos_cb =
      CallPlusplayerFunction<FunPlusplayerCapiSetEosCb>(
          plusplayer_capi_handle_, "plusplayer_set_eos_cb");
  if (plusplayer_capi_set_eos_cb) {
    return plusplayer_capi_set_eos_cb(handle, eos_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_buffer_status_cb(
    plusplayer_h handle, plusplayer_buffer_status_cb buffer_status_cb,
    void* userdata) {
  FunPlusplayerCapiSetBufferStatusCb plusplayer_capi_set_buffer_status_cb =
      CallPlusplayerFunction<FunPlusplayerCapiSetBufferStatusCb>(
          plusplayer_capi_handle_, "plusplayer_set_buffer_status_cb");
  if (plusplayer_capi_set_buffer_status_cb) {
    return plusplayer_capi_set_buffer_status_cb(handle, buffer_status_cb,
                                                userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_error_cb(
    plusplayer_h handle, plusplayer_error_cb error_cb, void* userdata) {
  FunPlusplayerCapiSetErrorCb plusplayer_capi_set_error_cb =
      CallPlusplayerFunction<FunPlusplayerCapiSetErrorCb>(
          plusplayer_capi_handle_, "plusplayer_set_error_cb");
  if (plusplayer_capi_set_error_cb) {
    return plusplayer_capi_set_error_cb(handle, error_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_error_msg_cb(
    plusplayer_h handle, plusplayer_error_msg_cb error_msg_cb, void* userdata) {
  FunPlusplayerCapiSetErrorMsgCb plusplayer_capi_set_error_msg_cb =
      CallPlusplayerFunction<FunPlusplayerCapiSetErrorMsgCb>(
          plusplayer_capi_handle_, "plusplayer_set_error_msg_cb");
  if (plusplayer_capi_set_error_msg_cb) {
    return plusplayer_capi_set_error_msg_cb(handle, error_msg_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_seek_done_cb(
    plusplayer_h handle, plusplayer_seek_done_cb seek_done_cb, void* userdata) {
  FunPlusplayerCapiSetSeekDoneCb plusplayer_capi_set_seek_done_cb =
      CallPlusplayerFunction<FunPlusplayerCapiSetSeekDoneCb>(
          plusplayer_capi_handle_, "plusplayer_set_seek_done_cb");
  if (plusplayer_capi_set_seek_done_cb) {
    return plusplayer_capi_set_seek_done_cb(handle, seek_done_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_subtitle_updated_cb(
    plusplayer_h handle, plusplayer_subtitle_updated_cb subtitle_updated_cb,
    void* userdata) {
  FunPlusplayerCapiSetSubtitleUpdatedCb
      plusplayer_capi_set_subtitle_updated_cb =
          CallPlusplayerFunction<FunPlusplayerCapiSetSubtitleUpdatedCb>(
              plusplayer_capi_handle_, "plusplayer_set_subtitle_updated_cb");
  if (plusplayer_capi_set_subtitle_updated_cb) {
    return plusplayer_capi_set_subtitle_updated_cb(handle, subtitle_updated_cb,
                                                   userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_ad_event_cb(
    plusplayer_h handle, plusplayer_ad_event_cb ad_event_cb, void* userdata) {
  FunPlusplayerCapiSetAdEventCb plusplayer_capi_set_ad_event_cb =
      CallPlusplayerFunction<FunPlusplayerCapiSetAdEventCb>(
          plusplayer_capi_handle_, "plusplayer_set_ad_event_cb");
  if (plusplayer_capi_set_ad_event_cb) {
    return plusplayer_capi_set_ad_event_cb(handle, ad_event_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_prepare_async(plusplayer_h handle) {
  FunPlusplayerCapiPrepareAsync plusplayer_capi_prepare_async =
      CallPlusplayerFunction<FunPlusplayerCapiPrepareAsync>(
          plusplayer_capi_handle_, "plusplayer_prepare_async");
  if (plusplayer_capi_prepare_async) {
    return plusplayer_capi_prepare_async(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_pause(plusplayer_h handle) {
  FunPlusplayerCapiPause plusplayer_capi_pause =
      CallPlusplayerFunction<FunPlusplayerCapiPause>(plusplayer_capi_handle_,
                                                     "plusplayer_pause");
  if (plusplayer_capi_pause) {
    return plusplayer_capi_pause(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_resume(plusplayer_h handle) {
  FunPlusplayerCapiResume plusplayer_capi_resume =
      CallPlusplayerFunction<FunPlusplayerCapiResume>(plusplayer_capi_handle_,
                                                      "plusplayer_resume");
  if (plusplayer_capi_resume) {
    return plusplayer_capi_resume(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_seek(plusplayer_h handle,
                                              uint64_t time) {
  FunPlusplayerCapiSeek plusplayer_capi_seek =
      CallPlusplayerFunction<FunPlusplayerCapiSeek>(plusplayer_capi_handle_,
                                                    "plusplayer_seek");
  if (plusplayer_capi_seek) {
    return plusplayer_capi_seek(handle, time);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_suspend(plusplayer_h handle) {
  FunPlusplayerCapiSuspend plusplayer_capi_suspend =
      CallPlusplayerFunction<FunPlusplayerCapiSuspend>(plusplayer_capi_handle_,
                                                       "plusplayer_suspend");
  if (plusplayer_capi_suspend) {
    return plusplayer_capi_suspend(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_restore(
    plusplayer_h handle, plusplayer_state_e target_state) {
  FunPlusplayerCapiRestore plusplayer_capi_restore =
      CallPlusplayerFunction<FunPlusplayerCapiRestore>(plusplayer_capi_handle_,
                                                       "plusplayer_restore");
  if (plusplayer_capi_restore) {
    return plusplayer_capi_restore(handle, target_state);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_playing_time(
    plusplayer_h handle, uint64_t* cur_time_ms) {
  FunPlusplayerCapiGetPlayingTime plusplayer_capi_get_playing_time =
      CallPlusplayerFunction<FunPlusplayerCapiGetPlayingTime>(
          plusplayer_capi_handle_, "plusplayer_get_playing_time");
  if (plusplayer_capi_get_playing_time) {
    return plusplayer_capi_get_playing_time(handle, cur_time_ms);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_display_mode(
    plusplayer_h handle, plusplayer_display_mode_e mode) {
  FunPlusplayerCapiSetDisplayMode plusplayer_capi_set_display_mode =
      CallPlusplayerFunction<FunPlusplayerCapiSetDisplayMode>(
          plusplayer_capi_handle_, "plusplayer_set_display_mode");
  if (plusplayer_capi_set_display_mode) {
    return plusplayer_capi_set_display_mode(handle, mode);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_display_roi(
    plusplayer_h handle, plusplayer_geometry_s roi) {
  FunPlusplayerCapiSetDisplayRoi plusplayer_capi_set_display_roi =
      CallPlusplayerFunction<FunPlusplayerCapiSetDisplayRoi>(
          plusplayer_capi_handle_, "plusplayer_set_display_roi");
  if (plusplayer_capi_set_display_roi) {
    return plusplayer_capi_set_display_roi(handle, roi);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_display_rotation(
    plusplayer_h handle, plusplayer_display_rotation_type_e rotation) {
  FunPlusplayerCapiSetDisplayRotation plusplayer_capi_set_display_rotation =
      CallPlusplayerFunction<FunPlusplayerCapiSetDisplayRotation>(
          plusplayer_capi_handle_, "plusplayer_set_display_rotation");
  if (plusplayer_capi_set_display_rotation) {
    return plusplayer_capi_set_display_rotation(handle, rotation);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_buffer_config(plusplayer_h handle,
                                                           const char* config,
                                                           int amount) {
  FunPlusplayerCapiSetBufferConfig plusplayer_capi_set_buffer_config =
      CallPlusplayerFunction<FunPlusplayerCapiSetBufferConfig>(
          plusplayer_capi_handle_, "plusplayer_set_buffer_config");
  if (plusplayer_capi_set_buffer_config) {
    return plusplayer_capi_set_buffer_config(handle, config, amount);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_duration(plusplayer_h handle,
                                                      int64_t* duration_ms) {
  FunPlusplayerCapiGetDuration plusplayer_capi_get_duration =
      CallPlusplayerFunction<FunPlusplayerCapiGetDuration>(
          plusplayer_capi_handle_, "plusplayer_get_duration");
  if (plusplayer_capi_get_duration) {
    return plusplayer_capi_get_duration(handle, duration_ms);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_playback_rate(
    plusplayer_h handle, const double playback_rate) {
  FunPlusplayerCapiSetPlaybackRate plusplayer_capi_set_playback_rate =
      CallPlusplayerFunction<FunPlusplayerCapiSetPlaybackRate>(
          plusplayer_capi_handle_, "plusplayer_set_playback_rate");
  if (plusplayer_capi_set_playback_rate) {
    return plusplayer_capi_set_playback_rate(handle, playback_rate);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_deactivate_audio(plusplayer_h handle) {
  FunPlusplayerCapiDeactivateAudio plusplayer_capi_deactivate_audio =
      CallPlusplayerFunction<FunPlusplayerCapiDeactivateAudio>(
          plusplayer_capi_handle_, "plusplayer_deactivate_audio");
  if (plusplayer_capi_deactivate_audio) {
    return plusplayer_capi_deactivate_audio(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_activate_audio(plusplayer_h handle) {
  FunPlusplayerCapiActivateAudio plusplayer_capi_activate_audio =
      CallPlusplayerFunction<FunPlusplayerCapiActivateAudio>(
          plusplayer_capi_handle_, "plusplayer_activate_audio");
  if (plusplayer_capi_activate_audio) {
    return plusplayer_capi_activate_audio(handle);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_property(
    plusplayer_h handle, plusplayer_property_e property, char** value) {
  FunPlusplayerCapiGetProperty plusplayer_capi_get_property =
      CallPlusplayerFunction<FunPlusplayerCapiGetProperty>(
          plusplayer_capi_handle_, "plusplayer_get_property");
  if (plusplayer_capi_get_property) {
    return plusplayer_capi_get_property(handle, property, value);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_is_live_streaming(plusplayer_h handle,
                                                           bool* is_live) {
  FunPlusplayerCapiIsLiveStreaming plusplayer_capi_is_live_streaming =
      CallPlusplayerFunction<FunPlusplayerCapiIsLiveStreaming>(
          plusplayer_capi_handle_, "plusplayer_is_live_streaming");
  if (plusplayer_capi_is_live_streaming) {
    return plusplayer_capi_is_live_streaming(handle, is_live);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_dvr_seekable_range(
    plusplayer_h handle, uint64_t* start_time_ms, uint64_t* end_time_ms) {
  FunPlusplayerCapiGetDvrSeekableRange plusplayer_capi_get_dvr_seekable_range =
      CallPlusplayerFunction<FunPlusplayerCapiGetDvrSeekableRange>(
          plusplayer_capi_handle_, "plusplayer_get_dvr_seekable_range");
  if (plusplayer_capi_get_dvr_seekable_range) {
    return plusplayer_capi_get_dvr_seekable_range(handle, start_time_ms,
                                                  end_time_ms);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_current_bandwidth(
    plusplayer_h handle, uint32_t* curr_bandwidth_bps) {
  FunPlusplayerCapiGetCurrentBandwidth plusplayer_capi_get_current_bandwidth =
      CallPlusplayerFunction<FunPlusplayerCapiGetCurrentBandwidth>(
          plusplayer_capi_handle_, "plusplayer_get_current_bandwidth");
  if (plusplayer_capi_get_current_bandwidth) {
    return plusplayer_capi_get_current_bandwidth(handle, curr_bandwidth_bps);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_count(
    plusplayer_h handle, plusplayer_track_type_e track_type, int* count) {
  FunPlusplayerCapiGetTrackCount plusplayer_capi_get_track_count =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackCount>(
          plusplayer_capi_handle_, "plusplayer_get_track_count");
  if (plusplayer_capi_get_track_count) {
    return plusplayer_capi_get_track_count(handle, track_type, count);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_select_track(
    plusplayer_h handle, plusplayer_track_type_e type, int index) {
  FunPlusplayerCapiSelectTrack plusplayer_capi_select_track =
      CallPlusplayerFunction<FunPlusplayerCapiSelectTrack>(
          plusplayer_capi_handle_, "plusplayer_select_track");
  if (plusplayer_capi_select_track) {
    return plusplayer_capi_select_track(handle, type, index);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

const char* PlusPlayerCapiProxy::plusplayer_capi_get_track_language_code(
    plusplayer_h handle, plusplayer_track_type_e type, int index) {
  FunPlusplayerCapiGetTrackLanguageCode
      plusplayer_capi_get_track_language_code =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackLanguageCode>(
              plusplayer_capi_handle_, "plusplayer_get_track_language_code");
  if (plusplayer_capi_get_track_language_code) {
    return plusplayer_capi_get_track_language_code(handle, type, index);
  }
  return nullptr;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_app_info(
    plusplayer_h handle, const plusplayer_app_info_s* app_info) {
  FunPlusplayerCapiSetAppInfo plusplayer_capi_set_app_info =
      CallPlusplayerFunction<FunPlusplayerCapiSetAppInfo>(
          plusplayer_capi_handle_, "plusplayer_set_app_info");
  if (plusplayer_capi_set_app_info) {
    return plusplayer_capi_set_app_info(handle, app_info);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_drm(
    plusplayer_h handle, plusplayer_drm_property_s drm_property) {
  FunPlusplayerCapiSetDrm plusplayer_capi_set_drm =
      CallPlusplayerFunction<FunPlusplayerCapiSetDrm>(plusplayer_capi_handle_,
                                                      "plusplayer_set_drm");
  if (plusplayer_capi_set_drm) {
    return plusplayer_capi_set_drm(handle, drm_property);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_drm_init_data_cb(
    plusplayer_h handle, plusplayer_drm_init_data_cb drm_init_data_callback,
    void* userdata) {
  FunPlusplayerCapiSetDrmInitDataCb plusplayer_capi_set_drm_init_data_cb =
      CallPlusplayerFunction<FunPlusplayerCapiSetDrmInitDataCb>(
          plusplayer_capi_handle_, "plusplayer_set_drm_init_data_cb");
  if (plusplayer_capi_set_drm_init_data_cb) {
    return plusplayer_capi_set_drm_init_data_cb(handle, drm_init_data_callback,
                                                userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::
    plusplayer_capi_set_adaptive_streaming_control_event_cb(
        plusplayer_h handle,
        plusplayer_adaptive_streaming_control_event_cb
            adaptive_streaming_control_event_cb,
        void* userdata) {
  FunPlusplayerCapiSetAdaptiveStreamingControlEventCb
      plusplayer_capi_set_adaptive_streaming_control_event_cb =
          CallPlusplayerFunction<
              FunPlusplayerCapiSetAdaptiveStreamingControlEventCb>(
              plusplayer_capi_handle_,
              "plusplayer_set_adaptive_streaming_control_event_cb");
  if (plusplayer_capi_set_adaptive_streaming_control_event_cb) {
    return plusplayer_capi_set_adaptive_streaming_control_event_cb(
        handle, adaptive_streaming_control_event_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_drm_license_acquired_done(
    plusplayer_h handle, plusplayer_track_type_e track_type) {
  FunPlusplayerCapiDrmLicenseAcquiredDone
      plusplayer_capi_drm_license_acquired_done =
          CallPlusplayerFunction<FunPlusplayerCapiDrmLicenseAcquiredDone>(
              plusplayer_capi_handle_, "plusplayer_drm_license_acquired_done");
  if (plusplayer_capi_drm_license_acquired_done) {
    return plusplayer_capi_drm_license_acquired_done(handle, track_type);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_subtitle_path(plusplayer_h handle,
                                                           const char* uri) {
  FunPlusplayerCapiSetSubtitlePath plusplayer_capi_set_subtitle_path =
      CallPlusplayerFunction<FunPlusplayerCapiSetSubtitlePath>(
          plusplayer_capi_handle_, "plusplayer_set_subtitle_path");
  if (plusplayer_capi_set_subtitle_path) {
    return plusplayer_capi_set_subtitle_path(handle, uri);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_video_stillmode(
    plusplayer_h handle, plusplayer_still_mode_e stillmode) {
  FunPlusplayerCapiSetVideoStillmode plusplayer_capi_set_video_stillmode =
      CallPlusplayerFunction<FunPlusplayerCapiSetVideoStillmode>(
          plusplayer_capi_handle_, "plusplayer_set_video_stillmode");
  if (plusplayer_capi_set_video_stillmode) {
    return plusplayer_capi_set_video_stillmode(handle, stillmode);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_alternative_video_resource(
    plusplayer_h handle, unsigned int rsc_type) {
  FunPlusplayerCapiSetAlternativeVideoResource
      plusplayer_capi_set_alternative_video_resource =
          CallPlusplayerFunction<FunPlusplayerCapiSetAlternativeVideoResource>(
              plusplayer_capi_handle_,
              "plusplayer_set_alternative_video_resource");
  if (plusplayer_capi_set_alternative_video_resource) {
    return plusplayer_capi_set_alternative_video_resource(handle, rsc_type);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_foreach_track(
    plusplayer_h handle, plusplayer_track_cb track_cb, void* userdata) {
  FunPlusplayerCapiGetForeachTrack plusplayer_capi_get_foreach_track =
      CallPlusplayerFunction<FunPlusplayerCapiGetForeachTrack>(
          plusplayer_capi_handle_, "plusplayer_get_foreach_track");
  if (plusplayer_capi_get_foreach_track) {
    return plusplayer_capi_get_foreach_track(handle, track_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_foreach_active_track(
    plusplayer_h handle, plusplayer_track_cb track_cb, void* userdata) {
  FunPlusplayerCapiGetForeachActiveTrack
      plusplayer_capi_get_foreach_active_track =
          CallPlusplayerFunction<FunPlusplayerCapiGetForeachActiveTrack>(
              plusplayer_capi_handle_, "plusplayer_get_foreach_active_track");
  if (plusplayer_capi_get_foreach_active_track) {
    return plusplayer_capi_get_foreach_active_track(handle, track_cb, userdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_cookie(plusplayer_h handle,
                                                    const char* cookie) {
  FunPlusplayerCapiSetCookie plusplayer_capi_set_cookie =
      CallPlusplayerFunction<FunPlusplayerCapiSetCookie>(
          plusplayer_capi_handle_, "plusplayer_set_cookie");
  if (plusplayer_capi_set_cookie) {
    return plusplayer_capi_set_cookie(handle, cookie);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_user_agent(
    plusplayer_h handle, const char* user_agent) {
  FunPlusplayerCapiSetUserAgent plusplayer_capi_set_user_agent =
      CallPlusplayerFunction<FunPlusplayerCapiSetUserAgent>(
          plusplayer_capi_handle_, "plusplayer_set_user_agent");
  if (plusplayer_capi_set_user_agent) {
    return plusplayer_capi_set_user_agent(handle, user_agent);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_set_resume_time(
    plusplayer_h handle, uint64_t resume_time_ms) {
  FunPlusplayerCapiSetResumeTime plusplayer_capi_set_resume_time =
      CallPlusplayerFunction<FunPlusplayerCapiSetResumeTime>(
          plusplayer_capi_handle_, "plusplayer_set_resume_time");
  if (plusplayer_capi_set_resume_time) {
    return plusplayer_capi_set_resume_time(handle, resume_time_ms);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_index(
    plusplayer_track_h track, int* track_index) {
  FunPlusplayerCapiGetTrackIndex plusplayer_capi_get_track_index =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackIndex>(
          plusplayer_capi_handle_, "plusplayer_get_track_index");
  if (plusplayer_capi_get_track_index) {
    return plusplayer_capi_get_track_index(track, track_index);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_id(plusplayer_track_h track,
                                                      int* track_id) {
  FunPlusplayerCapiGetTrackId plusplayer_capi_get_track_id =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackId>(
          plusplayer_capi_handle_, "plusplayer_get_track_id");
  if (plusplayer_capi_get_track_id) {
    return plusplayer_capi_get_track_id(track, track_id);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_mimetype(
    plusplayer_track_h track, const char** track_mimetype) {
  FunPlusplayerCapiGetTrackMimetype plusplayer_capi_get_track_mimetype =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackMimetype>(
          plusplayer_capi_handle_, "plusplayer_get_track_mimetype");
  if (plusplayer_capi_get_track_mimetype) {
    return plusplayer_capi_get_track_mimetype(track, track_mimetype);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_streamtype(
    plusplayer_track_h track, const char** track_streamtype) {
  FunPlusplayerCapiGetTrackStreamtype plusplayer_capi_get_track_streamtype =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackStreamtype>(
          plusplayer_capi_handle_, "plusplayer_get_track_streamtype");
  if (plusplayer_capi_get_track_streamtype) {
    return plusplayer_capi_get_track_streamtype(track, track_streamtype);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_container_type(
    plusplayer_track_h track, const char** track_containertype) {
  FunPlusplayerCapiGetTrackContainerType
      plusplayer_capi_get_track_container_type =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackContainerType>(
              plusplayer_capi_handle_, "plusplayer_get_track_container_type");
  if (plusplayer_capi_get_track_container_type) {
    return plusplayer_capi_get_track_container_type(track, track_containertype);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_type(
    plusplayer_track_h track, plusplayer_track_type_e* track_type) {
  FunPlusplayerCapiGetTrackType plusplayer_capi_get_track_type =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackType>(
          plusplayer_capi_handle_, "plusplayer_get_track_type");
  if (plusplayer_capi_get_track_type) {
    return plusplayer_capi_get_track_type(track, track_type);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_codec_data(
    plusplayer_track_h track, const char** track_codecdata) {
  FunPlusplayerCapiGetTrackCodecData plusplayer_capi_get_track_codec_data =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackCodecData>(
          plusplayer_capi_handle_, "plusplayer_get_track_codec_data");
  if (plusplayer_capi_get_track_codec_data) {
    return plusplayer_capi_get_track_codec_data(track, track_codecdata);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_codec_tag(
    plusplayer_track_h track, unsigned int* track_codectag) {
  FunPlusplayerCapiGetTrackCodecTag plusplayer_capi_get_track_codec_tag =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackCodecTag>(
          plusplayer_capi_handle_, "plusplayer_get_track_codec_tag");
  if (plusplayer_capi_get_track_codec_tag) {
    return plusplayer_capi_get_track_codec_tag(track, track_codectag);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_codec_data_len(
    plusplayer_track_h track, int* track_codecdatalen) {
  FunPlusplayerCapiGetTrackCodecDataLen
      plusplayer_capi_get_track_codec_data_len =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackCodecDataLen>(
              plusplayer_capi_handle_, "plusplayer_get_track_codec_data_len");
  if (plusplayer_capi_get_track_codec_data_len) {
    return plusplayer_capi_get_track_codec_data_len(track, track_codecdatalen);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_width(
    plusplayer_track_h track, int* track_width) {
  FunPlusplayerCapiGetTrackWidth plusplayer_capi_get_track_width =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackWidth>(
          plusplayer_capi_handle_, "plusplayer_get_track_width");
  if (plusplayer_capi_get_track_width) {
    return plusplayer_capi_get_track_width(track, track_width);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_height(
    plusplayer_track_h track, int* track_height) {
  FunPlusplayerCapiGetTrackHeight plusplayer_capi_get_track_height =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackHeight>(
          plusplayer_capi_handle_, "plusplayer_get_track_height");
  if (plusplayer_capi_get_track_height) {
    return plusplayer_capi_get_track_height(track, track_height);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_maxwidth(
    plusplayer_track_h track, int* track_maxwidth) {
  FunPlusplayerCapiGetTrackMaxwidth plusplayer_capi_get_track_maxwidth =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackMaxwidth>(
          plusplayer_capi_handle_, "plusplayer_get_track_maxwidth");
  if (plusplayer_capi_get_track_maxwidth) {
    return plusplayer_capi_get_track_maxwidth(track, track_maxwidth);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_maxheight(
    plusplayer_track_h track, int* track_maxheight) {
  FunPlusplayerCapiGetTrackMaxheight plusplayer_capi_get_track_maxheight =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackMaxheight>(
          plusplayer_capi_handle_, "plusplayer_get_track_maxheight");
  if (plusplayer_capi_get_track_maxheight) {
    return plusplayer_capi_get_track_maxheight(track, track_maxheight);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_framerate_num(
    plusplayer_track_h track, int* track_framerate_num) {
  FunPlusplayerCapiGetTrackFramerateNum
      plusplayer_capi_get_track_framerate_num =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackFramerateNum>(
              plusplayer_capi_handle_, "plusplayer_get_track_framerate_num");
  if (plusplayer_capi_get_track_framerate_num) {
    return plusplayer_capi_get_track_framerate_num(track, track_framerate_num);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_framerate_den(
    plusplayer_track_h track, int* track_framerate_den) {
  FunPlusplayerCapiGetTrackFramerateDen
      plusplayer_capi_get_track_framerate_den =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackFramerateDen>(
              plusplayer_capi_handle_, "plusplayer_get_track_framerate_den");
  if (plusplayer_capi_get_track_framerate_den) {
    return plusplayer_capi_get_track_framerate_den(track, track_framerate_den);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_sample_rate(
    plusplayer_track_h track, int* track_sample_rate) {
  FunPlusplayerCapiGetTrackSampleRate plusplayer_capi_get_track_sample_rate =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackSampleRate>(
          plusplayer_capi_handle_, "plusplayer_get_track_sample_rate");
  if (plusplayer_capi_get_track_sample_rate) {
    return plusplayer_capi_get_track_sample_rate(track, track_sample_rate);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_sample_format(
    plusplayer_track_h track, int* track_sample_format) {
  FunPlusplayerCapiGetTrackSampleFormat
      plusplayer_capi_get_track_sample_format =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackSampleFormat>(
              plusplayer_capi_handle_, "plusplayer_get_track_sample_format");
  if (plusplayer_capi_get_track_sample_format) {
    return plusplayer_capi_get_track_sample_format(track, track_sample_format);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_channels(
    plusplayer_track_h track, int* track_channels) {
  FunPlusplayerCapiGetTrackChannels plusplayer_capi_get_track_channels =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackChannels>(
          plusplayer_capi_handle_, "plusplayer_get_track_channels");
  if (plusplayer_capi_get_track_channels) {
    return plusplayer_capi_get_track_channels(track, track_channels);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_version(
    plusplayer_track_h track, int* track_version) {
  FunPlusplayerCapiGetTrackVersion plusplayer_capi_get_track_version =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackVersion>(
          plusplayer_capi_handle_, "plusplayer_get_track_version");
  if (plusplayer_capi_get_track_version) {
    return plusplayer_capi_get_track_version(track, track_version);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_layer(
    plusplayer_track_h track, int* track_layer) {
  FunPlusplayerCapiGetTrackLayer plusplayer_capi_get_track_layer =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackLayer>(
          plusplayer_capi_handle_, "plusplayer_get_track_layer");
  if (plusplayer_capi_get_track_layer) {
    return plusplayer_capi_get_track_layer(track, track_layer);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_bits_per_sample(
    plusplayer_track_h track, int* track_bits_per_sample) {
  FunPlusplayerCapiGetTrackBitsPerSample
      plusplayer_capi_get_track_bits_per_sample =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackBitsPerSample>(
              plusplayer_capi_handle_, "plusplayer_get_track_bits_per_sample");
  if (plusplayer_capi_get_track_bits_per_sample) {
    return plusplayer_capi_get_track_bits_per_sample(track,
                                                     track_bits_per_sample);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_block_align(
    plusplayer_track_h track, int* track_block_align) {
  FunPlusplayerCapiGetTrackBlockAlign plusplayer_capi_get_track_block_align =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackBlockAlign>(
          plusplayer_capi_handle_, "plusplayer_get_track_block_align");
  if (plusplayer_capi_get_track_block_align) {
    return plusplayer_capi_get_track_block_align(track, track_block_align);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_bitrate(
    plusplayer_track_h track, int* track_bitrate) {
  FunPlusplayerCapiGetTrackBitrate plusplayer_capi_get_track_bitrate =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackBitrate>(
          plusplayer_capi_handle_, "plusplayer_get_track_bitrate");
  if (plusplayer_capi_get_track_bitrate) {
    return plusplayer_capi_get_track_bitrate(track, track_bitrate);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_endianness(
    plusplayer_track_h track, int* track_endianness) {
  FunPlusplayerCapiGetTrackEndianness plusplayer_capi_get_track_endianness =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackEndianness>(
          plusplayer_capi_handle_, "plusplayer_get_track_endianness");
  if (plusplayer_capi_get_track_endianness) {
    return plusplayer_capi_get_track_endianness(track, track_endianness);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_is_signed(
    plusplayer_track_h track, bool* track_is_signed) {
  FunPlusplayerCapiGetTrackIsSigned plusplayer_capi_get_track_is_signed =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackIsSigned>(
          plusplayer_capi_handle_, "plusplayer_get_track_is_signed");
  if (plusplayer_capi_get_track_is_signed) {
    return plusplayer_capi_get_track_is_signed(track, track_is_signed);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_active(
    plusplayer_track_h track, bool* track_active) {
  FunPlusplayerCapiGetTrackActive plusplayer_capi_get_track_active =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackActive>(
          plusplayer_capi_handle_, "plusplayer_get_track_active");
  if (plusplayer_capi_get_track_active) {
    return plusplayer_capi_get_track_active(track, track_active);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_lang_code(
    plusplayer_track_h track, const char** track_lang_code) {
  FunPlusplayerCapiGetTrackLangCode plusplayer_capi_get_track_lang_code =
      CallPlusplayerFunction<FunPlusplayerCapiGetTrackLangCode>(
          plusplayer_capi_handle_, "plusplayer_get_track_lang_code");
  if (plusplayer_capi_get_track_lang_code) {
    return plusplayer_capi_get_track_lang_code(track, track_lang_code);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}

int PlusPlayerCapiProxy::plusplayer_capi_get_track_subtitle_format(
    plusplayer_track_h track, const char** track_subtitle_format) {
  FunPlusplayerCapiGetTrackSubtitleFormat
      plusplayer_capi_get_track_subtitle_format =
          CallPlusplayerFunction<FunPlusplayerCapiGetTrackSubtitleFormat>(
              plusplayer_capi_handle_, "plusplayer_get_track_subtitle_format");
  if (plusplayer_capi_get_track_subtitle_format) {
    return plusplayer_capi_get_track_subtitle_format(track,
                                                     track_subtitle_format);
  }
  return PLUSPLAYER_ERROR_TYPE_UNKNOWN;
}
