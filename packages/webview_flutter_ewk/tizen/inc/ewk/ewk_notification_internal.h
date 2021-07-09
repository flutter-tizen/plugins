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

#ifndef ewk_notification_internal_h
#define ewk_notification_internal_h

#include <Eina.h>
#include <Evas.h>
#include <tizen.h>

#include "ewk_security_origin_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _Ewk_Notification_Permission {
  const char* origin;
  Eina_Bool allowed;
};

typedef struct Ewk_Notification Ewk_Notification;
typedef struct _Ewk_Notification_Permission Ewk_Notification_Permission;
typedef struct Ewk_Notification_Permission_Request
    Ewk_Notification_Permission_Request;
typedef void (*Ewk_Notification_Show_Callback)(Ewk_Notification*, void*);
typedef void (*Ewk_Notification_Cancel_Callback)(uint64_t, void*);

/**
 * Sets callbacks for notifications handling.
 *
 * These callbacks will receive all notifications from all webview instances.
 * @e show_callback is used to display notification UI. Embeder should call
 * ewk_notification_showed after it displays notification UI. @e cancel_callback
 * will be called after notification is closed by engine. It's used to notify
 * embeder that related notification UI should be destroyed.
 *
 * @param show_callback notification show callback. Can't be NULL
 * @param cancel_callback notification close callback. Can't be NULL
 * @param user_data extra data for callback
 *
 * @return EINA_TRUE on success, EINA_FALSE on failure (i.e. when ewk_init was
 * not called)
 */
EXPORT_API Eina_Bool ewk_notification_callbacks_set(
    Ewk_Notification_Show_Callback show_callback,
    Ewk_Notification_Cancel_Callback cancel_callback, void* user_data);

/**
 * Resets notification callbacks to NULL
 *
 * @return EINA_TRUE on success, EINA_FALSE on failure (i.e. when ewk_init was
 * not called)
 */
EXPORT_API Eina_Bool ewk_notification_callbacks_reset();

/**
 * Get notification icon as Evas_Object
 *
 * @param ewk_notification pointer to notification data
 * @param evas canvas where icon object will be added
 *
 * @return Evas_Object containing icon if successful, @c NULL otherwise.
 *         Caller takes ownership of returned Evas_Object.
 */
EXPORT_API Evas_Object* ewk_notification_icon_get(
    const Ewk_Notification* ewk_notification, Evas* evas);

/**
 * Save notification icon as PNG image
 *
 * @param ewk_notification notification data pointer
 * @param path path where file will be saved
 *
 * @return EINA_TRUE on success
 */
EXPORT_API Eina_Bool ewk_notification_icon_save_as_png(
    const Ewk_Notification* ewk_notification, const char* path);

/**
 * Requests for getting body of notification.
 *
 * @param ewk_notification pointer of notificaion data
 *
 * @return body of notification
 *         Lifetime only valid as long as @a ewk_notification is valid.
 */
EXPORT_API const char* ewk_notification_body_get(
    const Ewk_Notification* ewk_notification);

/**
 * Notify that notification is clicked.
 *
 * @param notification_id identifier of notification
 *
 * @return EINA_TRUE on success, EINA_FALSE if notification id is invalid
 */
EXPORT_API Eina_Bool ewk_notification_clicked(uint64_t notification_id);

/**
 * Requests for getting icon url of notification.
 *
 * @param ewk_notification pointer of notification data
 *
 * @return Always returns NULL - this API is deprecated.
 *
 * @deprecated
 */
EINA_DEPRECATED EXPORT_API const char* ewk_notification_icon_url_get(
    const Ewk_Notification* ewk_notification);

/**
 * Requests for getting id of notification.
 *
 * @param ewk_notification pointer of notification data
 *
 * @return id of notification
 */
EXPORT_API uint64_t
ewk_notification_id_get(const Ewk_Notification* ewk_notification);

/**
 * Requests for setting cached notification permissions.
 *
 * By calling this notification permission is replaced as passed
 * ewk_notification_permissions.
 *
 * @param context context object
 * @param ewk_notification_permissions list of cached
 * permissions(Ewk_Notification_Permission)
 *
 * @return EINA_TRUE if successful, EINA_FALSE if ewk_init was not called
 */
