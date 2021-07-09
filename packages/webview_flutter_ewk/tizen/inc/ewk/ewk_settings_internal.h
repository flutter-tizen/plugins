/*
 * Copyright (C) 2013-2016 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG ELECTRONICS. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    ewk_settings_internal.h
 * @brief   Describes the settings API.
 *
 * @note The ewk_settings is for setting the preference of specific ewk_view.
 * We can get the ewk_settings from ewk_view using ewk_view_settings_get() API.
 */

#ifndef ewk_settings_internal_h
#define ewk_settings_internal_h

#include "ewk_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a type name for the callback function used to notify the client when
 * the continuous spell checking setting was changed by WebKit.
 *
 * @param enable @c EINA_TRUE if continuous spell checking is enabled or @c
 * EINA_FALSE if it's disabled
 */
typedef void (*Ewk_Settings_Continuous_Spell_Checking_Change_Cb)(
    Eina_Bool enable);

/**
 * \enum    _Ewk_Editable_Link_Behavior
 *
 * @brief   Editable link behavior mode (Must remain in sync with
 * WKEditableLinkBehavior)
 */
enum _Ewk_Editable_Link_Behavior {
  EWK_EDITABLE_LINK_BEHAVIOR_DEFAULT,
  EWK_EDITABLE_LINK_BEHAVIOR_ALWAYS_LIVE,
  EWK_EDITABLE_LINK_BEHAVIOR_ONLY_LIVE_WITH_SHIFTKEY,
  EWK_EDITABLE_LINK_BEHAVIOR_LIVE_WHEN_NOT_FOCUSED,
  EWK_EDITABLE_LINK_BEHAVIOR_NEVER_LIVE
};
typedef enum _Ewk_Editable_Link_Behavior Ewk_Editable_Link_Behavior;

enum _Ewk_Legacy_Font_Size_Mode {
  EWK_LEGACY_FONT_SIZE_MODE_ALWAYS,
  EWK_LEGACY_FONT_SIZE_MODE_ONLY_IF_PIXEL_VALUES_MATCH,
  EWK_LEGACY_FONT_SIZE_MODE_NEVER
};

typedef enum _Ewk_Legacy_Font_Size_Mode Ewk_Legacy_Font_Size_Mode;

enum _Ewk_List_Style_Position {
  EWK_LIST_STYLE_POSITION_OUTSIDE, /**< Default WebKit value. */
  EWK_LIST_STYLE_POSITION_INSIDE
};
typedef enum _Ewk_List_Style_Position Ewk_List_Style_Position;

/*
 * Enables/disables the Javascript Fullscreen API. The Javascript API allows
 * to request full screen mode, for more information see:
 * http://dvcs.w3.org/hg/fullscreen/raw-file/tip/Overview.html
 *
 * Default value for Javascript Fullscreen API setting is @c EINA_TRUE .
 *
 * @param settings settings object to enable Javascript Fullscreen API
 * @param enable @c EINA_TRUE to enable Javascript Fullscreen API or
 *               @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_fullscreen_enabled_set(Ewk_Settings *settings,
                                                         Eina_Bool enable);

/**
 * Returns whether the Javascript Fullscreen API is enabled or not.
 *
 * @param settings settings object to query whether Javascript Fullscreen API is
 * enabled
 *
 * @return @c EINA_TRUE if the Javascript Fullscreen API is enabled
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_fullscreen_enabled_get(const Ewk_Settings *settings);

/*
 * Enables/disables swipe to refresh feature. Swipe to refresh allows
 * us to refresh page using swipe gesture.
 *
 * Default value for swipe to refresh setting is @c EINA_FALSE.
 *
 * @param settings settings object to enable swipe to refresh feature
 * @param enable @c EINA_TRUE to enable swipe to refresh feature or
 *               @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_swipe_to_refresh_enabled_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns whether swipe to refresh feature is enabled or not.
 *
 * @param settings settings object to query whether swipe to refresh feature is
 * enabled
 *
 * @return @c EINA_TRUE if the swipe to refresh feature is enabled
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_swipe_to_refresh_enabled_get(const Ewk_Settings *settings);

/**
 * Requests enables/disables the plug-ins.
 *
 * @param settings settings object to set the plug-ins
 * @param enable @c EINA_TRUE to enable the plug-ins
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_plugins_enabled_set(Ewk_Settings *settings,
                                                      Eina_Bool enable);

/**
 * Returs enables/disables the plug-ins.
 *
 * @param settings settings object to set the plug-ins
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_settings_plugins_enabled_get(const Ewk_Settings *settings);

/**
 * Checks whether WebKit supports the @a encoding.
 *
 * @param encoding the encoding string to check whether WebKit supports it
 *
 * @return @c EINA_TRUE if WebKit supports @a encoding
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool ewk_settings_is_encoding_valid(const char *encoding);

/**
 * Sets link magnifier enabled.
 *
 * @param settings settings object
 * @param enabled link magnifier enabled
 */
