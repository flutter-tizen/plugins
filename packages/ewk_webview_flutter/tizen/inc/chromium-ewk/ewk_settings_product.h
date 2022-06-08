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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
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
 * @file    ewk_settings_product.h
 * @brief   Describes the settings API.
 *
 * @note The ewk_settings is for setting the preference of specific ewk_view.
 * We can get the ewk_settings from ewk_view using ewk_view_settings_get() API.
 */

#ifndef ewk_settings_product_h
#define ewk_settings_product_h

#include "ewk_settings_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Requests setting of force zoom.
 *
 * @param settings settings object to enable force zoom
 * @param enable to force zoom
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_force_zoom_set(Ewk_Settings* settings,
                                                 Eina_Bool enable);

/**
 * Returns the force zoom status.
 *
 * @param settings settings object to enable force zoom
 *
 * @return @c EINA_TRUE if enable force zoom or @c EINA_FALSE.
 */
EXPORT_API Eina_Bool ewk_settings_force_zoom_get(const Ewk_Settings* settings);

/**
 * Requests to set the default font size.
 *
 * @param settings settings object to set the default font size
 * @param size a new default font size to set
 *
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_settings_font_default_size_set(Ewk_Settings* settings,
                                                        int size);

/**
 * Returns the default font size.
 *
 * @param settings settings object to set the default font size
 *
 * @return @c default font size.
 */
EXPORT_API int ewk_settings_font_default_size_get(const Ewk_Settings* settings);

/**
 * Requests enables/disables if the scripts can open the new windows.
 *
 * @param settings settings object to set if the scripts can open the new
 * windows
 * @param allow @c EINA_TRUE if the scripts can open the new windows
 *        @c EINA_FALSE if not
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure (scripts are
 * disabled)
 */
EXPORT_API Eina_Bool
ewk_settings_scripts_window_open_set(Ewk_Settings* settings, Eina_Bool allow);

/**
 * Returns enables/disables if the scripts can open the new windows.
 *
 * @param settings settings object to set if the scripts can open the new
 * windows
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure (scripts are
 * disabled)
 */
EXPORT_API Eina_Bool
ewk_settings_scripts_window_open_get(const Ewk_Settings* settings);

/**
 * Returns default text encoding name.
 *
 * @param settings settings object to query default text encoding nae
 *
 * @return default text encoding name
 */
EXPORT_API const char* ewk_settings_default_encoding_get(
    const Ewk_Settings* settings);

/**
 * Requests to set default text encoding name.
 *
 * @param settings settings object to set default text encoding name
 * @param encoding default text encoding name
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_default_encoding_set(Ewk_Settings* settings,
                                                       const char* encoding);

/**
 * Requests to set editable link behavior.
 *
 * @param settings settings object to set editable link behavior
 * @param behavior editable link behaviro
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_editable_link_behavior_set(
    Ewk_Settings* settings, Ewk_Editable_Link_Behavior behavior);

/**
 * Requests to set the load remote images enable/disable
 *
 * @param settings settings object to set load remote images
 *
 * @param loadRemoteImages @c EINA_TRUE to enable the load remote images
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_load_remote_images_set(
    Ewk_Settings* settings, Eina_Bool loadRemoteImages);

/**
 * Returns enable/disable the load remote images
 *
 * @param settings settings object to get editable link behavior
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_load_remote_images_get(const Ewk_Settings* settings);

/**
 * Returns uses encoding detector.
 *
 * @deprecated Deprecated since Tizen 3.0.
 *
 * @param settings settings object to query uses encoding detector
 *
 * @see ewk_settings_encoding_detector_enabled_get
 *
 * @return uses encoding detector
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_settings_uses_encoding_detector_get(const Ewk_Settings* settings);

/**
 * Returns if password form autofill is enabled or disabled.
 *
 * @param setting setting object to get password form autofill
 *
 * @return @c EINA_TRUE if password form autofill is enabled
 *         @c EINA_FALSE if password form autofill is disabled
 */
