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
 * @file    ewk_console_message_internal.h
 * @brief   Describes the Console Message API.
 */

#ifndef ewk_console_message_internal_h
#define ewk_console_message_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Creates a type name for @a _Ewk_Console_Message. */
typedef struct _Ewk_Console_Message Ewk_Console_Message;

/// Creates a type name for Ewk_Console_Message_Level.
typedef enum {
  EWK_CONSOLE_MESSAGE_LEVEL_NULL,
  EWK_CONSOLE_MESSAGE_LEVEL_LOG,
  EWK_CONSOLE_MESSAGE_LEVEL_WARNING,
  EWK_CONSOLE_MESSAGE_LEVEL_ERROR,
  EWK_CONSOLE_MESSAGE_LEVEL_DEBUG,
  EWK_CONSOLE_MESSAGE_LEVEL_INFO,
} Ewk_Console_Message_Level;

/**
 * Returns the log severity of the console message from the Console Message
 object
 *
 * @param message console message object
 *
 * @return Ewk_Console_Message_Level indicating the console message level,
           LogMessageLevel = 1,
           WarningMessageLevel = 2,
           ErrorMessageLevel = 3,
           DebugMessageLevel = 4,
           InfoMessageLevel = 5
 */
EXPORT_API Ewk_Console_Message_Level
ewk_console_message_level_get(const Ewk_Console_Message *message);

/**
 * Returns the console message text from the Console Message object
 *
 * @param message console message object
 *
 * @return console message text on success or empty string on failure
 */
EXPORT_API Eina_Stringshare *ewk_console_message_text_get(
    const Ewk_Console_Message *message);

/**
 * Returns line no of the console message from the Console Message object
 *
 * @param message console message object
 *
 * @return the line number of the message on success or 0 on failure
 */
EXPORT_API unsigned ewk_console_message_line_get(
    const Ewk_Console_Message *message);

/**
 * Returns the source of the console message from the Console Message object
 *
 * @param message console message object
 *
 * @return source of the console message on success or empty string on failure
 */
EXPORT_API Eina_Stringshare *ewk_console_message_source_get(
    const Ewk_Console_Message *message);

#ifdef __cplusplus
}
#endif

#endif  // ewk_console_message_internal_h