EXPORT_API void ewk_settings_link_magnifier_enabled_set(Ewk_Settings *settings,
                                                        Eina_Bool enabled);

/**
 * Gets link magnifier enabled.
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE if link magnifier enabled, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool
ewk_settings_link_magnifier_enabled_get(const Ewk_Settings *settings);

/**
 * Requests to enable/disable link effect
 *
 * @param settings settings object to enable/disable link effect
 *
 * @param linkEffectEnabled @c EINA_TRUE to enable the link effect
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_link_effect_enabled_set(
    Ewk_Settings *settings, Eina_Bool linkEffectEnabled);

/**
 * Returns enable/disable link effect
 *
 * @param settings settings object to get whether link effect is enabled or
 * disabled
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_link_effect_enabled_get(const Ewk_Settings *settings);

/**
 * Requests to set using default keypad (default value : true)
 *
 * @param settings settings object to use default keypad
 * @param enable @c EINA_TRUE to use default keypad  @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_default_keypad_enabled_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns enable/disable using default keypad
 *
 * @param settings settings object to use default keypad
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_default_keypad_enabled_get(const Ewk_Settings *settings);

/**
 * Requests to set using keypad without user action (default value : true)
 *
 * @param settings settings object using keypad without user action
 * @param enable @c EINA_TRUE to use without user action @c EINA_FALSE to
 * disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_uses_keypad_without_user_action_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns using keypad without user action
 *
 * @param settings settings object using keypad without user action
 * @param settings settings object to query using keypad without user action
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_uses_keypad_without_user_action_get(const Ewk_Settings *settings);

/**
 * Requests setting use of text zoom.
 *
 * @param settings settings object to text zoom
 * @param enable to text zoom.
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_text_zoom_enabled_set(Ewk_Settings *settings,
                                                        Eina_Bool enable);

/**
 * Returns whether text zoom is enabled or not.
 *
 * @param settings settings object to text zoom
 *
 * @return @c EINA_TRUE if enable text zoom or @c EINA_FALSE.
 */
EXPORT_API Eina_Bool
ewk_settings_text_zoom_enabled_get(const Ewk_Settings *settings);

/**
 * Requests enable/disable text selection by default WebKit.
 *
 * @param settings setting object to set text selection by default WebKit
 * @param enable @c EINA_TRUE to enable text selection by default WebKit
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_text_selection_enabled_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns if text selection by default WebKit is enabled or disabled.
 *
 * @param settings setting object to get text selection by default WebKit
 *
 * @return @c EINA_TRUE if text selection by default WebKit is enabled
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_text_selection_enabled_get(const Ewk_Settings *settings);

/**
 * Requests enables/disables to clear text selection when webview lose focus
 *
 * @param settings setting object to set to clear text selection when webview
 * lose focus
 * @param enable @c EINA_TRUE to clear text selection when webview lose focus
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_clear_text_selection_automatically_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Enables/disables text autosizing.
 *
 * By default, the text autosizing is disabled.
 *
 * @param settings settings object to set the text autosizing
 * @param enable @c EINA_TRUE to enable the text autosizing
 *               @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 *
 * @see ewk_settings_text_autosizing_enabled_get()
 */