EXPORT_API Eina_Bool
ewk_settings_autofill_password_form_enabled_get(Ewk_Settings* settings);

/**
 * Returns if form candidate data for autofill is enabled or disabled.
 *
 * @param setting setting object to get form candidate data for autofill
 *
 * @return @c EINA_TRUE if form candidate data for autofill is enabled
 *         @c EINA_FALSE if form candidate data for autofill is disabled
 */
EXPORT_API Eina_Bool
ewk_settings_form_candidate_data_enabled_get(Ewk_Settings* settings);

/**
 * Returns whether the autofill_text feature is enabled.
 *
 * @param settings settings object to query whether autofill_text feature is
 * enabled
 *
 * @return @c EINA_TRUE if the autofill_text feature is enabled
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_form_profile_data_enabled_get(const Ewk_Settings* settings);

/**
 * Returns whether text selection is cleared when webview lose focus or not.
 *
 * @param settings setting object to get whether text selection is cleared when
 * webview lose focus or not
 *
 * @return @c EINA_TRUE if text selection is cleared when webview lose focus
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool ewk_settings_clear_text_selection_automatically_get(
    const Ewk_Settings* settings);

/**
 * Requests for drawing layer borders.
 *
 * @param settings settings object to drawing layer borders.
 * @param enable EINA_TRUE to draw layer borders.
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */

EXPORT_API Eina_Bool ewk_settings_compositing_borders_visible_set(
    Ewk_Settings* settings, Eina_Bool enable);

/**
 * Requests to set the scan malware enable/disable.
 *
 * @param settings settings object to set scan malware
 *
 * @param scan_malware_enabled @c EINA_TRUE to enable the scan malware
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_scan_malware_enabled_set(
    Ewk_Settings* settings, Eina_Bool scan_malware_enabled);

/**
 * @brief Returns enable/disable scan malware.
 *
 * @since_tizen 2.3
 *
 * @param[in] settings settings object to get malware scan behavior
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_scan_malware_enabled_get(const Ewk_Settings* settings);

/**
 * Requests to enable/disable spdy.
 *
 * @deprecated Deprecated since Tizen 4.0.
 *
 * @param settings param not needed, only present for API compatibility
 *
 * @param spdy_enabled @c EINA_TRUE to enable the spdy @c EINA_FALSE to disable
 *
 * @return always @c EINA_TRUE, only present for API compatibility
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_settings_spdy_enabled_set(Ewk_Settings* settings, Eina_Bool spdy_enabled);

/**
 * Get spdy enabled/disabled state.
 *
 * @deprecated Deprecated since Tizen 4.0.
 *
 * @param settings param not needed, only present for API compatibility
 *
 * @return @c EINA_TRUE if enabled or @c EINA_FALSE if disabled
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_settings_spdy_enabled_get(Ewk_Settings* settings);

/**
 * Requests to set the performance features of soup enable/disable.
 *
 * @param settings settings object to set performance features on soup
 *
 * @param spdy_enabled @c EINA_TRUE to enable the performance features on soup
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_performance_features_enabled_set(
    Ewk_Settings* settings, Eina_Bool performance_features_enabled);

/**
 * @brief Returns enable/disable performance features on soup.
 *
 * @since_tizen 2.3
 *
 * @param[in] settings settings object to get performance features
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool
ewk_settings_performance_features_enabled_get(const Ewk_Settings* settings);

/**
 * Requests to set using encoding detector.
 *
 * @deprecated Deprecated since Tizen 3.0.
 *
 * @param settings settings object to set using encoding detector
 * @param use use encoding detector
 *
 * @see ewk_settings_encoding_detector_enabled_set
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_settings_uses_encoding_detector_set(Ewk_Settings* settings, Eina_Bool use);

/**
 * Enables/disables the encoding detector.
 *
 * By default, the encoding detector is disabled.
 *
 * @param settings settings object to set the encoding detector
 * @param enable @c EINA_TRUE to enable the encoding detector,
 *        @c EINA_FALSE to disable
 *
 * @since_tizen 3.0
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_encoding_detector_enabled_set(
    Ewk_Settings* settings, Eina_Bool enable);

/**
 * Queries whether the encoding detector is enabled or not.
 *
 * @since_tizen 3.0
 *
 * @param settings settings object to query using encoding detector
 *
 * @return @c EINA_TRUE if encoding detecor is enabled
 *         @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool
ewk_settings_encoding_detector_enabled_get(const Ewk_Settings* settings);

/**
 * Set to load https sub resource when it has certificate error.
 *
 * @param settings settings object to enable/disable load sub resource
 * @param enabled a state to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_load_sub_resource_with_certificate_error_set(
    Ewk_Settings* settings, Eina_Bool enabled);

/*
 * Set to add http head DNT(do not track).
 *
 * @param settings settings object to enable/disable set DNT in http head.
 * @param enabled a state to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_do_not_track_set(Ewk_Settings* settings,
                                                   Eina_Bool enabled);

/**
 *@brief Set to allow running mixed contents or not.
 *
 * @since_tizen 3.0
 *
 * @param[in] settings The settings object to set mixed contents reply
 * @param[in] allow If @c EINA_TRUE allow to run mixed contents\n
 *      otherwise @c EINA_FALSE to not allow running mixed contents
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_settings_mixed_contents_set(const Ewk_Settings* settings, Eina_Bool allow);

/**
 * @deprecated Deprecated since Tizen 3.0.
 *
 * Enable/disable cache builder extension mode.
 *
 * By default, the cache builder extension is disabled.
 * Notify node position to client when focused node is change.
 * Can find focusable node from last known mouse position.
 * Support fast scroll when long pressing the direction key.
 *
 * @param settings settings object
 * @param enabled @c EINA_TRUE to enable the cache builder extension
 *                @c EINA_FALSE to disable
 */