EXPORT_API Eina_Bool ewk_notification_cached_permissions_set(
    Eina_List* ewk_notification_permissions);

/**
 * Requests for getting origin of notification permission request.
 *
 * @param request Ewk_Notification_Permission_Request object to get origin for
 * notification permission request
 *
 * @return security origin of notification permission request
 *         Lifetime only valid as long as @a ewk_notification is valid.
 */
EXPORT_API const Ewk_Security_Origin*
ewk_notification_permission_request_origin_get(
    const Ewk_Notification_Permission_Request* request);

/**
 * Reply the result about notification permission.
 *
 * @param request Ewk_Notification_Permission_Request object to get the
 *                infomation about notification permission request.
 * @param allow result about notification permission
 *
 * @return EINA_TRUE is successful. EINA_FALSE if reply was already called for
 *         this request or if request is NULL
 */
EXPORT_API Eina_Bool ewk_notification_permission_reply(
    Ewk_Notification_Permission_Request* request, Eina_Bool allow);

/**
 * Deprecated, use ewk_notification_permission_reply instead.
 * Sets permission of notification.
 *
 * @param request Ewk_Notification_Permission_Request object to allow/deny
 * notification permission request is freed in this function.
 * @param allowed @c EINA_TRUE if permission is allowed, @c EINA_FALSE if
 * permission is denied
 *
 * @return EINA_TRUE is successful. EINA_FALSE if reply was already called for
 *         this request or if request is NULL
 *
 * @deprecated
 * @see ewk_notification_permission_reply
 */
EINA_DEPRECATED EXPORT_API Eina_Bool ewk_notification_permission_request_set(
    Ewk_Notification_Permission_Request* request, Eina_Bool allowed);

/**
 * Suspend the operation for permission request.
 *
 * This suspends the operation for permission request.
 * This is very useful to decide the policy from the additional UI operation
 * like the popup.
 *
 * @param request Ewk_Notification_Permission_Request object to suspend
 * notification permission request
 */
EXPORT_API Eina_Bool ewk_notification_permission_request_suspend(
    Ewk_Notification_Permission_Request* request);

/**
 * Notify that notification policies are removed.
 *
 * @param context context object
 * @param origins list of security origins(made by UAs)
 */
EXPORT_API Eina_Bool ewk_notification_policies_removed(Eina_List* origins);

/**
 * Requests for getting security origin of notification.
 *
 * @param ewk_notification pointer of notification data
 *
 * @return security origin of notification
 *         Lifetime only valid as long as @a ewk_notification is valid.
 */
EXPORT_API const Ewk_Security_Origin* ewk_notification_security_origin_get(
    const Ewk_Notification* ewk_notification);

/**
 * Notify that notification is showed.
 *
 * @param notification_id identifier of notification
 */
EXPORT_API Eina_Bool ewk_notification_showed(uint64_t notification_id);

/**
 * Notify that notification was closed.
 *
 * @param notification_id identifier of notification
 * @param by_user         informs whether notification was closed by user
 *                        action or by some other application logic
 */
EXPORT_API Eina_Bool ewk_notification_closed(uint64_t notification_id,
                                             Eina_Bool by_user);

/**
 * Requests for getting title of notification.
 *
 * @param ewk_notification pointer of notification data
 *
 * @return title of notification
 *         Lifetime only valid as long as @a ewk_notification is valid.
 */
EXPORT_API const char* ewk_notification_title_get(
    const Ewk_Notification* ewk_notification);

/**
 * Query if the notification is silent
 *
 * @param ewk_notification pointer of notification data
 *
 * @return @c EINA_TRUE if the notification must be silent, @c EINA_FALSE
 *         otherwise
 */
EXPORT_API Eina_Bool
ewk_notification_silent_get(const Ewk_Notification* ewk_notification);

#ifdef __cplusplus
}
#endif
#endif  // ewk_notification_internal.h