EXPORT_API Eina_Bool ewk_settings_text_autosizing_enabled_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns whether the text autosizing is enabled.
 *
 * The text autosizing is a feature which adjusts the font size of text in wide
 * columns, and makes text more legible.
 *
 * @param settings settings object to query whether text autosizing is enabled
 *
 * @return @c EINA_TRUE if the text autosizing is enabled
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_text_autosizing_enabled_get(const Ewk_Settings *settings);

/**
 * Requests to enable/disable edge effect
 *
 * @param settings settings object to enable/disable edge effect
 *
 * @param enable @c EINA_TRUE to enable the edge effect
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_settings_edge_effect_enabled_set(Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns enable/disable edge effect
 *
 * @param settings settings object to get whether edge effect is enabled or
 * disabled
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_edge_effect_enabled_get(const Ewk_Settings *settings);

/**
 * Requests to enable/disable to select word by double tap
 *
 * @param settings settings object to enable/disable to select word by double
 * tap
 * @param enable @c EINA_TRUE to select word by double tap
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_select_word_automatically_set(
    Ewk_Settings *settings, Eina_Bool enabled);

/**
 * Returns enable/disable text selection by double tap
 *
 * @param settings settings object to get whether word by double tap is selected
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_select_word_automatically_get(const Ewk_Settings *settings);

/**
 * Sets legacy font size mode
 *
 * @param settings settings object
 * @param mode legacy font size mode
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_current_legacy_font_size_mode_set(
    Ewk_Settings *settings, Ewk_Legacy_Font_Size_Mode mode);

/**
 * Returns set legacy font size mode
 *
 * @param settings settings object
 *
 * @return @c Ewk_Legacy_Font_Size_Mode set legacy font size mode
 */
EXPORT_API Ewk_Legacy_Font_Size_Mode
ewk_settings_current_legacy_font_size_mode_get(const Ewk_Settings *settings);

/**
 * Sets to paste image as URI (default: paste as base64-encoded-data)
 *
 * @param settings settings object
 * @param enable @c EINA_TRUE to paste image as URI    @c EINA_FALSE to paste
 * image as data
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_paste_image_uri_mode_set(
    Ewk_Settings *settings, Eina_Bool enabled);

/**
 * Returns whether  paste image as URI mode is enabled
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_paste_image_uri_mode_get(const Ewk_Settings *settings);

/**
 * DEPRECATED.
 * Gets the initial position value for the HTML list element <ul></ul>.
 *
 * @param settings setting object to get the initial position value
 *
 * @return the initial position value for the HTML list element.
 */
EINA_DEPRECATED EXPORT_API Ewk_List_Style_Position
ewk_settings_initial_list_style_position_get(const Ewk_Settings *settings);

/**
 * DEPRECATED.
 * Sets the initial position value for the HTML list element <ul></ul>.
 *
 * This value affect the lists that are going to be created,
 * does not make sense to manipulate it for existed elements.
 *
 * @param settings setting object to set the initial list style position
 * @param style a new style to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_settings_initial_list_style_position_set(Ewk_Settings *settings,
                                             Ewk_List_Style_Position style);

/**
 * Enable or disable supporting of -webkit-text-size-adjust
 *
 * -webkit-text-size-adjust affects text size adjusting feature.
 *
 * @param settings setting object to set the support of -webkit-text-size-adjust
 * @param enable @c EINA_TRUE to support -webkit-text-size-adjust, @c EINA_FALSE
 * not to support
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_webkit_text_size_adjust_enabled_set(
    Ewk_Settings *settings, Eina_Bool enabled);

/**
 * @brief Gets the staus of -webkit-text-size-adjust supporting.
 *
 * @since_tizen 2.3
 *
 * @param[in] settings setting object to get the status of
 * -webkit-text-size-adjust supporting
 *
 * @return the status of -webkit-text-size-adjust supporting
 */
EXPORT_API Eina_Bool
ewk_settings_webkit_text_size_adjust_enabled_get(const Ewk_Settings *settings);

/**
 * Requests to enable/disable to detect email address when tapping on email
 * address without link property
 *
 * @param settings settings object to enable/disable to detect email address
 * when tapping on email address without link property
 *
 * @param enable @c EINA_TRUE to enable to detect email address when tapping on
 * email address without link property
 *        @c EINA_FALSE to disable
 */