EINA_DEPRECATED EXPORT_API void
ewk_settings_cache_builder_extension_enabled_set(Ewk_Settings* settings,
                                                 Eina_Bool enabled);

/**
 * @deprecated Deprecated since Tizen 3.0.
 *
 * Enable/disable focus ring.
 *
 * @note Focus ring is enabled by default but disabled for wrt on TV profile
 *
 * @param settings settings object
 * @param enabled @c EINA_TRUE to enable the focus ring
 *                @c EINA_FALSE to disable
 */
EINA_DEPRECATED EXPORT_API void ewk_settings_focus_ring_enabled_set(
    Ewk_Settings* settings, Eina_Bool enabled);

/**
 * Enables/disables the viewport meta tag.
 *
 * By default, the viewport meta tag is enabled on mobile and wearable,
 * but it is disabled on TV.
 *
 * @param settings settings object
 * @param enable @c EINA_TRUE to enable the viewport meta tag
 *               @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_viewport_meta_tag_set(Ewk_Settings* settings,
                                                        Eina_Bool enable);

/**
 * Allow/disallow to run and display mixed contents.
 *
 * By default, WebCore don't allow run and display mixed contents.
 * Some Apps want to allow them by default
 *
 * @param settings settings object
 * @param enable @c EINA_TRUE to allow run and display mixed contents
 *               @c EINA_FALSE to disable
 *
 */
EXPORT_API void ewk_settings_default_mixed_contents_policy_set(
    Ewk_Settings* settings, Eina_Bool enable);

/**
 * Enable/disable the WebGL feature.
 *
 * By default, the WebGL feature is Enabled.
 * Some Apps want to disallow WebGL feature.
 *
 * @note Should be used before ewk_view_url_set().
 *
 * @param settings settings object
 * @param disable @c EINA_TRUE to disable WebGL.
 *                @c EINA_FALSE to enable.
 *
 */
EXPORT_API void ewk_settings_disable_webgl_set(Ewk_Settings* settings,
                                               Eina_Bool disable);

