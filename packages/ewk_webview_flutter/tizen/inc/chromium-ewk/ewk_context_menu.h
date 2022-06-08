/*
 * Copyright (C) 2013-2016 Samsung Electronics.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * @file    ewk_context_menu.h
 * @brief   Describes the context menu API.
 */

#ifndef ewk_context_menu_h
#define ewk_context_menu_h

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
 * \enum    _Ewk_Context_Menu_Item_Tag
 * @brief   Enumeration that provides the tags of items for the context menu.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 */
enum _Ewk_Context_Menu_Item_Tag {
  EWK_CONTEXT_MENU_ITEM_TAG_NO_ACTION = 0,           /**< No action */
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK_IN_NEW_WINDOW, /**< Open link in new
                                                        window */
  EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_LINK_TO_DISK, /**< Download link to disk */
  EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_TO_CLIPBOARD, /**< Copy link to clipboard
                                                     */
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_NEW_WINDOW,     /**< Open image in new
                                                             window */
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_IMAGE_IN_CURRENT_WINDOW, /**< Open image in
                                                             current window */
  EWK_CONTEXT_MENU_ITEM_TAG_DOWNLOAD_IMAGE_TO_DISK,  /**< Download image to disk
                                                      */
  EWK_CONTEXT_MENU_ITEM_TAG_COPY_IMAGE_TO_CLIPBOARD, /**< Copy image to
                                                        clipboard */
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_FRAME_IN_NEW_WINDOW, /**< Open frame in new
                                                         window */
  EWK_CONTEXT_MENU_ITEM_TAG_COPY,                     /**< Copy */
  EWK_CONTEXT_MENU_ITEM_TAG_GO_BACK,                  /**< Go back */
  EWK_CONTEXT_MENU_ITEM_TAG_GO_FORWARD,               /**< Go forward */
  EWK_CONTEXT_MENU_ITEM_TAG_STOP,                     /**< Stop */
  EWK_CONTEXT_MENU_ITEM_TAG_SHARE,                    /**< Share */
  EWK_CONTEXT_MENU_ITEM_TAG_RELOAD,                   /**< Reload */
  EWK_CONTEXT_MENU_ITEM_TAG_CUT,                      /**< Cut  */
  EWK_CONTEXT_MENU_ITEM_TAG_PASTE,                    /**< Paste */
  EWK_CONTEXT_MENU_ITEM_TAG_SPELLING_GUESS,           /**< Spelling guess */
  EWK_CONTEXT_MENU_ITEM_TAG_NO_GUESSES_FOUND,         /**< Guess found */
  EWK_CONTEXT_MENU_ITEM_TAG_IGNORE_SPELLING,          /**< Ignore spelling */
  EWK_CONTEXT_MENU_ITEM_TAG_LEARN_SPELLING,           /**< Learn spelling */
  EWK_CONTEXT_MENU_ITEM_TAG_OTHER,                    /**< Other */
  EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_IN_SPOTLIGHT,   /**< Search in spotlight */
  EWK_CONTEXT_MENU_ITEM_TAG_SEARCH_WEB,            /**< Search web */
  EWK_CONTEXT_MENU_ITEM_TAG_LOOK_UP_IN_DICTIONARY, /**< Look up in dictionary */
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_WITH_DEFAULT_APPLICATION, /**< Open with
                                                              default
                                                              application */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_ACTUAL_SIZE,     /**< PDF actual size */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_ZOOM_IN,         /**< PDF zoom in */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_ZOOM_OUT,        /**< PDF zoom out */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_AUTO_SIZE,       /**< PDF auto size */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_SINGLE_PAGE,     /**< PDF single page */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_FACTING_PAGES,   /**< PDF facting page */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_CONTINUOUS,      /**< PDF continuous */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_NEXT_PAGE,       /**< PDF next page */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_PREVIOUS_PAGE,   /**< PDF previous page */
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_LINK,           /**< Open link */
  EWK_CONTEXT_MENU_ITEM_TAG_IGNORE_GRAMMAR,      /**< Ignore grammar */
  EWK_CONTEXT_MENU_ITEM_TAG_SPELLING_MENU,       /**< Spelling menu */
  EWK_CONTEXT_MENU_ITEM_TAG_SHOW_SPELLING_PANEL, /**< Show spelling panel */
  EWK_CONTEXT_MENU_ITEM_TAG_CHECK_SPELLING,      /**< Check spelling */
  EWK_CONTEXT_MENU_ITEM_TAG_CHECK_SPELLING_WHILE_TYPING, /**< Check spelling
                                                            white typing */
  EWK_CONTEXT_MENU_ITEM_TAG_CHECK_GRAMMAR_WITH_SPELLING, /**< Check grammar with
                                                            spelling */
  EWK_CONTEXT_MENU_ITEM_TAG_FONT_MENU,                   /**< Font menu */
  EWK_CONTEXT_MENU_ITEM_TAG_SHOW_FONTS,                  /**< Show fonts */
  EWK_CONTEXT_MENU_ITEM_TAG_BOLD,                        /**< Bold */
  EWK_CONTEXT_MENU_ITEM_TAG_ITALIC,                      /**< Italic */
  EWK_CONTEXT_MENU_ITEM_TAG_UNDERLINE,                   /**< Underline */
  EWK_CONTEXT_MENU_ITEM_TAG_OUTLINE,                     /**< Outline */
  EWK_CONTEXT_MENU_ITEM_TAG_STYLES,                      /**< Style */
  EWK_CONTEXT_MENU_ITEM_TAG_SHOW_COLORS,                 /**< Show colors */
  EWK_CONTEXT_MENU_ITEM_TAG_SPEECH_MENU,                 /**< Speech menu */
  EWK_CONTEXT_MENU_ITEM_TAG_START_SPEAKING,              /**< Start speaking */
  EWK_CONTEXT_MENU_ITEM_TAG_STOP_SPEAKING,               /**< Stop speaking */
  EWK_CONTEXT_MENU_ITEM_TAG_WRITING_DIRECTION_MENU, /**< Writing direction menu
                                                     */
  EWK_CONTEXT_MENU_ITEM_TAG_DEFAULT_DIRECTION,      /**< Default direction */
  EWK_CONTEXT_MENU_ITEM_TAG_LEFT_TO_RIGHT,          /**< Left to right */
  EWK_CONTEXT_MENU_ITEM_TAG_RIGHT_TO_LEFT,          /**< Right to left */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_SINGLE_PAGE_SCROLLING,  /**< PDF single page
                                                           scrolling */
  EWK_CONTEXT_MENU_ITEM_TAG_PDF_FACING_PAGES_SCROLLING, /**< PDF facing page
                                                           scrolling */
  EWK_CONTEXT_MENU_ITEM_TAG_INSPECT_ELEMENT,            /**< Inspect element */
  EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_MENU,    /**< Text direction menu */
  EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_DEFAULT, /**< Text direction default
                                                     */
  EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_LEFT_TO_RIGHT,   /**< Text direction
                                                               left to right */
  EWK_CONTEXT_MENU_ITEM_TAG_TEXT_DIRECTION_RIGHT_TO_LEFT,   /**< Text direction
                                                               right to left */
  EWK_CONTEXT_MENU_ITEM_TAG_CORRECT_SPELLING_AUTOMATICALLY, /**< Correct
                                                               spelling
                                                               automatically */
  EWK_CONTEXT_MENU_ITEM_TAG_SUBSTITUTIONS_MENU,   /**< Substitutions menu */
  EWK_CONTEXT_MENU_ITEM_TAG_SHOW_SUBSTITUTIONS,   /**< Show substitutions */
  EWK_CONTEXT_MENU_ITEM_TAG_SMART_COPY_PASTE,     /**< Smart copy paste */
  EWK_CONTEXT_MENU_ITEM_TAG_SMART_QUOTES,         /**< Smart quotes */
  EWK_CONTEXT_MENU_ITEM_TAG_SMART_DASHES,         /**< Smart dashes */
  EWK_CONTEXT_MENU_ITEM_TAG_SMART_LINKS,          /**< Smart links */
  EWK_CONTEXT_MENU_ITEM_TAG_TEXT_REPLACEMENT,     /**< Text replacement */
  EWK_CONTEXT_MENU_ITEM_TAG_TRANSFORMATIONS_MENU, /**< Transformation menu */
  EWK_CONTEXT_MENU_ITEM_TAG_MAKE_UPPER_CASE,      /**< Make upper case */
  EWK_CONTEXT_MENU_ITEM_TAG_MAKE_LOWER_CASE,      /**< Make lower case */
  EWK_CONTEXT_MENU_ITEM_TAG_CAPITALIZE,           /**< Capitalize */
  EWK_CONTEXT_MENU_ITEM_TAG_CHANGE_BACK,          /**< Change back */
  EWK_CONTEXT_MENU_ITEM_TAG_OPEN_MEDIA_IN_NEW_WINDOW,     /**< Open media in new
                                                             window */
  EWK_CONTEXT_MENU_ITEM_TAG_COPY_MEDIA_LINK_TO_CLIPBOARD, /**< Copy media link
                                                             to clipboard */
  EWK_CONTEXT_MENU_ITEM_TAG_TOGGLE_MEDIA_CONTROLS, /**< Toggle media controls */
  EWK_CONTEXT_MENU_ITEM_TAG_TOGGLE_MEDIA_LOOP,     /**< Toggle media loop */
  EWK_CONTEXT_MENU_ITEM_TAG_ENTER_VIDEO_FULLSCREEN, /**< Enter video fullscreen
                                                     */
  EWK_CONTEXT_MENU_ITEM_TAG_MEDIA_PLAY_PAUSE,       /**< Media play pause */
  EWK_CONTEXT_MENU_ITEM_TAG_MEDIA_MUTE,             /**< Media mute */
  EWK_CONTEXT_MENU_ITEM_TAG_DICTATION_ALTERNATIVE, /**< Dictation alternative */
  EWK_CONTEXT_MENU_ITEM_TAG_SELECT_ALL,            /**< Select all */
  EWK_CONTEXT_MENU_ITEM_TAG_SELECT_WORD,           /**< Select word */
  EWK_CONTEXT_MENU_ITEM_TAG_TEXT_SELECTION_MODE,   /**< Text selection mode */
  EWK_CONTEXT_MENU_ITEM_TAG_CLIPBOARD,             /**< Clipboard */
  EWK_CONTEXT_MENU_ITEM_TAG_DRAG,                  /**< Drag */
  EWK_CONTEXT_MENU_ITEM_TAG_TRANSLATE,             /**< Translate */
  EWK_CONTEXT_MENU_ITEM_TAG_COPY_LINK_DATA,        /**< Copy link data */
  EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG =
      10000 /**< If app want to add customized item, use enum value after
               #EWK_CONTEXT_MENU_ITEM_BASE_APPLICATION_TAG */
};