EXPORT_API void ewk_settings_detect_contents_automatically_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns enable/disable to detect email address when tapping on email address
 * without link property
 *
 * @param settings settings object to get whether email address is detected when
 * tapping on email address without link property
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_detect_contents_automatically_get(const Ewk_Settings *settings);

/**
 * Sets cache builder mode enabled.
 *
 * @param settings settings object
 * @param enabled cache builder mode
 */
EXPORT_API void ewk_settings_cache_builder_enabled_set(Ewk_Settings *settings,
                                                       Eina_Bool enabled);

/**
 * Requests enables/disables to the specific extra feature
 *
 * @param settings setting object to enable/disable the specific extra feature
 * @param feature feature name
 * @param enable @c EINA_TRUE to enable the specific extra feature
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API void ewk_settings_extra_feature_set(Ewk_Settings *settings,
                                               const char *feature,
                                               Eina_Bool enable);

/**
 * Returns enable/disable to the specific extra feature
 *
 * @param settings settings object to get whether the specific extra feature is
 * enabled or not.
 * @param feature feature name
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool ewk_settings_extra_feature_get(
    const Ewk_Settings *settings, const char *feature);

/**
 * Enables/disables the javascript access to clipboard.
 *
 * By default, JavaScript access to clipboard is disabled.
 *
 * @param settings settings object to set javascript access to clipboard
 * @param enable @c EINA_TRUE to enable javascript access to clipboard
 *               @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_javascript_can_access_clipboard_set(
    Ewk_Settings *settings, Eina_Bool enable);

/**
 * Returns whether javascript access to clipboard is enabled.
 *
 * @param settings settings object to query if the javascript can access to
 * clipboard
 *
 * @return @c EINA_TRUE if the javascript can access to clipboard
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_javascript_can_access_clipboard_get(const Ewk_Settings *settings);

/**
 * Enables/disables the DOM Paste is allowed.
 *
 * By default, DOM Paste is disabled.
 *
 * @param settings settings object to set DOM Paste allowance
 * @param enable @c EINA_TRUE to enable DOM Paste
 *               @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_dom_paste_allowed_set(Ewk_Settings *settings,
                                                        Eina_Bool enable);

/**
 * Returns whether DOM Paste is allowed.
 *
 * @param settings settings object to query if the DOM Paste is enabled.
 *
 * @return @c EINA_TRUE if the DOM Paste is enabled
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_dom_paste_allowed_get(const Ewk_Settings *settings);

/**
 * Sets compatibility mode with the given Tizen version.
 *
 * @since_tizen 3.0
 *
 * This API allows rendering the pages as if they are run with the specified
 * engine version. The compatibility mode changes behavior of web features
 * since they follow the newest specifications resulting in different behaviour
 * for web apps/pages written for earlier engines.
 *
 * To have content compatible with Tizen 2.4 (WebKit) one should call:
 *   ewk_settings_tizen_compatibility_mode_set(settings, 2, 4, 0);
 *
 * This API must be called before loading content.
 *
 * @param settings setting object on which compatibility mode is set
 * @param major major version
 * @param minor minor version
 * @param release release version
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_tizen_compatibility_mode_set(
    Ewk_Settings *settings, unsigned major, unsigned minor, unsigned release);

/**
 * Sets whether the webview supports multiple windows.
 *
 * If set to true, 'create,window' callback must be implemented by appliction.
 * If it is false, current webview will be reused to navigate.
 *
 * Default value is @c EINA_TRUE.
 *
 * @since_tizen 6.0
 *
 * @param settings setting object
 * @param support whether to support multiple windows
 */
EXPORT_API void ewk_settings_multiple_windows_support_set(
    Ewk_Settings *settings, Eina_Bool support);

/**
 * Gets whether the webview supports multiple windows.
 *
 * @since_tizen 6.0
 *
 * @param settings setting object
 */
EXPORT_API Eina_Bool
ewk_settings_multiple_windows_support_get(Ewk_Settings *settings);

#ifdef __cplusplus
}
#endif
#endif  // ewk_settings_internal_h
