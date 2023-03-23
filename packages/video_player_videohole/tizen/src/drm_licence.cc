// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_licence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "curl/curl.h"
#include "log.h"

#define DEFAULT_USER_AGENT_PLAYREADY "User-Agent: PlayReadyClient"
#define DEFAULT_USER_AGENT_WIDEVINE "User-Agent: Widevine CDM v1.0"
#define HTTP_HEADER_PLAYREADY_LICGET                      \
  "Content-Type: text/xml; charset=utf-8\r\nSOAPAction: " \
  "\"http://schemas.microsoft.com/DRM/2007/03/protocols/AcquireLicense\""
#define HTTP_HEADER_WIDEVINE_LICGET "Content-Type: application/octet-stream"

#define INFO(...) LOG_INFO(__VA_ARGS__)
#define INFO_CURL_HEADERS(headers)                 \
  {                                                \
    INFO("REQ Headers: BEGIN");                    \
    struct curl_slist* p;                          \
    for (p = headers; p != nullptr; p = p->next) { \
      INFO("%s", p->data);                         \
    }                                              \
    INFO("REQ Headers: END");                      \
  }

#define MAX_POPUP_MESSAGE_LENGTH (1024)
#define ERROR_TITLE_LICENSE_FETCHING_FAILURE "Fetching License Failed"

