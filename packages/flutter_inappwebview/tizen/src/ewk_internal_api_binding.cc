// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ewk_internal_api_binding.h"

#include <dlfcn.h>

#include "log.h"

namespace {

template <typename T>
T ResolveEwkSymbol(void* handle, const char* symbol) {
  dlerror();
  void* value = dlsym(handle, symbol);
  const char* error = dlerror();
  if (error || !value) {
    LOG_ERROR("Failed to resolve EWK symbol %s: %s", symbol,
              error ? error : "not found");
    return nullptr;
  }
  return reinterpret_cast<T>(value);
}

}  // namespace

EwkInternalApiBinding::EwkInternalApiBinding() {
  handle_ = dlopen("libchromium-ewk.so", RTLD_LAZY);
  if (!handle_) {
    const char* error = dlerror();
    LOG_ERROR("Failed to open libchromium-ewk.so: %s",
              error ? error : "unknown error");
  }
}

EwkInternalApiBinding::~EwkInternalApiBinding() {
  if (handle_) {
    dlclose(handle_);
  }
}

bool EwkInternalApiBinding::Initialize() {
  if (!handle_) {
    LOG_ERROR(
        "EWK internal API binding was initialized without a library handle.");
    return false;
  }

  // ewk_view
  view.SetBackgroundColor = ResolveEwkSymbol<EwkViewBgColorSetFnPtr>(
      handle_, "ewk_view_bg_color_set");
  view.TouchEventsEnabledSet =
      ResolveEwkSymbol<EwkViewTouchEventsEnabledSetFnPtr>(
          handle_, "ewk_view_touch_events_enabled_set");
  view.FeedTouchEvent = ResolveEwkSymbol<EwkViewFeedTouchEventFnPtr>(
      handle_, "ewk_view_feed_touch_event");
  view.MouseEventsEnabledSet =
      ResolveEwkSymbol<EwkViewMouseEventsEnabledSetFnPtr>(
          handle_, "ewk_view_mouse_events_enabled_set");
  view.FeedMouseDown = ResolveEwkSymbol<EwkViewFeedMouseDownFnPtr>(
      handle_, "ewk_view_feed_mouse_down");
  view.FeedMouseUp = ResolveEwkSymbol<EwkViewFeedMouseUpFnPtr>(
      handle_, "ewk_view_feed_mouse_up");
  view.FeedMouseWheel = ResolveEwkSymbol<EwkViewFeedMouseWheelFnPtr>(
      handle_, "ewk_view_feed_mouse_wheel");
  view.SendKeyEvent = ResolveEwkSymbol<EwkViewSendKeyEventFnPtr>(
      handle_, "ewk_view_send_key_event");
  view.OffscreenRenderingEnabledSet =
      ResolveEwkSymbol<EwkViewOffscreenRenderingEnabledSetFnPtr>(
          handle_, "ewk_view_offscreen_rendering_enabled_set");
  view.ImeWindowSet = ResolveEwkSymbol<EwkViewImeWindowSetFnPtr>(
      handle_, "ewk_view_ime_window_set");
  view.KeyEventsEnabledSet = ResolveEwkSymbol<EwkViewKeyEventsEnabledSetFnPtr>(
      handle_, "ewk_view_key_events_enabled_set");
  view.SupportVideoHoleSet = ResolveEwkSymbol<EwkViewSupportVideoHoleSetFnPtr>(
      handle_, "ewk_view_set_support_video_hole");

  view.OnJavaScriptAlert =
      ResolveEwkSymbol<EwkViewJavaScriptAlertCallbackSetFnPtr>(
          handle_, "ewk_view_javascript_alert_callback_set");
  view.OnJavaScriptConfirm =
      ResolveEwkSymbol<EwkViewJavaScriptConfirmCallbackSetFnPtr>(
          handle_, "ewk_view_javascript_confirm_callback_set");
  view.OnJavaScriptPrompt =
      ResolveEwkSymbol<EwkViewJavaScriptPromptCallbackSetFnPtr>(
          handle_, "ewk_view_javascript_prompt_callback_set");

  view.JavaScriptAlertReply =
      ResolveEwkSymbol<EwkViewJavaScriptAlertReplyFnPtr>(
          handle_, "ewk_view_javascript_alert_reply");
  view.JavaScriptConfirmReply =
      ResolveEwkSymbol<EwkViewJavaScriptConfirmReplyFnPtr>(
          handle_, "ewk_view_javascript_confirm_reply");
  view.JavaScriptPromptReply =
      ResolveEwkSymbol<EwkViewJavaScriptPromptReplyFnPtr>(
          handle_, "ewk_view_javascript_prompt_reply");

  // ewk_main
  main.SetArguments =
      ResolveEwkSymbol<EwkSetArgumentsFnPtr>(handle_, "ewk_set_arguments");

  // ewk_settings
  settings.ImePanelEnabledSet =
      ResolveEwkSymbol<EwkSettingsImePanelEnabledSetFnPtr>(
          handle_, "ewk_settings_ime_panel_enabled_set");
  settings.ForceZoomSet = ResolveEwkSymbol<EwkSettingsForceZoomSetFnPtr>(
      handle_, "ewk_settings_force_zoom_set");

  // ewk_console_message
  console_message.LevelGet = ResolveEwkSymbol<EwkConsoleMessageLevelGetFnPtr>(
      handle_, "ewk_console_message_level_get");
  console_message.TextGet = ResolveEwkSymbol<EwkConsoleMessageTextGetFnPtr>(
      handle_, "ewk_console_message_text_get");

  return view.SetBackgroundColor && view.TouchEventsEnabledSet &&
         view.FeedTouchEvent && view.MouseEventsEnabledSet &&
         view.FeedMouseDown && view.FeedMouseUp && view.FeedMouseWheel &&
         view.SendKeyEvent && view.OffscreenRenderingEnabledSet &&
         view.ImeWindowSet && view.KeyEventsEnabledSet &&
         view.SupportVideoHoleSet && view.OnJavaScriptAlert &&
         view.OnJavaScriptConfirm && view.OnJavaScriptPrompt &&
         view.JavaScriptAlertReply && view.JavaScriptConfirmReply &&
         view.JavaScriptPromptReply && main.SetArguments &&
         settings.ImePanelEnabledSet && settings.ForceZoomSet &&
         console_message.LevelGet && console_message.TextGet;
}