/**
 * Returns whether the viewport meta tag is enabled.
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE if the viewport meta tag is enabled
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_viewport_meta_tag_get(const Ewk_Settings* settings);

/**
 * Enables/disables web security.
 *
 * By default, the web security is enabled.
 *
 * @param settings settings object to set the web security
 * @param enable @c EINA_TRUE to enable the web security
 *             @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 *
 * @see ewk_settings_web_security_enabled_get()
 */
EXPORT_API Eina_Bool
ewk_settings_web_security_enabled_set(Ewk_Settings* settings, Eina_Bool enable);

/**
 * Returns whether the web security is enabled.
 *
 * @param settings settings object to query whether web security is enabled
 *
 * @return @c EINA_TRUE if the web security is enabled
 *       @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_web_security_enabled_get(const Ewk_Settings* settings);

/**
 * Set to uses the arrow scroll.
 *
 * @param settings settings object
 * @param enabled a state to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_uses_arrow_scroll_set(Ewk_Settings* settings,
                                                        Eina_Bool enabled);

/**
 * Returns whether uses arrow scroll is enabled or not.
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE if uses arrow is enabled, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_settings_uses_arrow_scroll_get(Ewk_Settings* settings);

/**
 * Set to uses scrollbar thumb focus notifications.
 *
 * @param settings settings object
 * @param enabled a state to set
 */
EXPORT_API Eina_Bool ewk_settings_uses_scrollbar_thumb_focus_notifications_set(
    Ewk_Settings* settings, Eina_Bool use);

/**
 * Allow/Disallow file access from external url
 *
 * By default, file access from external url is disallowed
 *
 * This is only for TV Product
 *
 * @param settings settings object to allow file access from external url
 * @param enable @c EINA_TRUE to allow file access from external url
 *             @c EINA_FALSE to disallow
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 *
 * @see ewk_settings_allow_file_access_from_external_url_get()
 */
EXPORT_API Eina_Bool ewk_settings_allow_file_access_from_external_url_set(
    Ewk_Settings* settings, Eina_Bool allow);

/**
 * Set to support the media playback notification.
 *
 * This is only for TV Product
 *
 * @param settings settings object
 * @param enabled a state to set
 *
 */
EXPORT_API void ewk_settings_media_playback_notification_set(
    Ewk_Settings* settings, Eina_Bool enabled);

/**
 * Returns enable/disable the media playback notification.
 *
 * This is only for TV Product
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE if media plyaback notification is enabled, @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool
ewk_settings_media_playback_notification_get(const Ewk_Settings* settings);

/**
 * Set to support the subtitle notification.
 *
 * @param settings settings object
 * @param enabled a state to set
 *
 */
EXPORT_API void ewk_settings_media_subtitle_notification_set(
    Ewk_Settings* settings, Eina_Bool enabled);

/**
 * Returns enable/disable the subtitle notification.
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE if subtitle notification is enabled, @c EINA_FALSE
otherwise
 */
EXPORT_API Eina_Bool
ewk_settings_media_subtitle_notification_get(const Ewk_Settings* settings);

/**
 * Returns whether file access from external url is enabled
 *
 * This is only for TV Product
 *
 * @param settings settings object to query whether file access from external
 * url is enabled
 *
 * @return @c EINA_TRUE if file access from external url is allow
 *       @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool ewk_settings_allow_file_access_from_external_url_get(
    const Ewk_Settings* settings);

/**
 * @brief Requests to enable/disable private browsing.
 *
 * @details This setting change affects all webview with same context.
 *
 * @since_tizen 2.3
 *
 * @param[in] settings The settings object to set private browsing
 * @param[in] enable If @c EINA_TRUE private browsing is enabled\n
 *                   otherwise @c EINA_FALSE to disable it
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_private_browsing_enabled_set(
    Ewk_Settings* settings, Eina_Bool enable);

/**
 * @brief Enables/disables private browsing.
 *
 * @since_tizen 2.3
 *
 * @param[in] settings The settings object to query if private browsing is
 *            enabled
 *
 * @return @c EINA_TRUE if private browsing is enabled,\n
 *         otherwise @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool
ewk_settings_private_browsing_enabled_get(const Ewk_Settings* settings);

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
EXPORT_API void ewk_settings_extra_feature_set(Ewk_Settings* settings,
                                               const char* feature,
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
    const Ewk_Settings* settings, const char* feature);

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
    Ewk_Settings* settings, Eina_Bool enable);

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
ewk_settings_text_autosizing_enabled_get(const Ewk_Settings* settings);

/**
 * Sets the scale factor for text autosizing.
 *
 * Default value is 1.0.
 *
 * @param settings settings object to set the text autosizing
 * @param factor font scale factor for text autosizing
 */
