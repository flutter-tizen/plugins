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

#ifndef ewk_policy_decision_internal_h
#define ewk_policy_decision_internal_h

#include "ewk_frame_internal.h"
#include "ewk_policy_decision.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns user id for Authorization from Policy Decision object.
 *
 * @param policy_decision policy decision object
 *
 * @return @c user id string on success or empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_userid_get(
    const Ewk_Policy_Decision* policy_decision);

/**
 * Returns password for Authorization from Policy Decision object.
 *
 * @param policy_decision policy decision object
 *
 * @return @c password string on success or empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_password_get(
    const Ewk_Policy_Decision* policy_decision);

/**
 * Suspend the operation for policy decision.
 *
 * This suspends the operation for policy decision when the signal for policy is
 * emitted. This is very useful to decide the policy from the additional UI
 * operation like the popup.
 *
 * @param policy_decision policy decision object
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_policy_decision_suspend(Ewk_Policy_Decision* policy_decision);

/**
 * Cause a download from this decision.
 *
 * @param policy_decision policy decision object
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_policy_decision_download(Ewk_Policy_Decision* policy_decision);

/**
 * Gets the frame reference from Policy Decision object.
 *
 * @param policy_decision policy decision object
 *
 * @return frame reference on success, or NULL on failure
 */
EXPORT_API Ewk_Frame_Ref
ewk_policy_decision_frame_get(Ewk_Policy_Decision* policy_decision);

/**
 * Checks if frame requested in policy decision is main frame.
 *
 * @param policy_decision policy decision object
 *
 * @return @c EINA_TRUE or @c EINA_FALSE
 */
EXPORT_API Eina_Bool
ewk_policy_decision_is_main_frame(const Ewk_Policy_Decision* policy_decision);

#ifdef __cplusplus
}
#endif
#endif  // ewk_policy_decision_internal_h
