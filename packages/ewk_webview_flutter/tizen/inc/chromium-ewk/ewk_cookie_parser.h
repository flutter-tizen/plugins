/**
 * @file  ewk_cookie_parser
 * @brief EWK Cookie Parser
 *
 * This class exposes the Chromium cookie parser. It allows
 * for ewk components to handle cookie-alike structures without
 * re-inventing the wheel.
 *
 * Copyright 2020 by Samsung Electronics, Inc.,
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung.
 */

#ifndef __EWK_COOKIE_PARSER_H__
#define __EWK_COOKIE_PARSER_H__

#include <Ecore.h>
#include <tizen.h>

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 */

/**
 * @brief This structure is used to return the contents of the parsed cookie.
 * This structure is used a bridge between the Chromium cookie handling code
 * and the calling application.
 */
struct EWKCookieContents {
  time_t expiry_date_utc;
  std::string domain;
  std::string name;
  std::string path;
  std::string value;
};

/**
 * @brief Parses a Cookie String and Returns the Contents.
 *
 * @details The application can use this function use the internal Cookie
 *          parsing code to decode cookie strings. This allows the calling
 *          application to manage non-standard cookies without effecting the
 *          security policies that have been set up in Chromium. These can be
 *          used for non-standard schemes the may want to use cookies.
 *
 *          The cookie is only treated as valid if it should be readably by a
 *          javascript function. I.e. if the HttpOnly and Secure flags are not
 *          set. It is the responsibility of the calling application to check to
 *          see if the cookies expiry_date has expired.
 *
 *          NOTE: if the cookie has an expiry_date == 0 then the cookie should
 *                be treated as a session cookie.
 *
 * @since_tizen 4.0
 *
 * @param[in]   cookie_str  The cookie string to be decoded.
 * @param[out]  cookie      The decoded cookie data.
 *
 * @return @c true if the cookie string is value, else false.
 */
EXPORT_API Eina_Bool ewk_parse_cookie(const std::string& cookie_str,
                                      EWKCookieContents& cookie);

#ifdef __cplusplus
}
#endif

#endif