EXPORT_API Eina_Bool ewk_settings_text_autosizing_font_scale_factor_set(
    Ewk_Settings* settings, double factor);

/**
 * Gets the current scale factor for text autosizing.
 *
 * @param settings settings object to set scale factor for text autosizing
 *
 * @return the current font scale factor for text autosizing.
 * In case of error, it returns non-positive value.
 */
EXPORT_API double ewk_settings_text_autosizing_font_scale_factor_get(
    const Ewk_Settings* settings);

/**
 * @brief Sets the scale factor for text autosizing.
 *
 * @details Default value is 1.0.
 *
 * @param settings settings object to set the text autosizing
 * @param factor font scale factor for text autosizing
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_text_autosizing_scale_factor_set(
    Ewk_Settings* settings, double factor);

/**
 * @brief Gets the current scale factor for text autosizing.
 *
 * @param settings settings object to set scale factor for text autosizing
 *
 * @return the current font scale factor for text autosizing
 */
EXPORT_API double ewk_settings_text_autosizing_scale_factor_get(
    const Ewk_Settings* settings);

/**
 * Sets text style for selection mode enabled.
 *
 * @param settings settings object
 * @param enabled text style for selection mode
 */

EXPORT_API void ewk_settings_text_style_state_enabled_set(
    Ewk_Settings* settings, Eina_Bool enabled);

/**
 * Gets text style for selection mode enabled.
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE if text style for selection mode enabled, @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool
ewk_settings_text_style_state_enabled_get(const Ewk_Settings* settings);

/**
 * @brief Enables/disables legacy font size mode
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] settings settings object
 * @param[in] enable If @c EINA_TRUE legacy font size is enabled\n
 *            otherwise @c EINA_FALSE to disable it
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_legacy_font_size_enabled_set(
    Ewk_Settings* settings, Eina_Bool enabled);

/**
 * @brief Return whether legacy font size mode is enabled
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] settings settings object
 *
 * @return @c EINA_TRUE if Ewk_Legacy_Font_Size_Mode set to legacy font size
 * mode
 *        @c EINA_FALSE if Ewk_Legacy_Font_Size_Mode not set to legacy font size
 * mode
 */
EXPORT_API Eina_Bool
ewk_settings_legacy_font_size_enabled_get(Ewk_Settings* settings);

/**
 * @brief Sets font-family as system font for font rendering
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] settings settings object
 * @param[in] use @c EINA_TRUE to use one of the system fonts which is selected
 * by user in Settings
 *               @c EINA_FALSE to use a system default font
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_settings_use_system_font_set(Ewk_Settings* settings,
                                                      Eina_Bool use);

/**
 * @brief Returns whether we use the system font which is selected by user in
 * Settings or use a system default font
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] settings settings object
 *
 * @return @c EINA_TRUE if we use the sysem font which is selected by user in
 * Settings
 *        @c EINA_FALSE if we use a system default font or on failure
 */
EXPORT_API Eina_Bool ewk_settings_use_system_font_get(Ewk_Settings* settings);

/**
 * @brief Requests enables/disables to control text selection handles from app
 *
 * @since_tizen 2.3
 *
 * @param[in] settings setting object to set to control text selection handles
 * from app
 * @param[in] enable @c EINA_TRUE to control text selection handles from app\n
 *        @c EINA_FALSE to disable
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API void ewk_settings_selection_handle_enabled_set(
    Ewk_Settings* settings, Eina_Bool enable);

/**
* @brief Returns whether text selection handles are controlled from app or not
*
* @since_tizen 2.3
*
* @param[in] settings setting object to get whether text selection handles are
controlled from app or not
*
* @return @c EINA_TRUE if text selection handles are controlled from app\n
*         @c EINA_FALSE if not or on failure
*/
EXPORT_API Eina_Bool
ewk_settings_selection_handle_enabled_get(const Ewk_Settings* settings);

