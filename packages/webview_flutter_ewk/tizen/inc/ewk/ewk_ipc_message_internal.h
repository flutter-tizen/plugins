/*
 * Copyright (C) 2014-2016 Samsung Electronics. All rights reserved.
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
 * @file    ewk_ipc_message_internal.h
 * @brief   Custom support for ipc messages
 */

#ifndef ewk_ipc_message_internal_h
#define ewk_ipc_message_internal_h

#include <Eina.h>
#include <tizen.h>

#include "ewk_context_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Ewk_Wrt_Message_Data Ewk_IPC_Wrt_Message_Data;

/*
 * Create new Ewk_Wrt_Message_Data object. It has to freed by the caller
 * using ewk_ipc_wrt_message_data_del()
 */
EXPORT_API Ewk_IPC_Wrt_Message_Data *ewk_ipc_wrt_message_data_new();

/*
 * Delete the Ewk_Wrt_Message_Object passed by the caller.
 */
EXPORT_API void ewk_ipc_wrt_message_data_del(Ewk_IPC_Wrt_Message_Data *data);

/*
 * Set message type
 *
 * @return false if NULL string was provided, true otherwise
 */
EXPORT_API Eina_Bool ewk_ipc_wrt_message_data_type_set(
    Ewk_IPC_Wrt_Message_Data *data, const char *type);

/*
 * Get the type of given message. The string should be released with
 * eina_stringshare_del()
 *
 * @return true on success, false in case provided data structure is NULL
 */
EXPORT_API Eina_Stringshare *ewk_ipc_wrt_message_data_type_get(
    const Ewk_IPC_Wrt_Message_Data *data);

/*
 * Set message value
 *
 * @return false if NULL string was provided, true otherwise
 */
EXPORT_API Eina_Bool ewk_ipc_wrt_message_data_value_set(
    Ewk_IPC_Wrt_Message_Data *data, const char *value);

/*
 * Get message value. The string should be released with eina_stringshare_del()
 *
 * @return true on success, false in case provided data structure is NULL
 */
EXPORT_API Eina_Stringshare *ewk_ipc_wrt_message_data_value_get(
    const Ewk_IPC_Wrt_Message_Data *data);

/*
 * Set message ID
 *
 * @return false if NULL string was provided, true otherwise
 */
EXPORT_API Eina_Bool
ewk_ipc_wrt_message_data_id_set(Ewk_IPC_Wrt_Message_Data *data, const char *id);

/*
 * Get message ID. The string should be released with eina_stringshare_del()
 *
 * @return true on success, false in case provided data structure is NULL
 */
EXPORT_API Eina_Stringshare *ewk_ipc_wrt_message_data_id_get(
    const Ewk_IPC_Wrt_Message_Data *data);

/*
 * Get message reference ID
 *
 * @return false if NULL string was provided, true otherwise
 */
EXPORT_API Eina_Bool ewk_ipc_wrt_message_data_reference_id_set(
    Ewk_IPC_Wrt_Message_Data *data, const char *reference_id);

/*
 * Set message reference ID. The string should be released with
 * eina_stringshare_del()
 *
 * @return true on success, false in case provided data structure is NULL
 */
EXPORT_API Eina_Stringshare *ewk_ipc_wrt_message_data_reference_id_get(
    const Ewk_IPC_Wrt_Message_Data *data);

/**
 *  Send IPC message to Wrt
 *
 *  Plugins -> Wrt (Renderer->Browser)
 */
EXPORT_API Eina_Bool ewk_ipc_plugins_message_send(
    int routingId, const Ewk_IPC_Wrt_Message_Data *data);

/**
 *   Send IPC message to Plugins
 *
 *   Wrt -> Plugins (Browser->Renderer)
 *   //TODO - it is always send to all rendereres - it need to be modified
 */
EXPORT_API Eina_Bool ewk_ipc_wrt_message_send(
    Ewk_Context *context, const Ewk_IPC_Wrt_Message_Data *data);

/**
 *  Send Synchronous IPC message to Wrt
 *
 *  Plugins -> Wrt (answer: -> Plugins) (Renderer->Browser -> Renderer)
 *
 *  If success the value member will be set by the handler
 */
EXPORT_API Eina_Bool ewk_ipc_plugins_sync_message_send(
    int routingId, Ewk_IPC_Wrt_Message_Data *data);

#ifdef __cplusplus
}
#endif

#endif  // ewk_ipc_message_internal_h
