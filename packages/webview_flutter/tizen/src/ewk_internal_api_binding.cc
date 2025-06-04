// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ewk_internal_api_binding.h"

#include <dlfcn.h>

EwkInternalApiBinding::EwkInternalApiBinding() {
  handle_ = dlopen("libchromium-ewk.so", RTLD_LAZY);
}

EwkInternalApiBinding::~EwkInternalApiBinding() {
  if (handle_) {
    dlclose(handle_);
  }
}

bool EwkInternalApiBinding::Initialize() {
  if (!handle_) {
    return false;
  }

  // ewk_view
  view.SetBackgroundColor = reinterpret_cast<EwkViewBgColorSetFnPtr>(
      dlsym(handle_, "ewk_view_bg_color_set"));
  view.TouchEventsEnabledSet =
      reinterpret_cast<EwkViewTouchEventsEnabledSetFnPtr>(
          dlsym(handle_, "ewk_view_mouse_events_enabled_set"));
  view.FeedTouchEvent = reinterpret_cast<EwkViewFeedTouchEventFnPtr>(
      dlsym(handle_, "ewk_view_feed_touch_event"));
  view.MouseEventsEnabledSet =
      reinterpret_cast<EwkViewMouseEventsEnabledSetFnPtr>(
          dlsym(handle_, "ewk_view_mouse_events_enabled_set"));
  view.FeedMouseDown = reinterpret_cast<EwkViewFeedMouseDownFnPtr>(
      dlsym(handle_, "ewk_view_feed_mouse_down"));
  view.FeedMouseUp = reinterpret_cast<EwkViewFeedMouseUpFnPtr>(
      dlsym(handle_, "ewk_view_feed_mouse_up"));
  view.FeedMouseMove = reinterpret_cast<EwkViewFeedMouseMoveFnPtr>(
      dlsym(handle_, "ewk_view_feed_mouse_move"));
  view.FeedMouseWheel = reinterpret_cast<EwkViewFeedMouseWheelFnPtr>(
      dlsym(handle_, "ewk_view_feed_mouse_wheel"));
  view.SendKeyEvent = reinterpret_cast<EwkViewSendKeyEventFnPtr>(
      dlsym(handle_, "ewk_view_send_key_event"));
  view.OffscreenRenderingEnabledSet =
      reinterpret_cast<EwkViewOffscreenRenderingEnabledSetFnPtr>(
          dlsym(handle_, "ewk_view_offscreen_rendering_enabled_set"));
  view.ImeWindowSet = reinterpret_cast<EwkViewImeWindowSetFnPtr>(
      dlsym(handle_, "ewk_view_ime_window_set"));
  view.KeyEventsEnabledSet = reinterpret_cast<EwkViewKeyEventsEnabledSetFnPtr>(
      dlsym(handle_, "ewk_view_key_events_enabled_set"));
  view.SupportVideoHoleSet = reinterpret_cast<EwkViewSupportVideoHoleSetFnPtr>(
      dlsym(handle_, "ewk_view_set_support_video_hole"));

  view.OnJavaScriptAlert =
      reinterpret_cast<EwkViewJavaScriptAlertCallbackSetFnPtr>(
          dlsym(handle_, "ewk_view_javascript_alert_callback_set"));
  view.OnJavaScriptConfirm =
      reinterpret_cast<EwkViewJavaScriptConfirmCallbackSetFnPtr>(
          dlsym(handle_, "ewk_view_javascript_confirm_callback_set"));
  view.OnJavaScriptPrompt =
      reinterpret_cast<EwkViewJavaScriptPromptCallbackSetFnPtr>(
          dlsym(handle_, "ewk_view_javascript_prompt_callback_set"));

  view.JavaScriptAlertReply =
      reinterpret_cast<EwkViewJavaScriptAlertReplyFnPtr>(
          dlsym(handle_, "ewk_view_javascript_alert_reply"));
  view.JavaScriptConfirmReply =
      reinterpret_cast<EwkViewJavaScriptConfirmReplyFnPtr>(
          dlsym(handle_, "ewk_view_javascript_confirm_reply"));
  view.JavaScriptPromptReply =
      reinterpret_cast<EwkViewJavaScriptPromptReplyFnPtr>(
          dlsym(handle_, "ewk_view_javascript_prompt_reply"));

  // ewk_main
  main.SetArguments = reinterpret_cast<EwkSetArgumentsFnPtr>(
      dlsym(handle_, "ewk_set_arguments"));

  main.SetVersionPolicy = reinterpret_cast<EwkSetVersionPolicyFnPtr>(
      dlsym(handle_, "ewk_set_version_policy"));

  // ewk_settings
  settings.ImePanelEnabledSet =
      reinterpret_cast<EwkSettingsImePanelEnabledSetFnPtr>(
          dlsym(handle_, "ewk_settings_ime_panel_enabled_set"));
  settings.ForceZoomSet = reinterpret_cast<EwkSettingsForceZoomSetFnPtr>(
      dlsym(handle_, "ewk_settings_force_zoom_set"));

  // ewk_console_message
  console_message.LevelGet = reinterpret_cast<EwkConsoleMessageLevelGetFnPtr>(
      dlsym(handle_, "ewk_console_message_level_get"));
  console_message.TextGet = reinterpret_cast<EwkConsoleMessageTextGetFnPtr>(
      dlsym(handle_, "ewk_console_message_text_get"));
  console_message.LineGet = reinterpret_cast<EwkConsoleMessageLineGetFnPtr>(
      dlsym(handle_, "ewk_console_message_line_get"));
  console_message.SourceGet = reinterpret_cast<EwkConsoleMessageSourceGetFnPtr>(
      dlsym(handle_, "ewk_console_message_source_get"));

  return view.SetBackgroundColor && view.TouchEventsEnabledSet &&
         view.FeedTouchEvent && view.MouseEventsEnabledSet &&
         view.FeedMouseDown && view.FeedMouseUp && view.FeedMouseMove &&
         view.FeedMouseWheel && view.SendKeyEvent &&
         view.OffscreenRenderingEnabledSet && view.ImeWindowSet &&
         view.KeyEventsEnabledSet && view.SupportVideoHoleSet &&
         view.OnJavaScriptAlert && view.OnJavaScriptConfirm &&
         view.OnJavaScriptPrompt && view.JavaScriptAlertReply &&
         view.JavaScriptConfirmReply && view.JavaScriptPromptReply &&
         main.SetArguments && main.SetVersionPolicy &&
         settings.ImePanelEnabledSet && settings.ForceZoomSet &&
         console_message.LevelGet && console_message.TextGet &&
         console_message.LineGet && console_message.SourceGet;
}
