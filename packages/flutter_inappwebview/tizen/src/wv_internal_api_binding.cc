// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wv_internal_api_binding.h"

#include <dlfcn.h>

WvInternalApiBinding::WvInternalApiBinding() {
  // NOTE: WV symbols are exported by the Chromium EWK library.
  handle_ = dlopen("libchromium-ewk.so", RTLD_LAZY);
}

WvInternalApiBinding::~WvInternalApiBinding() {
  if (handle_) {
    dlclose(handle_);
  }
}

bool WvInternalApiBinding::Initialize() {
  if (initialize_result_.has_value()) {
    return *initialize_result_;
  }

  if (!handle_) {
    initialize_result_ = false;
    return false;
  }

  main.Init = reinterpret_cast<WvInitFnPtr>(dlsym(handle_, "wv_init"));
  main.Shutdown =
      reinterpret_cast<WvShutdownFnPtr>(dlsym(handle_, "wv_shutdown"));
  main.SetArguments =
      reinterpret_cast<WvSetArgumentsFnPtr>(dlsym(handle_, "wv_set_arguments"));

  view.Create =
      reinterpret_cast<WvViewCreateFnPtr>(dlsym(handle_, "wv_view_create"));
  view.Destroy =
      reinterpret_cast<WvViewDestroyFnPtr>(dlsym(handle_, "wv_view_destroy"));
  view.Resize =
      reinterpret_cast<WvViewResizeFnPtr>(dlsym(handle_, "wv_view_resize"));
  view.FocusSet = reinterpret_cast<WvViewFocusSetFnPtr>(
      dlsym(handle_, "wv_view_focus_set"));
  view.UrlSet =
      reinterpret_cast<WvViewUrlSetFnPtr>(dlsym(handle_, "wv_view_url_set"));
  view.UrlGet =
      reinterpret_cast<WvViewUrlGetFnPtr>(dlsym(handle_, "wv_view_url_get"));
  view.UrlRequestSet = reinterpret_cast<WvViewUrlRequestSetFnPtr>(
      dlsym(handle_, "wv_view_url_request_set"));
  view.HtmlStringLoad = reinterpret_cast<WvViewHtmlStringLoadFnPtr>(
      dlsym(handle_, "wv_view_html_string_load"));
  view.BackPossible = reinterpret_cast<WvViewBackPossibleFnPtr>(
      dlsym(handle_, "wv_view_back_possible"));
  view.ForwardPossible = reinterpret_cast<WvViewForwardPossibleFnPtr>(
      dlsym(handle_, "wv_view_forward_possible"));
  view.Back = reinterpret_cast<WvViewBackFnPtr>(dlsym(handle_, "wv_view_back"));
  view.Forward =
      reinterpret_cast<WvViewForwardFnPtr>(dlsym(handle_, "wv_view_forward"));
  view.Reload =
      reinterpret_cast<WvViewReloadFnPtr>(dlsym(handle_, "wv_view_reload"));
  view.Stop = reinterpret_cast<WvViewStopFnPtr>(dlsym(handle_, "wv_view_stop"));
  view.Suspend =
      reinterpret_cast<WvViewSuspendFnPtr>(dlsym(handle_, "wv_view_suspend"));
  view.Resume =
      reinterpret_cast<WvViewResumeFnPtr>(dlsym(handle_, "wv_view_resume"));
  view.TitleGet = reinterpret_cast<WvViewTitleGetFnPtr>(
      dlsym(handle_, "wv_view_title_get"));
  view.LoadProgressGet = reinterpret_cast<WvViewLoadProgressGetFnPtr>(
      dlsym(handle_, "wv_view_load_progress_get"));
  view.ScaleSet = reinterpret_cast<WvViewScaleSetFnPtr>(
      dlsym(handle_, "wv_view_scale_set"));
  view.ScaleGet = reinterpret_cast<WvViewScaleGetFnPtr>(
      dlsym(handle_, "wv_view_scale_get"));
  view.UserAgentSet = reinterpret_cast<WvViewUserAgentSetFnPtr>(
      dlsym(handle_, "wv_view_user_agent_set"));
  view.UserAgentGet = reinterpret_cast<WvViewUserAgentGetFnPtr>(
      dlsym(handle_, "wv_view_user_agent_get"));
  view.ScrollSet = reinterpret_cast<WvViewScrollSetFnPtr>(
      dlsym(handle_, "wv_view_scroll_set"));
  view.ScrollPosGet = reinterpret_cast<WvViewScrollPosGetFnPtr>(
      dlsym(handle_, "wv_view_scroll_pos_get"));
  view.ScriptExecute = reinterpret_cast<WvViewScriptExecuteFnPtr>(
      dlsym(handle_, "wv_view_script_execute"));
  view.BgColorSet = reinterpret_cast<WvViewBgColorSetFnPtr>(
      dlsym(handle_, "wv_view_bg_color_set"));
  view.ContextGet = reinterpret_cast<WvViewContextGetFnPtr>(
      dlsym(handle_, "wv_view_context_get"));
  view.SettingsGet = reinterpret_cast<WvViewSettingsGetFnPtr>(
      dlsym(handle_, "wv_view_settings_get"));
  view.FeedTouchEvent = reinterpret_cast<WvViewFeedTouchEventFnPtr>(
      dlsym(handle_, "wv_view_feed_touch_event"));
  view.FeedMouseDown = reinterpret_cast<WvViewFeedMouseDownFnPtr>(
      dlsym(handle_, "wv_view_feed_mouse_down"));
  view.FeedMouseUp = reinterpret_cast<WvViewFeedMouseUpFnPtr>(
      dlsym(handle_, "wv_view_feed_mouse_up"));
  view.FeedMouseMove = reinterpret_cast<WvViewFeedMouseMoveFnPtr>(
      dlsym(handle_, "wv_view_feed_mouse_move"));
  view.FeedMouseWheel = reinterpret_cast<WvViewFeedMouseWheelFnPtr>(
      dlsym(handle_, "wv_view_feed_mouse_wheel"));
  view.SendKeyEvent = reinterpret_cast<WvViewSendKeyEventFnPtr>(
      dlsym(handle_, "wv_view_send_key_event"));
  view.TouchEventsEnabledSet =
      reinterpret_cast<WvViewTouchEventsEnabledSetFnPtr>(
          dlsym(handle_, "wv_view_touch_events_enabled_set"));
  view.MouseEventsEnabledSet =
      reinterpret_cast<WvViewMouseEventsEnabledSetFnPtr>(
          dlsym(handle_, "wv_view_mouse_events_enabled_set"));
  view.KeyEventsEnabledSet = reinterpret_cast<WvViewKeyEventsEnabledSetFnPtr>(
      dlsym(handle_, "wv_view_key_events_enabled_set"));
  view.ImeWindowSet = reinterpret_cast<WvViewImeWindowSetFnPtr>(
      dlsym(handle_, "wv_view_ime_window_set"));
  view.AddCallback = reinterpret_cast<WvViewAddCallbackFnPtr>(
      dlsym(handle_, "wv_view_add_cb"));
  view.RemoveFullCallback = reinterpret_cast<WvViewRemoveFullCallbackFnPtr>(
      dlsym(handle_, "wv_view_remove_full_cb"));
  view.OnJavaScriptAlert =
      reinterpret_cast<WvViewJavaScriptAlertCallbackSetFnPtr>(
          dlsym(handle_, "wv_view_javascript_alert_callback_set"));
  view.OnJavaScriptConfirm =
      reinterpret_cast<WvViewJavaScriptConfirmCallbackSetFnPtr>(
          dlsym(handle_, "wv_view_javascript_confirm_callback_set"));
  view.OnJavaScriptPrompt =
      reinterpret_cast<WvViewJavaScriptPromptCallbackSetFnPtr>(
          dlsym(handle_, "wv_view_javascript_prompt_callback_set"));
  view.JavaScriptAlertReply = reinterpret_cast<WvViewJavaScriptAlertReplyFnPtr>(
      dlsym(handle_, "wv_view_javascript_alert_reply"));
  view.JavaScriptConfirmReply =
      reinterpret_cast<WvViewJavaScriptConfirmReplyFnPtr>(
          dlsym(handle_, "wv_view_javascript_confirm_reply"));
  view.JavaScriptPromptReply =
      reinterpret_cast<WvViewJavaScriptPromptReplyFnPtr>(
          dlsym(handle_, "wv_view_javascript_prompt_reply"));
  view.SetSupportVideoHole = reinterpret_cast<WvViewSetSupportVideoHoleFnPtr>(
      dlsym(handle_, "wv_view_set_support_video_hole"));
  view.MainFrameScrollbarVisibleSet =
      reinterpret_cast<WvViewMainFrameScrollbarVisibleSetFnPtr>(
          dlsym(handle_, "wv_view_main_frame_scrollbar_visible_set"));

  context.CookieManagerGet = reinterpret_cast<WvContextCookieManagerGetFnPtr>(
      dlsym(handle_, "wv_context_cookie_manager_get"));
  context.CacheModelSet = reinterpret_cast<WvContextCacheModelSetFnPtr>(
      dlsym(handle_, "wv_context_cache_model_set"));
  context.CacheClear = reinterpret_cast<WvContextCacheClearFnPtr>(
      dlsym(handle_, "wv_context_cache_clear"));

  cookie_manager.AcceptPolicySet =
      reinterpret_cast<WvCookieManagerAcceptPolicySetFnPtr>(
          dlsym(handle_, "wv_cookie_manager_accept_policy_set"));
  cookie_manager.CookiesClear =
      reinterpret_cast<WvCookieManagerCookiesClearFnPtr>(
          dlsym(handle_, "wv_cookie_manager_cookies_clear"));

  settings.JavaScriptEnabledSet =
      reinterpret_cast<WvSettingsJavaScriptEnabledSetFnPtr>(
          dlsym(handle_, "wv_settings_javascript_enabled_set"));
  settings.ImePanelEnabledSet =
      reinterpret_cast<WvSettingsImePanelEnabledSetFnPtr>(
          dlsym(handle_, "wv_settings_ime_panel_enabled_set"));
  settings.ForceZoomSet = reinterpret_cast<WvSettingsForceZoomSetFnPtr>(
      dlsym(handle_, "wv_settings_force_zoom_set"));

  error.CodeGet = reinterpret_cast<WvErrorCodeGetFnPtr>(
      dlsym(handle_, "wv_error_code_get"));
  error.DescriptionGet = reinterpret_cast<WvErrorDescriptionGetFnPtr>(
      dlsym(handle_, "wv_error_description_get"));
  error.UrlGet =
      reinterpret_cast<WvErrorUrlGetFnPtr>(dlsym(handle_, "wv_error_url_get"));

  policy_decision.Use = reinterpret_cast<WvPolicyDecisionUseFnPtr>(
      dlsym(handle_, "wv_policy_decision_use"));
  policy_decision.UrlGet = reinterpret_cast<WvPolicyDecisionUrlGetFnPtr>(
      dlsym(handle_, "wv_policy_decision_url_get"));

  console_message.LevelGet = reinterpret_cast<WvConsoleMessageLevelGetFnPtr>(
      dlsym(handle_, "wv_console_message_level_get"));
  console_message.TextGet = reinterpret_cast<WvConsoleMessageTextGetFnPtr>(
      dlsym(handle_, "wv_console_message_text_get"));

  initialize_result_ =
      main.Init && main.Shutdown && main.SetArguments && view.Create &&
      view.Destroy && view.Resize && view.FocusSet && view.UrlSet &&
      view.UrlGet && view.UrlRequestSet && view.HtmlStringLoad &&
      view.BackPossible && view.ForwardPossible && view.Back && view.Forward &&
      view.Reload && view.Stop && view.Suspend && view.Resume &&
      view.TitleGet && view.LoadProgressGet && view.ScaleSet && view.ScaleGet &&
      view.UserAgentSet && view.UserAgentGet && view.ScrollSet &&
      view.ScrollPosGet && view.ScriptExecute && view.BgColorSet &&
      view.ContextGet && view.SettingsGet && view.FeedTouchEvent &&
      view.FeedMouseDown && view.FeedMouseUp && view.FeedMouseMove &&
      view.FeedMouseWheel && view.SendKeyEvent && view.TouchEventsEnabledSet &&
      view.MouseEventsEnabledSet && view.KeyEventsEnabledSet &&
      view.ImeWindowSet && view.AddCallback && view.RemoveFullCallback &&
      view.OnJavaScriptAlert && view.OnJavaScriptConfirm &&
      view.OnJavaScriptPrompt && view.JavaScriptAlertReply &&
      view.JavaScriptConfirmReply && view.JavaScriptPromptReply &&
      view.SetSupportVideoHole && view.MainFrameScrollbarVisibleSet &&
      context.CookieManagerGet && context.CacheModelSet && context.CacheClear &&
      cookie_manager.AcceptPolicySet && cookie_manager.CookiesClear &&
      settings.JavaScriptEnabledSet && settings.ImePanelEnabledSet &&
      settings.ForceZoomSet && error.CodeGet && error.DescriptionGet &&
      error.UrlGet && policy_decision.Use && policy_decision.UrlGet &&
      console_message.LevelGet && console_message.TextGet;
  return *initialize_result_;
}