/**
 * @brief disclose Set-Cookie headers over IPC.
 *        Some apps which get cookie through xhr.getAllResponseHeaders interface
 *        Currently blink don't disclose response header with Set-Cookie field
 *        to XMLHttpRequest.
 *
 * @since_tizen 3.0
 *
 * @param[in] settings setting object
 * @param[in] Enabled true means app allow to disclose "Set-Cookie"
 *
 */
EXPORT_API void ewk_settings_disclose_set_cookie_headers_enabled(
    Ewk_Settings* settings, Eina_Bool Enabled);

/**
 * @brief Request to set the spatial navigation usage set by hbbtv
 *
 * @since_tizen 3.0
 *
 * @param[in] settings setting object
 * @param[in] enable @c EINA_TRUE enable to use spatial navigation
 *                   @c EINA_FALSE to disable
 */
EXPORT_API void ewk_settings_spatial_navigation_enabled_set(
    Ewk_Settings* settings, Eina_Bool enable);

/**
 * Request to set enable/disable the ime panel
 *
 * By default, the ime panel is enabled
 * Some Apps want to disable ime panel
 *
 * @param settings setting object
 * @param enabled @c EINA_TRUE to enable ime panel
 *                @c EINA_FALSE to disable ime panel
 *
 */
EXPORT_API void ewk_settings_ime_panel_enabled_set(Ewk_Settings* settings,
                                                   Eina_Bool enabled);

/**
 * Returns whether the ime panel is enabled or disabled
 *
 * @param settings setting object
 *
 * @return @c EINA_TRUE enable ime panel or @c EINA_FALSE disable ime panel
 */
EXPORT_API Eina_Bool
ewk_settings_ime_panel_enabled_get(const Ewk_Settings* settings);

/**
 * Request to set enable/disable drag and drop
 *
 * By default, the drag and drop is Disabled
 *
 * @param settings settings object
 * @param enabled @c EINA_TRUE to enable drag and drop
 *                @c EINA_FALSE to disable drag and drop
 *
 */
EXPORT_API void ewk_settings_drag_drop_enabled_set(Ewk_Settings* settings,
                                                   Eina_Bool enabled);

/**
 * Returns whether drag and drop is enabled or disabled
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE drag and drop is enabled, @c EINA_FALSE drag and drop is
 * disabled
 */
EXPORT_API Eina_Bool
ewk_settings_drag_drop_enabled_get(const Ewk_Settings* settings);

/**
 * Request to set enable/disable clipboard
 *
 * By default, the clipboard is Disabled
 *
 * @param settings settings object
 * @param enabled @c EINA_TRUE to enable clipboard
 *                @c EINA_FALSE to disable clipboard
 *
 */
EXPORT_API void ewk_settings_clipboard_enabled_set(Ewk_Settings* settings,
                                                   Eina_Bool enabled);

/**
 * Returns whether clipboard is enabled
 *
 * @param settings settings object
 *
 * @return @c EINA_TRUE clipboard is enabled, @c EINA_FALSE clipboard is
 * disabled
 */
EXPORT_API Eina_Bool
ewk_settings_clipboard_enabled_get(const Ewk_Settings* settings);

/**
 * Request to enable/disable create select menu by client
 *
 * By default, the disabled create select menu by client
 *
 * @param settings settings object
 * @param enabled @c EINA_TRUE to enable create select pick by client
 *                @c EINA_FALSE to disable create select pick by client
 *
 */
EXPORT_API void ewk_settings_create_select_menu_by_client(
    Ewk_Settings* settings, Eina_Bool enabled);

#ifdef __cplusplus
}
#endif
#endif  // ewk_settings_product_h