namespace {
struct SDynamicBuf {
  unsigned char* data;
  size_t size;
  size_t allocated;
};

struct SHttpSession {
  void* curl_handle;
  unsigned char* post_data;  // request body
  size_t post_data_len;      // length of request body
  DrmLicenseHelper::DrmType type;
  size_t send_data_len;  // length of send already
  SDynamicBuf header;    // response header
  SDynamicBuf body;      // response body
  long res_code;
};

// Internal Static Functions
static size_t ReceiveHeader(void* ptr, size_t size, size_t nmemb, void* stream);
static size_t ReceiveBody(void* ptr, size_t size, size_t nmemb, void* stream);
static size_t SendBody(void* ptr, size_t size, size_t nmemb, void* stream);
static bool AppendData(SDynamicBuf* append_buf, const void* append_data,
                       size_t append_size);
static char* GetRedirectLocation(const char* append_headers,
                                 bool support_https);
static struct curl_slist* CurlSlistAppend(struct curl_slist* append_list,
                                          const char* append_string);
static DRM_RESULT ComposePostDataTZ(SHttpSession* http_session,
                                    const char* post_data, int post_data_len,
                                    const char* soap_header);
static struct curl_slist* SetHttpHeader(CURL* http_curl,
                                        DrmLicenseHelper::DrmType type,
                                        const char* http_cookie,
                                        const char* http_header,
                                        const char* http_user_agent);
static SHttpSession* HttpOpen(void);
static int CbCurlProgress(void* ptr, double total_to_download,
                          double now_downloaded, double total_to_upload,
                          double now_uploaded);
static DRM_RESULT HttpStartTransaction(
    SHttpSession* http_session, const char* http_url, const void* post_data,
    unsigned post_data_len, DrmLicenseHelper::DrmType type,
    const char* http_cookie, const char* http_soap_header,
    const char* http_header, const char* http_user_agent,
    bool* http_cancel_request);
static void HttpClose(SHttpSession* http_session);
static void CheckCurlFail(CURLcode expr, struct curl_slist* headers);

static void CheckCurlFail(CURLcode expr, struct curl_slist* headers) {
  if (expr != CURLE_OK) {
    INFO("Error %d", __LINE__);
    if (headers != nullptr) {
      INFO_CURL_HEADERS(headers);
      curl_slist_free_all(headers);
    }
  }
}

bool AppendData(SDynamicBuf* append_buf, const void* append_data,
                size_t append_size) {
  size_t new_size = append_buf->size + append_size;
  if (append_buf->allocated < new_size) {
    new_size += 1024;
    unsigned char* buf =
        reinterpret_cast<unsigned char*>(realloc(append_buf->data, new_size));
    if (!buf) {
      LOG_ERROR("[DrmLicence] AppendData : realloc fail ");
      return false;
    }
    append_buf->data = buf;
    append_buf->allocated = new_size;
    LOG_DEBUG(
        "[DrmLicence] AppendData : realloc append_size(%d), size(%d) "
        "append_buf->allocated(%d)",
        append_size, append_buf->size, append_buf->allocated);
  }
  memcpy(append_buf->data + append_buf->size, append_data, append_size);
  append_buf->size += append_size;

  return true;
}

char* GetRedirectLocation(const char* append_headers, bool support_https) {
  if (!append_headers) {
    return nullptr;
  }
  // Get the header's location value
  const char* location = strcasestr(append_headers, "Location");
  if (!location) {
    return nullptr;
  }
  const char* ptr = location + strlen("Location");

  while (*ptr == ':') {
    ptr++;
  }
  while (*ptr == ' ') {
    ptr++;
  }

  unsigned i = 0;
  while (ptr[i] && (ptr[i] != ' ') && (ptr[i] != '\n') && (ptr[i] != '\r')) {
    i++;
  }

  if (support_https) {
    // [soyoung] get redirection location
    // for using https itself
    char* ret = static_cast<char*>(malloc(i + 1));
    if (!ret) {
      return nullptr;
    }
    memcpy(ret, ptr, i);
    ret[i] = 0;
    return ret;
  } else {
    // Convert Redirection Location from https to http
    // [soyoung]
    // Redirect location from https to http
    // If the petition URL contains "https," the client may use SSL for the
    // connection. (For non-SSL transport, remove the "s" in "https" from the
    // URL.) If SSL is used, the client should check the server's certificate to
    // ensure it is current, matches the domain, and is properly signed by a
    // trusted authority.
    int len = i;
    const char* p = ptr + 4;
    const char http_str[6] = "http\0";
    if (i < 7) {
      return nullptr;  // wrong location, no space even for http://
    }

    if (strncasecmp(ptr, "https", 5) == 0) {
      len--;
      p++;
    }

    char* ret = static_cast<char*>(malloc(len + 1));
    if (!ret) {
      return nullptr;
    }

    memcpy(ret, http_str, 4);
    memcpy(ret + 4, p, len - 4);
    ret[len] = 0;
    return ret;
  }
}

struct curl_slist* CurlSlistAppend(struct curl_slist* append_list,
                                   const char* append_string) {
  if (!append_list) {
    return nullptr;
  }

  struct curl_slist* new_list = nullptr;
  new_list = curl_slist_append(append_list, append_string);
  if (!new_list) {
    curl_slist_free_all(append_list);
  }

  return new_list;
}

DRM_RESULT ComposePostDataTZ(SHttpSession* http_session, const char* post_data,
                             int post_data_len, const char* soap_header) {
  DRM_RESULT drm_result = DRM_SUCCESS;
  const char* pointer;
  char* dest;
  int dest_len;
  int remain;

  int soap_header_len = soap_header ? strlen(soap_header) : 0;
  dest_len = post_data_len;

  http_session->post_data =
      reinterpret_cast<unsigned char*>(malloc(dest_len + 1));
  if (http_session->post_data == nullptr) {
    LOG_ERROR("[DrmLicence] Failed to alloc post data.");
    return DRM_E_POINTER;
  }
  dest = reinterpret_cast<char*>(http_session->post_data);
  remain = post_data_len;

  if (soap_header_len > 0) {
    dest_len += soap_header_len + sizeof("<soap:Header>\r\n</soap:Header>\r");
    /* append to the last in an existing soap header */
    pointer = strstr(post_data, "</soap:Header>");
    if (pointer > post_data && pointer < post_data + remain) {
      int header_len = pointer - post_data;
      memcpy(dest, post_data, header_len);
      dest += header_len;
      dest_len -= header_len;
      remain -= header_len;

      memcpy(dest, soap_header, soap_header_len);
      dest += soap_header_len;
      if (*dest == '\0') {
        dest--;
      }
    } else {
      /* insert soap header in front of soap body */
      pointer = strstr(post_data, "<soap:Body>");
      if (pointer > post_data && pointer < post_data + remain) {
        int header_len = pointer - post_data;
        memcpy(dest, post_data, header_len);
        dest += header_len;
        dest_len -= header_len;
        remain -= header_len;
        *dest = '\0';
        strncat(dest, "<soap:Header>", dest_len);
        header_len = strlen(dest);
        dest += header_len;
        dest_len -= header_len;

        memcpy(dest, soap_header, soap_header_len);
        header_len = soap_header_len;
        dest += header_len;
        dest_len -= header_len;

        *dest = '\0';
        strncat(dest, "</soap:Header>", dest_len);
        header_len = strlen(dest);
        dest += header_len;
        dest_len -= header_len;
      } else {
        /* not a SOAP message */
        pointer = post_data;
      }
    }
  } else {
    pointer = post_data;
  }

  memcpy(dest, pointer, remain);
  dest += remain;
  *dest = '\0';

  http_session->post_data_len =
      dest - reinterpret_cast<char*>(http_session->post_data);
  if (soap_header_len > 0) {
    LOG_INFO("[DrmLicence] [soap header added %d ] %s ",
             http_session->post_data_len, http_session->post_data);
  }

  return drm_result;
}

struct curl_slist* SetHttpHeader(CURL* http_curl,
                                 DrmLicenseHelper::DrmType type,
                                 const char* http_cookie,
                                 const char* http_header,
                                 const char* http_user_agent) {
  const char* user_agent = nullptr;
  const char* header = nullptr;

  switch (type) {
    case DrmLicenseHelper::DRM_TYPE_PLAYREADY:
      user_agent = DEFAULT_USER_AGENT_PLAYREADY;
      header = HTTP_HEADER_PLAYREADY_LICGET;
      break;
    case DrmLicenseHelper::DRM_TYPE_WIDEVINE:
      user_agent = DEFAULT_USER_AGENT_WIDEVINE;
      header = HTTP_HEADER_WIDEVINE_LICGET;
      break;
    default:
      LOG_ERROR("[DrmLicence] Invalid DRM Type");
      return nullptr;
  }

  struct curl_slist* headers = nullptr;
  if (http_user_agent) {
    const char* user_agent_prefix = "User-Agent: ";
    unsigned prefix_len = strlen(user_agent_prefix);
    unsigned user_agent_len = strlen(http_user_agent);

    char* user_agent_string =
        static_cast<char*>(malloc(prefix_len + user_agent_len + 1));
    if (nullptr == user_agent_string) {
      LOG_ERROR("[DrmLicence] Memory allocation failed.");
      return nullptr;
    }

    memcpy(user_agent_string, user_agent_prefix, prefix_len);
    memcpy(user_agent_string + prefix_len, http_user_agent, user_agent_len);
    user_agent_string[prefix_len + user_agent_len] = 0;
    LOG_INFO(
        "[DrmLicence] SetHttpHeader :  user-agent added to header --- (%s)",
        user_agent_string);

    headers = curl_slist_append(nullptr, user_agent_string);
    free(user_agent_string);
  } else {
    headers = curl_slist_append(nullptr, user_agent);
  }

  if (nullptr == headers) {
    LOG_ERROR("[DrmLicence] UserAgent attach failed.");
    return nullptr;
  }

  LOG_DEBUG(
      "[DrmLicence] SetHttpHeader : type(%d), http_cookie(%s), "
      "http_header(%s)",
      type, http_cookie, http_header);

  headers = CurlSlistAppend(headers, header);

  if (http_cookie) {
    const char* cookie_prefix = "Cookie: ";
    unsigned prefix_len = strlen(cookie_prefix);
    unsigned cookie_len = strlen(http_cookie);

    char* cookie = static_cast<char*>(malloc(prefix_len + cookie_len + 1));

    if (cookie) {
      memcpy(cookie, cookie_prefix, prefix_len);
      memcpy(cookie + prefix_len, http_cookie, cookie_len);
      cookie[prefix_len + cookie_len] = '\0';

      headers = CurlSlistAppend(headers, cookie);

      LOG_INFO("[DrmLicence] SetHttpHeader :  cookie added to header --- (%s)",
               cookie);

      free(cookie);
    }
  }

  if (http_header) {
    LOG_INFO(
        "[DrmLicence] SetHttpHeader :  HttpHeader added to header --- (%s)",
        http_header);
    headers = CurlSlistAppend(headers, http_header);
  }

  if (headers) {
    curl_easy_setopt(http_curl, CURLOPT_HTTPHEADER, headers);
  }

  return headers;
}

static SHttpSession* HttpOpen(void) {
  SHttpSession* http_session = nullptr;

  CURL* http_curl = curl_easy_init();
  if (http_curl) {
    http_session =
        reinterpret_cast<SHttpSession*>(malloc(sizeof(SHttpSession)));
    if (http_session) {
      memset(http_session, 0, sizeof(SHttpSession));
      http_session->curl_handle = http_curl;
      return http_session;
    }
    curl_easy_cleanup(http_curl);
  }
  LOG_ERROR("[DrmLicence] Can't create CURL object, curl_global_init missed");
  return nullptr;
}

int CbCurlProgress(void* ptr, double total_to_download, double now_downloaded,
                   double total_to_upload, double now_uploaded) {
  bool* http_cancel_qequest = static_cast<bool*>(ptr);

  if (http_cancel_qequest) {
    LOG_INFO("[DrmLicence] http_cancel_qequest : (%d)", *http_cancel_qequest);

    if (*http_cancel_qequest) {
      LOG_INFO("[DrmLicence] %s:%d curl works canceled.", __FUNCTION__,
               __LINE__);
      return 1;
    }
  }

  return 0;
}

DRM_RESULT HttpStartTransaction(
    SHttpSession* http_session, const char* http_url, const void* post_data,
    unsigned post_data_len, DrmLicenseHelper::DrmType type,
    const char* http_cookie, const char* http_soap_header,
    const char* http_header, const char* http_user_agent,
    bool* http_cancel_request) {
  CURLcode res = CURLE_OK;
  struct curl_slist* headers = nullptr;
  CURL* http_curl = http_session->curl_handle;

  // 1. Set Post Data
  http_session->post_data_len = post_data_len;
  http_session->send_data_len = 0;
  http_session->body.size = 0;
  http_session->header.size = 0;

  LOG_INFO("[DrmLicence] HttpStartTransaction : type(%d)", type);
  if (http_url) {
    LOG_INFO("[DrmLicence] http_url : %s", http_url);
  }

  // 2. Set Header type
  http_session->type = type;
  headers =
      SetHttpHeader(http_curl, type, http_cookie, http_header, http_user_agent);
  if (!headers) {
    LOG_ERROR("[DrmLicence] Failed to set HTTP header.");
    return DRM_E_NETWORK_HEADER;
  }

  curl_easy_setopt(http_curl, CURLOPT_VERBOSE, 0L);

  // Check
  curl_easy_setopt(http_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

  int soap_flag = 0;
  free(http_session->post_data);
  http_session->post_data = nullptr;
  http_session->post_data_len = 0;
  if (post_data && post_data_len > 0) {
    if (http_soap_header != nullptr) {
      DRM_RESULT drm_result =
          ComposePostDataTZ(http_session, static_cast<const char*>(post_data),
                            post_data_len, http_soap_header);
      if (drm_result != DRM_SUCCESS) {
        LOG_ERROR(
            "[DrmLicence] Failed to compose post data, drm_result : 0x%lx",
            drm_result);
        return drm_result;
      } else if (drm_result == DRM_SUCCESS) {
        soap_flag = 1;
      }
    }

    res = curl_easy_setopt(http_curl, CURLOPT_POST, 1L);
    CheckCurlFail(res, headers);

    if (soap_flag == 0) {
      if (!(http_session->post_data =
                reinterpret_cast<unsigned char*>(malloc(post_data_len)))) {
        if (headers != nullptr) {
          curl_slist_free_all(headers);
        }
        LOG_ERROR("[DrmLicence] Failed to alloc post data.");
        return DRM_E_POINTER;
      }

      if (http_session->post_data) {
        memcpy(http_session->post_data, post_data, post_data_len);
        http_session->post_data_len = post_data_len;
      }
    }

    res = curl_easy_setopt(http_curl, CURLOPT_READFUNCTION, SendBody);
    CheckCurlFail(res, headers);

    res = curl_easy_setopt(http_curl, CURLOPT_POSTFIELDSIZE,
                           http_session->post_data_len);
    CheckCurlFail(res, headers);

    res = curl_easy_setopt(http_curl, CURLOPT_READDATA, http_session);
    CheckCurlFail(res, headers);
  } else {
    curl_easy_setopt(http_curl, CURLOPT_HTTPGET, 1L);
  }

  curl_easy_setopt(http_curl, CURLOPT_USE_SSL, 1L);
  curl_easy_setopt(http_curl, CURLOPT_SSL_VERIFYPEER, 1L);  // 0L
  curl_easy_setopt(http_curl, CURLOPT_SSL_VERIFYHOST, 2L);  // 0L

  // set timeout 10 seconds
  curl_easy_setopt(http_curl, CURLOPT_TIMEOUT, 10);

  res = curl_easy_setopt(http_curl, CURLOPT_URL, http_url);
  CheckCurlFail(res, headers);

  res = curl_easy_setopt(http_curl, CURLOPT_NOPROGRESS, 0L);
  CheckCurlFail(res, headers);
  res = curl_easy_setopt(http_curl, CURLOPT_PROGRESSFUNCTION, CbCurlProgress);
  CheckCurlFail(res, headers);
  res = curl_easy_setopt(http_curl, CURLOPT_PROGRESSDATA, http_cancel_request);
  CheckCurlFail(res, headers);

  res = curl_easy_setopt(http_curl, CURLOPT_HEADERFUNCTION, ReceiveHeader);
  CheckCurlFail(res, headers);

  res = curl_easy_setopt(http_curl, CURLOPT_BUFFERSIZE, 1024L * 20L);
  CheckCurlFail(res, headers);

  res = curl_easy_setopt(http_curl, CURLOPT_WRITEFUNCTION, ReceiveBody);
  CheckCurlFail(res, headers);

  res = curl_easy_setopt(http_curl, CURLOPT_WRITEHEADER, http_session);
  CheckCurlFail(res, headers);

  res = curl_easy_setopt(http_curl, CURLOPT_WRITEDATA, http_session);
  CheckCurlFail(res, headers);

  res = curl_easy_setopt(
      http_curl, CURLOPT_NOSIGNAL,
      1);  // Add by SJKIM 2013.12.18 for signal safe [according to guide]
  CheckCurlFail(res, headers);

  res = curl_easy_perform(http_curl);

  if (res == CURLE_OK) {
    LOG_INFO("[DrmLicence] after curl_easy_perform : res(%d)", res);
    curl_easy_getinfo(http_curl, CURLINFO_RESPONSE_CODE,
                      static_cast<long*>(&http_session->res_code));
    LOG_INFO(
        "[DrmLicence] after curl_easy_perform : http_session->res_code(%ld)",
        http_session->res_code);
  } else if (res == CURLE_PARTIAL_FILE) {
    LOG_INFO("[DrmLicence] after curl_easy_perform : res(%d)", res);
    curl_easy_getinfo(http_curl, CURLINFO_RESPONSE_CODE,
                      static_cast<long*>(&http_session->res_code));
    LOG_INFO(
        "[DrmLicence] after curl_easy_perform : http_session->res_code(%ld)",
        http_session->res_code);
    res = CURLE_OK;
  } else if (res == CURLE_SEND_ERROR) {
    LOG_INFO("[DrmLicence] after curl_easy_perform : res(%d)", res);
    curl_easy_getinfo(http_curl, CURLINFO_RESPONSE_CODE,
                      static_cast<long*>(&http_session->res_code));
    LOG_INFO(
        "[DrmLicence] after curl_easy_perform : http_session->res_code(%ld)",
        http_session->res_code);
    res = CURLE_OK;
  } else {
    LOG_INFO("[DrmLicence] after curl_easy_perform : res(%d)", res);
    curl_easy_getinfo(http_curl, CURLINFO_RESPONSE_CODE,
                      static_cast<long*>(&http_session->res_code));
    LOG_INFO(
        "[DrmLicence] after curl_easy_perform : http_session->res_code(%ld)",
        http_session->res_code);
    if (res == CURLE_OPERATION_TIMEDOUT) {
      LOG_INFO("[DrmLicence] CURLE_OPERATION_TIMEDOUT occurred");
    }

    if (headers != nullptr) {
      curl_slist_free_all(headers);
    }

    if (res == CURLE_OUT_OF_MEMORY) {
      LOG_ERROR("[DrmLicence] Failed to alloc from curl.");
      return DRM_E_POINTER;
    } else if (res == CURLE_ABORTED_BY_CALLBACK) {
      *http_cancel_request = false;
      LOG_ERROR("[DrmLicence] Network job canceled by caller.");
      return DRM_E_NETWORK_CANCELED;
    } else {
      LOG_ERROR("[DrmLicence] Failed from curl, curl message : %s",
                curl_easy_strerror(res));
      return DRM_E_NETWORK_CURL;
    }
  }

  if (res != CURLE_OK) {
    if (res == CURLE_OUT_OF_MEMORY) {
      LOG_ERROR("[DrmLicence] Failed to alloc from curl.");
      return DRM_E_POINTER;
    } else {
      LOG_ERROR("[DrmLicence] Failed from curl, curl message : %s",
                curl_easy_strerror(res));
      return DRM_E_NETWORK_CURL;
    }
  }

  return DRM_SUCCESS;
}

void HttpClose(SHttpSession* http_session) {
  if (!http_session) {
    return;
  }

  if (http_session->curl_handle != nullptr) {
    curl_easy_cleanup(http_session->curl_handle);
  }

  if (http_session->post_data) {
    free(http_session->post_data);
  }

  if (http_session->body.data) {
    free(http_session->body.data);
  }

  if (http_session->header.data) {
    free(http_session->header.data);
  }

  free(http_session);
}

size_t ReceiveHeader(void* ptr, size_t size, size_t nmemb, void* stream) {
  LOG_DEBUG("[DrmLicence] size:%d nmemb:%d", (int)size, (int)nmemb);

  size_t data_size = size * nmemb;

  if (data_size > 0) {
    SHttpSession* http_session = reinterpret_cast<SHttpSession*>(stream);

    if (!AppendData(&http_session->header, ptr, data_size)) {
      return 0;
    }
  }
  return data_size;
}

size_t ReceiveBody(void* ptr, size_t size, size_t nmemb, void* stream) {
  LOG_DEBUG("[DrmLicence] size:%d nmemb:%d", (int)size, (int)nmemb);

  size_t data_size = size * nmemb;

  if (data_size > 0) {
    SHttpSession* http_session = reinterpret_cast<SHttpSession*>(stream);

    if (!AppendData(&http_session->body, ptr, data_size)) {
      return 0;
    }
  }
  return data_size;
}

size_t SendBody(void* ptr, size_t size, size_t nmemb, void* stream) {
  LOG_DEBUG("[DrmLicence] size:%d nmemb:%d", (int)size, (int)nmemb);

  SHttpSession* http_session = reinterpret_cast<SHttpSession*>(stream);

  size_t avail_data = http_session->post_data_len - http_session->send_data_len;
  size_t can_send = size * nmemb;

  if (avail_data == 0) {
    return 0;
  }

  if (can_send > avail_data) {
    can_send = avail_data;
  }

  memcpy(ptr, http_session->post_data + http_session->send_data_len, can_send);
  http_session->send_data_len += can_send;
  return can_send;
}
}  // namespace

DRM_RESULT DrmLicenseHelper::DoTransactionTZ(
    const char* http_server_url, const void* challenge,
    unsigned long challenge_len, unsigned char** response,
    unsigned long* response_len, DrmLicenseHelper::DrmType type,
    const char* http_cookie, SExtensionCtxTZ* http_ext_ctx) {
  *response = nullptr;
  *response_len = 0;

  const char* http_url = http_server_url;
  SHttpSession* http_session;
  char* redirect_url = nullptr;

  DRM_RESULT drm_result = DRM_SUCCESS;

  // Redirection 3 times..
  for (int i = 0; i < 3; i++) {
    if (!(http_session = HttpOpen())) {
      LOG_ERROR("[DrmLicence] Failed to open HTTP session.");
      break;
    }

    char* soap_header = nullptr;
    char* http_header = nullptr;
    char* user_agent = nullptr;
    bool* cancel_request = nullptr;

    if (http_ext_ctx != nullptr) {
      if (http_ext_ctx->http_soap_header) {
        soap_header = http_ext_ctx->http_soap_header;
      }

      if (http_ext_ctx->http_header) {
        soap_header = http_ext_ctx->http_header;
      }

      if (http_ext_ctx->http_user_agent) {
        user_agent = http_ext_ctx->http_user_agent;
      }

      cancel_request = &(http_ext_ctx->cancel_request);
    }

    drm_result = HttpStartTransaction(
        http_session, http_url, challenge, challenge_len, type, http_cookie,
        soap_header, http_header, user_agent, cancel_request);
    if (drm_result != DRM_SUCCESS) {
      LOG_ERROR(
          "[DrmLicence] Failed on network transaction(%d/%d), drm_result : "
          "0x%lx",
          i + 1, 3, drm_result);
      break;
    }

    if (http_session->res_code == 301 || http_session->res_code == 302) {
      // Convert https to http for GETSECURECLOCKSERVER_URL
      redirect_url = GetRedirectLocation(
          reinterpret_cast<const char*>(http_session->header.data), true);

      HttpClose(http_session);
      http_session = nullptr;
      if (!redirect_url) {
        LOG_ERROR("[DrmLicence] Failed to get redirect URL");
        break;
      }
      http_url = redirect_url;
    } else {
      if (http_session->res_code != 200) {
        LOG_ERROR("[DrmLicence] Server returns response Code %ld [%s][%d]",
                  http_session->res_code, http_session->body.data,
                  http_session->body.size);

        if (http_session->res_code >= 400 && http_session->res_code < 500) {
          drm_result = DRM_E_NETWORK_CLIENT;
        } else if (http_session->res_code >= 500 &&
                   http_session->res_code < 600) {
          drm_result = DRM_E_NETWORK_SERVER;
        } else {
          drm_result = DRM_E_NETWORK;
        }
        break;
      }

      *response = http_session->body.data;
      *response_len = http_session->body.size;

      http_session->body.data = nullptr;
      http_session->body.size = 0;
      http_session->body.allocated = 0;
      drm_result = DRM_SUCCESS;
      break;
    }
  }

  if (redirect_url) {
    free(redirect_url);
    redirect_url = nullptr;
  }

  HttpClose(http_session);

  if (drm_result != DRM_SUCCESS) {
    LOG_ERROR("[DrmLicence] Failed on network transaction, drm_result : 0x%lx",
              drm_result);
  }

  return drm_result;
}
