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
 * @file    ewk_certificate_internal.h
 * @brief   This file describes the engine APIs which allow deciding
 *          whether pages with certificate compromise should be opened or not.
 */

#ifndef ewk_certificate_internal_h
#define ewk_certificate_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _Ewk_Certificate_Policy_Decision_Error {
  EWK_CERTIFICATE_POLICY_DECISION_ERROR_COMMON_NAME_INVALID, /**< The server
      responded with a certificate whose common name did not match the host
      name. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_DATE_INVALID, /**< The server responded
     with a certificate that by our clock, appears to either not yet be valid or
     to have expired. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_AUTHORITY_INVALID, /**< The server
     responded with a certificate that is signed by an authority we don't
     trust. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_NO_REVOCATION_MECHANISM, /**<
     The certificate has no mechanism for determining if it is revoked. In
     effect, this certificate cannot be revoked. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_UNABLE_TO_CHECK_REVOCATION, /**<
     Revocation information for the security certificate for this site is not
     available. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_REVOKED, /**< The server responded with
      a certificate that has been revoked. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_INVALID, /**< The server responded with
      a certificate that is invalid. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_WEAK_ALGORITHM, /**< The server
      responded with a certificate that is signed using a weak signature
      algorithm. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_NON_UNIQUE_NAME, /**< The host name
      specified in the certificate is not unique. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_WEAK_KEY, /**< The server responded with
      a certificate that contains a weak key (e.g. a too-small RSA key). */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_PINNED_KEY_NOT_IN_CHAIN, /**<
      The certificate didn't match the public key pins for the host name. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_NAME_VIOLATION, /**< The certificate
      claimed DNS names that are in violation of name constraints. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_VALIDITY_TOO_LONG, /**<
      The certificate's validity period is too long. */

  EWK_CERTIFICATE_POLICY_DECISION_ERROR_UNKNOWN = 1000 /**< Reported in case of
      wrong usage of ewk_certificate_policy_decision_error_get API. */
} Ewk_Certificate_Policy_Decision_Error;

typedef struct _Ewk_Certificate_Policy_Decision Ewk_Certificate_Policy_Decision;

/**
 * Set the variable to allow the site access about certificate error.
 * After this function call Ewk_Certificate_Policy_Decision object becomes
 * invalid.
 *
 * @param certificate_policy_decision certificate information data
 *
 * @param allowed decided permission value from user
 */
EXPORT_API void ewk_certificate_policy_decision_allowed_set(
    Ewk_Certificate_Policy_Decision* certificate_policy_decision,
    Eina_Bool allowed);

/**
 * Suspend the operation for certificate error policy decision.
 *
 * This suspends the operation for certificate error policy decision when the
 * signal for policy is emitted. This is very usefull to decide the policy from
 * the additional UI operation like the popup. After calling this API the client
 * must call ewk_certificate_policy_decision_allowed_set in order to finalize
 * the certificate handling. Otherwise the memory leak will occur.
 *
 * @param certificate_policy_decision certificate information data
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_certificate_policy_decision_suspend(
    Ewk_Certificate_Policy_Decision* certificate_policy_decision);

/**
 * Get the variable url to check the site's url data about certificate error.
 *
 * @param certificate_policy_decision certificate information data
 *
 * @return @c url string on success or empty string on failure. The string
 * is only valid until related Ewk_Certificate_Policy_Decision object is valid.
 */
EXPORT_API Eina_Stringshare* ewk_certificate_policy_decision_url_get(
    Ewk_Certificate_Policy_Decision* certificate_policy_decision);

/**
 * Get the variable certificate pem data to check the information about
 * certificate error.
 *
 * @param certificate_policy_decision certificate information data
 *
 * @return @c certificate pem string on success or empty string on failure.
 * The string is only valid until related Ewk_Certificate_Policy_Decision
 * object is valid.
 */
EXPORT_API Eina_Stringshare*
ewk_certificate_policy_decision_certificate_pem_get(
    Ewk_Certificate_Policy_Decision* certificate_policy_decision);

/**
 * Get the error of the certificate.
 *
 * @param certificate_policy_decision certificate information data
 *
 * @return @c error number on success or
 * EWK_CERTIFICATE_POLICY_DECISION_ERROR_UNKNOWN on failure
 */
EXPORT_API Ewk_Certificate_Policy_Decision_Error
ewk_certificate_policy_decision_error_get(
    Ewk_Certificate_Policy_Decision* certificate_policy_decision);

/**
 * @brief The structure type that hold certificate's information
 */
typedef struct _Ewk_Certificate_Info Ewk_Certificate_Info;

/**
 * @brief Query certificate's PEM data
 *
 * @param[in] cert_info Certificate's information
 *
 * @return A certificate itself in the PEM format. It may be null what indicates
 * that webpage doesn't use the SSL protocol (e.g. HTTP).
 */
EXPORT_API const char* ewk_certificate_info_pem_get(
    const Ewk_Certificate_Info* cert_info);

/**
 * @brief Query if the context loaded with a given certificate is secure
 *
 * @details Even that webpage was successfully loaded with a given certificate,
 * its context may not be secure. Secure context means that webpage is fully
 * authenticated (using SSL certificates) and its content doesn't contain any
 * insecure elements (like HTTP CSS, images, scripts etc.)
 *
 * @param[in] cert_info Certificate's information
 *
 * @return EINA_TRUE if the context is secure. Otherwise returns EINA_FALSE
 */
EXPORT_API Eina_Bool
ewk_certificate_info_is_context_secure(const Ewk_Certificate_Info* cert_info);

/**
 * Returns information whether the certificate compromise comes from main frame.
 *
 * Certificate issue can be associated with main frame or sub resource
 * such as image, script, font etc. Browsers usually notify the user about
 * certificate compromise if it comes from main frame, whereas all
 * sub resource are silently blocked, since the user does not really
 * have a context for making the right decision.
 *
 * @certificate_policy_decision certificate information data
 *
 * @return @c EINA_TRUE if the certificate compromise comes from main frame,
 * @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_certificate_policy_decision_from_main_frame_get(
    const Ewk_Certificate_Policy_Decision* certificate_policy_decision);

#ifdef __cplusplus
}
#endif
#endif
