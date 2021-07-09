// Copyright 2016 Samsung Electronics. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ewk_manifest_h
#define ewk_manifest_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @brief Get for web page manifest. Used for getting information about
 *        the manifest of web page.
 *
 * @since_tizen 3.0
 */
typedef struct _Ewk_View_Request_Manifest Ewk_View_Request_Manifest;

/**
 * \enum _Ewk_View_Orientation_Type
 * @brief  Enumeration that provides the type of orientation.
 *         These are attributes locking the screen orientation.
 *
 * @details It contains enum values used to specify orientation types.
 *
 * @since_tizen 3.0
 */
enum _Ewk_View_Orientation_Type {
  WebScreenOrientationLockDefault = 0,        /**< Equivalent to unlock. */
  WebScreenOrientationLockPortraitPrimary,    /**< portrait-primary */
  WebScreenOrientationLockPortraitSecondary,  /**< portrait-secondary */
  WebScreenOrientationLockLandscapePrimary,   /**< landscape-primary */
  WebScreenOrientationLockLandscapeSecondary, /**< landscape-secondary */
  WebScreenOrientationLockAny,                /**< any */
  WebScreenOrientationLockLandscape,          /**< landscape */
  WebScreenOrientationLockPortrait,           /**< portrait */
  WebScreenOrientationLockNatural,            /**< natural */
};

/**
 * @brief Enumeration that creates a type name for the
 * #_Ewk_View_Orientation_Type.
 *
 * @since_tizen 3.0
 */
typedef enum _Ewk_View_Orientation_Type Ewk_View_Orientation_Type;

/**
 * \enum _Ewk_View_Web_Display_Mode
 *
 * @brief  Enumeration that provides the mode of web display.
 *         These are attributes representing how the web application is being
 *         presented within the context.
 *
 * @details It contains enum values used to specify web display mode.
 *
 * @since_tizen 3.0
 */
enum _Ewk_View_Web_Display_Mode {
  WebDisplayModeUndefined = 0, /**< undefined */
  WebDisplayModeBrowser,       /**< browser */
  WebDisplayModeMinimalUi,     /**< minimul-ui */
  WebDisplayModeStandalone,    /**< standalone */
  WebDisplayModeFullscreen,    /**< fullscreen */
  WebDisplayModeLast = WebDisplayModeFullscreen
};

/**
 * @brief Enumeration that creates a type name for the
 * #_Ewk_View_Web_Display_Mode.
 *
 * @since_tizen 3.0
 */
typedef enum _Ewk_View_Web_Display_Mode Ewk_View_Web_Display_Mode;

/**
 * @brief Get the short name of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @return @c short name string if short name exists otherwise null.
 * The string is only valid until related Ewk_View_Request_Manifest object is
 * valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API const char* ewk_manifest_short_name_get(
    Ewk_View_Request_Manifest* manifest);

/**
 * @brief Get the name of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @return @c name string if name exists otherwise null. The string
 * is only valid until related Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API const char* ewk_manifest_name_get(
    Ewk_View_Request_Manifest* manifest);

/**
 * @brief Get the start url of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @return @c start url string if start url exists otherwise null. The string
 * is only valid until related Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API const char* ewk_manifest_start_url_get(
    Ewk_View_Request_Manifest* manifest);

/**
 * @brief Get the orientation type of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @return @c orientation type enum if orientation type exists
 * otherwise WebScreenOrientationLockDefault.
 * The enum is only valid until related
 * Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API Ewk_View_Orientation_Type
ewk_manifest_orientation_type_get(Ewk_View_Request_Manifest* manifest);

/**
 * @brief Get the web display mode of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @return @c web display mode enum if web display mode exists
 * otherwise WebDisplayModeUndefined.
 * The enum is only valid until related
 * Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API Ewk_View_Web_Display_Mode
ewk_manifest_web_display_mode_get(Ewk_View_Request_Manifest* manifest);

/**
 * @brief Get the theme color of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @param[out] r The red component of the theme color,
 *             Pass NULL if you don't want to get the value.
 * @param[out] g The green component of the theme color,
 *             Pass NULL if you don't want to get the value.
 * @param[out] b The blue component of the theme color,
 *             Pass NULL if you don't want to get the value.
 * @param[out] a The alpha component of the theme color,
 *             Pass NULL if you don't want to get the value.
 *
 * @return @c EINA_TRUE if theme color exists
 * otherwise EINA_FALSE. The value is only valid
 * until related Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API Eina_Bool
ewk_manifest_theme_color_get(Ewk_View_Request_Manifest* manifest, uint8_t* r,
                             uint8_t* g, uint8_t* b, uint8_t* a);

/**
 * @brief Get the background color of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @param[out] r The red component of the background color,
 *             Pass NULL if you don't want to get the value.
 * @param[out] g The green component of the background color,
 *             Pass NULL if you don't want to get the value.
 * @param[out] b The blue component of the background color,
 *             Pass NULL if you don't want to get the value.
 * @param[out] a The alpha component of the background color,
 *             Pass NULL if you don't want to get the value.
 *
 * @return @c EINA_TRUE if background color exists
 * otherwise EINA_FALSE. The value is only valid
 * until related Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API Eina_Bool ewk_manifest_background_color_get(
    Ewk_View_Request_Manifest* manifest, uint8_t* r, uint8_t* g, uint8_t* b,
    uint8_t* a);

/**
 * @brief Get the count of icon of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 *
 * @return @c count of icon if icons exist otherwise 0. The value
 * is only valid until related Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API size_t
ewk_manifest_icons_count_get(Ewk_View_Request_Manifest* manifest);

/**
 * @brief Get the src of icon of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 * @param[in] number the index of icons to be obtained
 *
 * @return @c src of icon if src exists otherwise null. The value
 * is only valid until related Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API const char* ewk_manifest_icons_src_get(
    Ewk_View_Request_Manifest* manifest, size_t number);

/**
 * @brief Get the type of icon of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 * @param[in] number the index of icons to be obtained
 *
 * @return @c type of icon if type exists otherwise null. The string
 * is only valid until related Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API const char* ewk_manifest_icons_type_get(
    Ewk_View_Request_Manifest* manifest, size_t number);

/**
 * @brief Get the count of the icon's sizes
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 * @param[in] number the index of icons to be obtained
 *
 * @return @c count of the icon's sizes if size of icons exists otherwise 0.
 * The value is only valid until related
 * Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API size_t ewk_manifest_icons_sizes_count_get(
    Ewk_View_Request_Manifest* manifest, size_t number);

/**
 * @brief Get the width of icon of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 * @param[in] number the index of icons to be obtained
 * @param[in] sizes_number the index of sizes to be obtained
 *
 * @return @c width of icon if width exists otherwise -1.
 * The value is only valid until related
 * Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API int ewk_manifest_icons_width_get(Ewk_View_Request_Manifest* manifest,
                                            size_t number, size_t sizes_number);

/**
 * @brief Get the height of icon of the manifest
 *
 * @since_tizen 3.0
 *
 * @param[in] manifest Manifest object to get manifest information.
 * @param[in] number the index of icons to be obtained
 * @param[in] sizes_number the index of sizes to be obtained
 *
 * @return @c height of icon if height exists otherwise -1.
 * The value is only valid until related
 * Ewk_View_Request_Manifest object is valid.
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API int ewk_manifest_icons_height_get(
    Ewk_View_Request_Manifest* manifest, size_t number, size_t sizes_number);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  // ewk_manifest_h