/**
 * @brief The structure type that creates a type name for
 * Ewk_Context_Menu_Item_Tag.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 */
typedef uint32_t Ewk_Context_Menu_Item_Tag;

/**
 * @brief The structure type that creates a type name for _Ewk_Context_Menu.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 */
typedef struct _Ewk_Context_Menu Ewk_Context_Menu;

/**
 * @brief The structure type that creates a type name for
 * _Ewk_Context_Menu_Item.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 */
typedef struct _Ewk_Context_Menu_Item Ewk_Context_Menu_Item;

/**
 * @brief Counts the number of the context menu item.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] menu The context menu object
 *
 * @return The number of current context menu item
 */
EXPORT_API unsigned ewk_context_menu_item_count(Ewk_Context_Menu* menu);

/**
 * @brief Returns the nth item in a context menu.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] menu The context menu object
 * @param[in] n The number of the item
 *
 * @return The nth item of context menu
 */
EXPORT_API Ewk_Context_Menu_Item* ewk_context_menu_nth_item_get(
    Ewk_Context_Menu* menu, unsigned int n);

/**
 * @brief Removes the context menu item from the context menu object.
 *
 * @remarks If all context menu items are removed, neither context menu nor\n
 *          selection handles will be shown.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] menu The context menu object
 * @param[in] item The context menu item to remove
 *
 * @return @c EINA_TRUE on successful request,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_menu_item_remove(Ewk_Context_Menu* menu,
                                                  Ewk_Context_Menu_Item* item);

/**
 * @if MOBILE
 * @brief Adds the context menu item to the context menu object.
 *
 * @since_tizen 2.3
 *
 * @param[in] menu The context menu object
 * @param[in] tag The tag of context menu item
 * @param[in] title The title of context menu item
 * @param[in] enabled If @c true the context menu item is enabled,\n
 *                    otherwise @c false
 *
 * @return @c EINA_TRUE on successful request,\n
 *         otherwise @c EINA_FALSE on failure
 * @endif
 */
