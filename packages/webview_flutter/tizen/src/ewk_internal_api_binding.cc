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
  view.FeedTouchEvent = reinterpret_cast<EwkViewFeedTouchEventFnPtr>(
      dlsym(handle_, "ewk_view_feed_touch_event"));
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

  view.OnJavascriptAlert =
      reinterpret_cast<EwkViewJavascriptAlertCallbackSetFnPtr>(
          dlsym(handle_, "ewk_view_javascript_alert_callback_set"));
  view.OnJavascriptConfirm =
      reinterpret_cast<EwkViewJavascriptConfirmCallbackSetFnPtr>(
          dlsym(handle_, "ewk_view_javascript_confirm_callback_set"));
  view.OnJavascriptPrompt =
      reinterpret_cast<EwkViewJavascriptPromptCallbackSetFnPtr>(
          dlsym(handle_, "ewk_view_javascript_prompt_callback_set"));

  view.JavascriptAlertReply =
      reinterpret_cast<EwkViewJavascriptAlertReplyFnPtr>(
          dlsym(handle_, "ewk_view_javascript_alert_reply"));
  view.JavascriptConfirmReply =
      reinterpret_cast<EwkViewJavascriptConfirmReplyFnPtr>(
          dlsym(handle_, "ewk_view_javascript_confirm_reply"));
  view.JavascriptPromptReply =
      reinterpret_cast<EwkViewJavascriptPromptReplyFnPtr>(
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

  return view.SetBackgroundColor && view.FeedTouchEvent && view.SendKeyEvent &&
         view.OffscreenRenderingEnabledSet && view.ImeWindowSet &&
         view.KeyEventsEnabledSet && view.SupportVideoHoleSet &&
         view.OnJavascriptAlert && view.OnJavascriptConfirm &&
         view.OnJavascriptPrompt && view.JavascriptAlertReply &&
         view.JavascriptConfirmReply && view.JavascriptPromptReply &&
         main.SetArguments && main.SetVersionPolicy &&
         settings.ImePanelEnabledSet && settings.ForceZoomSet &&
         console_message.LevelGet && console_message.TextGet &&
         console_message.LineGet && console_message.SourceGet;
}
