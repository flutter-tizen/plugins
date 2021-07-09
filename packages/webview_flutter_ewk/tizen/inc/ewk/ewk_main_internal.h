/*
 * Copyright (C) 2016 Samsung Electronics. All rights reserved.
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
 * @file    ewk_main_internal.h
 * @brief   The general initialization of Chromium-efl,
 *          not tied to any view object.
 */

#ifndef ewk_main_internal_h
#define ewk_main_internal_h

#include "ewk_main.h"

#ifdef __cplusplus
extern "C" {
#endif

enum Ewk_Process_Model {
  EWK_PROCESS_MODEL_MULTI,
  EWK_PROCESS_MODEL_MULTI_WITH_SINGLE_RENDERER,
  EWK_PROCESS_MODEL_SINGLE,
};

/**
 * @brief Enumeration that creates a type name for the #Ewk_Process_Model.
 * @since_tizen 6.0
 */
typedef enum Ewk_Process_Model Ewk_Process_Model;

/**
 * @brief Decide which process model to use to launch Chromium
 *
 * Must be called before the following APIs:\n
 * ewk_view_add\n
 * ewk_view_add_in_incognito_mode\n
 * ewk_view_add_with_context\n
 * ewk_view_add_with_session_data\n
 * ewk_context_default_get\n
 * ewk_context_new\n
 * ewk_context_new_with_injected_bundle_path
 *
 * @details Choose process model from single process model,
 *          multi process model and multi process model with single renderer
 * process.
 *
 * @since_tizen 6.0
 */
EXPORT_API void ewk_process_model_set(Ewk_Process_Model process_model);

/**
 * @brief Enable memory optimization mode.
 *
 * Must be called before the following APIs:\n
 * ewk_view_add\n
 * ewk_view_add_in_incognito_mode\n
 * ewk_view_add_with_context\n
 * ewk_view_add_with_session_data\n
 * ewk_context_default_get\n
 * ewk_context_new\n
 * ewk_context_new_with_injected_bundle_path
 *
 * @details Memory optimization mode decreases memory usage especially graphic
 * related. But, it could cause performance degradations.
 *
 * @since_tizen 6.0
 */
EXPORT_API void ewk_memory_optimization_mode_enable(void);

/**
 * Set argument count and argument vector.
 *
 * Must be called before the following APIs:\n
 * ewk_view_add\n
 * ewk_view_add_in_incognito_mode\n
 * ewk_view_add_with_context\n
 * ewk_view_add_with_session_data\n
 * ewk_context_default_get\n
 * ewk_context_new\n
 * ewk_context_new_with_injected_bundle_path
 *
 * This API allows passing application arguments to the engine.
 * Also there is possible to add custom parameters. However,
 * the engine expects *original* application arguments
 * to remain unchanged. Passing copy of them using strdup and friends
 * is prohibited. The recommended way to call this API is:
 *
 * int main(int argc, char* argv[]) {
 *   ewk_set_arguments(argc, argv);
 *   ...
 * }
 *
 * or
 *
 * int main(int argc, char* argv[]) {
 *     char* arg_options[] = {
 *       argv[0],
 *       "my_custom_param_1",
 *       "my_custom_param_2",
 *     };
 *     int arg_cnt = sizeof(arg_options) / sizeof(arg_options[0]);
 *     ewk_set_arguments(arg_cnt, arg_options);
 * }
 *
 * @note Calling the function for the second time has no user-visible effect.
 *
 * @param argc argument count
 * @param argv argument array
 */
EXPORT_API void ewk_set_arguments(int argc, char** argv);

/**
 * Deprecated.
 * Set home directory.
 *
 * If new path is NULL or empty string, home directory is considered as not set.
 *
 */
EINA_DEPRECATED EXPORT_API void ewk_home_directory_set(const char* path);

/**
 * @brief Set version selection policy
 *
 * @details Set a version selection policy when plural web engines are
 *          installed. If the runtime prefers preload version, set 0.
 *          If the runtime prefers updated version, set 1.
 *          Each product may have a different default policy so this function
 *          provides a way to set a definitive policy to each runtime.
 *
 * @since_tizen 5.5
 *
 * @note Must be called before ewk_init. Calling the function after ewk_init has
 *       no effect.
 *
 * @param[in] preference 0 means conservative. 1 means progressive.
 *
 * @return current policy
 *
 * @see ewk_init
 */
EXPORT_API int ewk_set_version_policy(int preference);

/**
 * @brief Set timeout for wating chromium mount.
 *
 * @details It waits up to timeout for chromium mount event
 *          by calling LwipcWaitEvent api and returns immediately
 *          when chromium mount is completed
 *
 * @since_tizen 5.5
 *
 * @param[in] timeout_msec timeout value (in milliseconds)
 *
 * @return @c true if chromium mount is done, otherwise @c false
 *
 */
EXPORT_API Eina_Bool ewk_wait_chromium_ready(unsigned int timeout_msec);

/**
 * @brief Check if the chromium mount is done.
 *
 * @details Check if the chromium mount is complete by calling LwipcIsDone.
 *
 * @since_tizen 5.5
 *
 * @return @c true if chromium mount is done, otherwise @c false
 *
 */
EXPORT_API Eina_Bool ewk_check_chromium_ready();

#ifdef __cplusplus
}
#endif
#endif  // ewk_main_internal_h