EXPORT_API Eina_Bool ewk_context_menu_item_append_as_action(
    Ewk_Context_Menu* menu, Ewk_Context_Menu_Item_Tag tag, const char* title,
    Eina_Bool enabled);

/**
 * @if MOBILE
 * @brief Adds the context menu item to the context menu object.
 *
 * @since_tizen 2.3
 *
 * @param[in] menu The context menu object
 * @param[in] tag The tag of context menu item
 * @param[in] title The title of context menu item
 * @param[in] icon_file The path of icon to be set on context menu item
 * @param[in] enabled If @c true the context menu item is enabled,\n
 *                    otherwise @c false
 *
 * @return @c EINA_TRUE on successful request,\n
 *         otherwise @c EINA_FALSE on failure
 * @endif
 */
EXPORT_API Eina_Bool ewk_context_menu_item_append(Ewk_Context_Menu* menu,
                                                  Ewk_Context_Menu_Item_Tag tag,
                                                  const char* title,
                                                  const char* icon_file,
                                                  Eina_Bool enabled);

/**
 * @brief Returns the tag of context menu item.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] item The context menu item object
 *
 * @return The tag of context menu item
 */
EXPORT_API Ewk_Context_Menu_Item_Tag
ewk_context_menu_item_tag_get(Ewk_Context_Menu_Item* item);

/**
 * @if MOBILE
 * @brief Returns the link url string of context menu item.
 *
 * @since_tizen 2.3
 *
 * @param[in] item The context menu item object
 *
 * @return The current link url string on success,\n
 *         otherwise @c 0 on failure
 * @endif
 */
EXPORT_API const char* ewk_context_menu_item_link_url_get(
    Ewk_Context_Menu_Item* item);

/**
 * @if MOBILE
 * @brief Returns the image url string of context menu item.
 *
 * @since_tizen 2.3
 *
 * @param[in] item The context menu item object
 *
 * @return The current image url string on success,\n
 *         otherwise @c 0 on failure
 * @endif
 */
EXPORT_API const char* ewk_context_menu_item_image_url_get(
    Ewk_Context_Menu_Item* item);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_context_menu_h
