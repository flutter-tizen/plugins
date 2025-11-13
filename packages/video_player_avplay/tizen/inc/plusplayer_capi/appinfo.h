/**
 * @file
 * @brief          AppInfo related enums
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style display releted data structures
 *                 and enums.
 * @see            The display related enum values and data structures will be
 *                 converted by this managed C version types to avoid binary
 *                 compatibility.
 *
 * Copyright (c) 2025 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_APPINFO_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_APPINFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Player app information.
 */
typedef struct {
  char* id;         /**< App id */
  char* version;    /**< App version */
  char* type;       /**< App type. ex)"MSE", "HTML5", etc.. */
  char* extra_data; /**< Extra field for future use */
} plusplayer_app_info_s;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_APPINFO_H__
