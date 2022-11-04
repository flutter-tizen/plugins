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

#define CHECK_CURL_FAIL(expr)      \
  {                                \
    if (expr != CURLE_OK) {        \
      INFO("Error %d ", __LINE__); \
      goto ErrorExit;              \
    }                              \
  }

#define MAX_POPUP_MESSAGE_LENGTH (1024)
#define ERROR_TITLE_LICENSE_FETCHING_FAILURE "Fetching License Failed"

namespace {
struct SDynamicBuf {
  unsigned char* i_data;
  size_t i_size;
  size_t i_allocated;
};

struct SHttpSession {
  void* curl_handle;
  unsigned char* post_data;  // request body
  size_t post_data_len;      // length of request body
  DrmLicenseHelper::EDrmType type;
  size_t send_data_len;  // length of send already
  SDynamicBuf header;    // response header
  SDynamicBuf body;      // response body
  long res_code;
};

// Internal Static Functions
static size_t ReceiveHeader(void* ptr, size_t size, size_t nmemb, void* stream);
static size_t ReceiveBody(void* ptr, size_t size, size_t nmemb, void* stream);
static size_t SendBody(void* ptr, size_t size, size_t nmemb, void* stream);
static bool AppendData(SDynamicBuf* a_buf, const void* a_data, size_t a_size);
static char* GetRedirectLocation(const char* a_headers, bool b_support_https);
static struct curl_slist* CurlSlistAppend(struct curl_slist* a_list,
                                          const char* a_string);
static DRM_RESULT ComposePostDataTZ(SHttpSession* h_session,
                                    const char* pb_post_data, int cb_post_data,
                                    const char* ext_soap_header);
static struct curl_slist* SetHttpHeader(CURL* p_curl,
                                        DrmLicenseHelper::EDrmType type,
                                        const char* p_cookie,
                                        const char* p_http_header,
                                        const char* p_user_agent);
static SHttpSession* HttpOpen(void);
static int CbCurlProgress(void* ptr, double total_to_download,
                          double now_downloaded, double total_to_upload,
                          double now_uploaded);
static DRM_RESULT HttpStartTransaction(
    SHttpSession* h_session, const char* p_url, const void* pb_post_data,
    unsigned cb_post_data, DrmLicenseHelper::EDrmType type,
    const char* p_cookie, const char* p_soap_header, const char* p_http_header,
    const char* p_user_agent, bool* p_cancel_request);
static void HttpClose(SHttpSession* h_session);

bool AppendData(SDynamicBuf* a_buf, const void* a_data, size_t a_size) {
  size_t new_size = a_buf->i_size + a_size;
  if (a_buf->i_allocated < new_size) {
    new_size += 1024;
    unsigned char* buf =
        reinterpret_cast<unsigned char*>(realloc(a_buf->i_data, new_size));
    if (!buf) {
      LOG_ERROR("[DrmLicence] AppendData : realloc fail ");
      return false;
    }
    a_buf->i_data = buf;
    a_buf->i_allocated = new_size;
    LOG_DEBUG(
        "[DrmLicence] AppendData : realloc a_size(%d), i_size(%d) "
        "a_buf->i_allocated(%d)",
        a_size, a_buf->i_size, a_buf->i_allocated);
  }
  memcpy(a_buf->i_data + a_buf->i_size, a_data, a_size);
  a_buf->i_size += a_size;

  return true;
}

char* GetRedirectLocation(const char* a_headers, bool b_support_https) {
  if (!a_headers) {
    return nullptr;
  }

  const char* p_location = strcasestr(a_headers, "Location");
  if (!p_location) {
    return nullptr;
  }

  const char* ptr = p_location + strlen("Location");

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

  if (b_support_https) {
    // [soyoung] get redirection location
    // for using https itself
    char* ret = (char*)malloc(i + 1);
    if (!ret) {
      return nullptr;
    }
    memcpy(ret, ptr, i);
    ret[i] = 0;
    return ret;
  } else {
    // Convert Redirection Location from https to http
    // [soyoung]
    // Redirect location 捞 https 老 版快绰 http 肺 函券
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

    char* ret = (char*)malloc(len + 1);
    if (!ret) {
      return nullptr;
    }

    memcpy(ret, http_str, 4);
    memcpy(ret + 4, p, len - 4);
    ret[len] = 0;
    return ret;
  }
}

struct curl_slist* CurlSlistAppend(struct curl_slist* a_list,
                                   const char* a_string) {
  if (!a_list) {
    return nullptr;
  }

  struct curl_slist* new_list = curl_slist_append(a_list, a_string);
  if (!new_list) {
    curl_slist_free_all(a_list);
  }

  return new_list;
}

DRM_RESULT ComposePostDataTZ(SHttpSession* h_session, const char* pb_post_data,
                             int cb_post_data, const char* ext_soap_header) {
  DRM_RESULT dr = DRM_SUCCESS;
  const char* p;
  char* dest;
  int dest_len;
  int remain;

  h_session->post_data_len = 0;

  int ext_soap_header_len = ext_soap_header ? strlen(ext_soap_header) : 0;

  dest_len = cb_post_data;

  if (ext_soap_header_len > 0) {
    dest_len +=
        ext_soap_header_len + sizeof("<soap:Header>\r\n</soap:Header>\r");
  }

  h_session->post_data = reinterpret_cast<unsigned char*>(malloc(dest_len + 1));
  if (h_session->post_data == nullptr) {
    LOG_ERROR("[DrmLicence] Failed to alloc post data.");
    return DRM_E_POINTER;
  }
  dest = (char*)h_session->post_data;
  remain = cb_post_data;

  if (ext_soap_header_len > 0) {
    /* append to the last in an existing soap header */
    p = strstr(pb_post_data, "</soap:Header>");
    if (p > pb_post_data && p < pb_post_data + remain) {
      int hd_len = p - pb_post_data;
      memcpy(dest, pb_post_data, hd_len);
      dest += hd_len;
      dest_len -= hd_len;
      remain -= hd_len;

      memcpy(dest, ext_soap_header, ext_soap_header_len);
      dest += ext_soap_header_len;
      if (*dest == '\0') {
        dest--;
      }
    } else {
      /* insert soap header in front of soap body */
      p = strstr(pb_post_data, "<soap:Body>");
      if (p > pb_post_data && p < pb_post_data + remain) {
        int hd_len = p - pb_post_data;
        memcpy(dest, pb_post_data, hd_len);
        dest += hd_len;
        dest_len -= hd_len;
        remain -= hd_len;
        *dest = '\0';
        strncat(dest, "<soap:Header>", dest_len);
        hd_len = strlen(dest);
        dest += hd_len;
        dest_len -= hd_len;

        memcpy(dest, ext_soap_header, ext_soap_header_len);
        hd_len = ext_soap_header_len;
        dest += hd_len;
        dest_len -= hd_len;

        *dest = '\0';
        strncat(dest, "</soap:Header>", dest_len);
        hd_len = strlen(dest);
        dest += hd_len;
        dest_len -= hd_len;
      } else {
        /* not a SOAP message */
        p = pb_post_data;
      }
    }
  } else {
    p = pb_post_data;
  }

  memcpy(dest, p, remain);
  dest += remain;
  *dest = '\0';

  h_session->post_data_len = dest - (char*)h_session->post_data;
  if (ext_soap_header_len > 0) {
    LOG_INFO("[DrmLicence] [soap header added %d ] %s ",
             h_session->post_data_len, h_session->post_data);
  }

  return dr;
}

struct curl_slist* SetHttpHeader(CURL* p_curl, DrmLicenseHelper::EDrmType type,
                                 const char* p_cookie,
                                 const char* p_http_header,
                                 const char* p_user_agent) {
  const char* user_agent = nullptr;
  const char* hdr = nullptr;

  switch (type) {
    case DrmLicenseHelper::DRM_TYPE_PLAYREADY:
      user_agent = DEFAULT_USER_AGENT_PLAYREADY;
      hdr = HTTP_HEADER_PLAYREADY_LICGET;
      break;
    case DrmLicenseHelper::DRM_TYPE_WIDEVINE:
      user_agent = DEFAULT_USER_AGENT_WIDEVINE;
      hdr = HTTP_HEADER_WIDEVINE_LICGET;
      break;
    default:
      LOG_ERROR("[DrmLicence] Invalid DRM Type");
      return nullptr;
  }

  struct curl_slist* headers = nullptr;
  if (p_user_agent) {
    const char* user_agent_prefix = "User-Agent: ";
    unsigned prefix_len = strlen(user_agent_prefix);
    unsigned user_agent_len = strlen(p_user_agent);

    char* user_agent_string = (char*)malloc(prefix_len + user_agent_len + 1);
    if (nullptr == user_agent_string) {
      LOG_ERROR("[DrmLicence] Memory allocation failed.");
      return nullptr;
    }

    memcpy(user_agent_string, user_agent_prefix, prefix_len);
    memcpy(user_agent_string + prefix_len, p_user_agent, user_agent_len);
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
      "[DrmLicence] SetHttpHeader : type(%d), p_cookie(%s), "
      "p_http_header(%s)",
      type, p_cookie, p_http_header);

  headers = CurlSlistAppend(headers, hdr);

  if (p_cookie) {
    const char* cookie_prefix = "Cookie: ";
    unsigned prefix_len = strlen(cookie_prefix);
    unsigned cookie_len = strlen(p_cookie);

    char* cookie = (char*)malloc(prefix_len + cookie_len + 1);

    if (cookie) {
      memcpy(cookie, cookie_prefix, prefix_len);
      memcpy(cookie + prefix_len, p_cookie, cookie_len);
      cookie[prefix_len + cookie_len] = '\0';

      headers = CurlSlistAppend(headers, cookie);

      LOG_INFO("[DrmLicence] SetHttpHeader :  cookie added to header --- (%s)",
               cookie);

      free(cookie);
    }
  }

  if (p_http_header) {
    LOG_INFO(
        "[DrmLicence] SetHttpHeader :  HttpHeader added to header --- (%s)",
        p_http_header);
    headers = CurlSlistAppend(headers, p_http_header);
  }

  if (headers) {
    curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, headers);
  }

  return headers;
}

static SHttpSession* HttpOpen(void) {
  SHttpSession* p_session = nullptr;

  CURL* p_curl = curl_easy_init();
  if (p_curl) {
    p_session = (SHttpSession*)malloc(sizeof(SHttpSession));
    if (p_session) {
      memset(p_session, 0, sizeof(SHttpSession));
      p_session->curl_handle = p_curl;
      return p_session;
    }
    curl_easy_cleanup(p_curl);
  }
  LOG_ERROR("[DrmLicence] Can't create CURL object, curl_global_init missed");
  return nullptr;
}

int CbCurlProgress(void* ptr, double total_to_download, double now_downloaded,
                   double total_to_upload, double now_uploaded) {
  bool* p_cancel_qequest = (bool*)ptr;

  if (p_cancel_qequest) {
    LOG_INFO("[DrmLicence] p_cancel_qequest : (%d)", *p_cancel_qequest);

    if (*p_cancel_qequest) {
      LOG_INFO("[DrmLicence] %s:%d curl works canceled.", __FUNCTION__,
               __LINE__);
      return 1;
    }
  }

  return 0;
}

DRM_RESULT HttpStartTransaction(SHttpSession* h_session, const char* p_url,
                                const void* pb_post_data, unsigned cb_post_data,
                                DrmLicenseHelper::EDrmType type,
                                const char* p_cookie, const char* p_soap_header,
                                const char* p_http_header,
                                const char* p_user_agent,
                                bool* p_cancel_request) {
  CURLcode f_res = CURLE_OK;
  struct curl_slist* headers = nullptr;
  CURL* p_curl = h_session->curl_handle;

  // 1. Set Post Data
  h_session->post_data_len = cb_post_data;
  h_session->send_data_len = 0;
  h_session->body.i_size = 0;
  h_session->header.i_size = 0;

  LOG_INFO("[DrmLicence] HttpStartTransaction : type(%d)", type);
  if (p_url) {
    LOG_INFO("[DrmLicence] p_url : %s", p_url);
  }

  // 2. Set Header type
  h_session->type = type;
  headers = SetHttpHeader(p_curl, type, p_cookie, p_http_header, p_user_agent);
  if (!headers) {
    LOG_ERROR("[DrmLicence] Failed to set HTTP header.");
    return DRM_E_NETWORK_HEADER;
  }

  curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 0L);

  // Check
  curl_easy_setopt(p_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

  int soap_flag = 0;
  free(h_session->post_data);
  h_session->post_data = nullptr;
  if (pb_post_data && cb_post_data > 0) {
    if (p_soap_header != nullptr) {
      DRM_RESULT dr = ComposePostDataTZ(h_session, (char*)pb_post_data,
                                        cb_post_data, p_soap_header);
      if (dr != DRM_SUCCESS) {
        LOG_ERROR("[DrmLicence] Failed to compose post data, dr : 0x%lx", dr);
        return dr;
      } else if (dr == DRM_SUCCESS) {
        soap_flag = 1;
      }
    }

    f_res = curl_easy_setopt(p_curl, CURLOPT_POST, 1L);
    CHECK_CURL_FAIL(f_res);

    if (soap_flag == 0) {
      if (!(h_session->post_data =
                reinterpret_cast<unsigned char*>(malloc(cb_post_data)))) {
        if (headers != nullptr) {
          curl_slist_free_all(headers);
        }
        LOG_ERROR("[DrmLicence] Failed to alloc post data.");
        return DRM_E_POINTER;
      }

      if (h_session->post_data) {
        memcpy(h_session->post_data, pb_post_data, cb_post_data);
        h_session->post_data_len = cb_post_data;
      }
    }

    f_res = curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, SendBody);
    CHECK_CURL_FAIL(f_res);

    f_res = curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE,
                             h_session->post_data_len);
    CHECK_CURL_FAIL(f_res);

    f_res = curl_easy_setopt(p_curl, CURLOPT_READDATA, h_session);
    CHECK_CURL_FAIL(f_res);
  } else {
    curl_easy_setopt(p_curl, CURLOPT_HTTPGET, 1L);
  }

  curl_easy_setopt(p_curl, CURLOPT_USE_SSL, 1L);
  curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 1L);  // 0L
  curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, 2L);  // 0L

  // set timeout 10 seconds
  curl_easy_setopt(p_curl, CURLOPT_TIMEOUT, 10);

  f_res = curl_easy_setopt(p_curl, CURLOPT_URL, p_url);
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_setopt(p_curl, CURLOPT_NOPROGRESS, 0L);
  CHECK_CURL_FAIL(f_res);
  f_res = curl_easy_setopt(p_curl, CURLOPT_PROGRESSFUNCTION, CbCurlProgress);
  CHECK_CURL_FAIL(f_res);
  f_res = curl_easy_setopt(p_curl, CURLOPT_PROGRESSDATA, p_cancel_request);
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_setopt(p_curl, CURLOPT_HEADERFUNCTION, ReceiveHeader);
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_setopt(p_curl, CURLOPT_BUFFERSIZE, 1024L * 20L);
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, ReceiveBody);
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, h_session);
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, h_session);
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_setopt(
      p_curl, CURLOPT_NOSIGNAL,
      1);  // Add by SJKIM 2013.12.18 for signal safe [according to guide]
  CHECK_CURL_FAIL(f_res);

  f_res = curl_easy_perform(p_curl);

  if (f_res == CURLE_OK) {
    LOG_INFO("[DrmLicence] after curl_easy_perform : f_res(%d)", f_res);
    curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE,
                      (long*)&h_session->res_code);
    LOG_INFO("[DrmLicence] after curl_easy_perform : h_session->res_code(%ld)",
             h_session->res_code);
  } else if (f_res == CURLE_PARTIAL_FILE) {
    LOG_INFO("[DrmLicence] after curl_easy_perform : f_res(%d)", f_res);
    curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE,
                      (long*)&h_session->res_code);
    LOG_INFO("[DrmLicence] after curl_easy_perform : h_session->res_code(%ld)",
             h_session->res_code);
    f_res = CURLE_OK;
  } else if (f_res == CURLE_SEND_ERROR) {
    LOG_INFO("[DrmLicence] after curl_easy_perform : f_res(%d)", f_res);
    curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE,
                      (long*)&h_session->res_code);
    LOG_INFO("[DrmLicence] after curl_easy_perform : h_session->res_code(%ld)",
             h_session->res_code);
    f_res = CURLE_OK;
  } else {
    LOG_INFO("[DrmLicence] after curl_easy_perform : f_res(%d)", f_res);
    curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE,
                      (long*)&h_session->res_code);
    LOG_INFO("[DrmLicence] after curl_easy_perform : h_session->res_code(%ld)",
             h_session->res_code);
    if (f_res == CURLE_OPERATION_TIMEDOUT) {
      LOG_INFO("[DrmLicence] CURLE_OPERATION_TIMEDOUT occurred");
    }

    if (headers != nullptr) {
      curl_slist_free_all(headers);
    }

    if (f_res == CURLE_OUT_OF_MEMORY) {
      LOG_ERROR("[DrmLicence] Failed to alloc from curl.");
      return DRM_E_POINTER;
    } else if (f_res == CURLE_ABORTED_BY_CALLBACK) {
      *p_cancel_request = false;
      LOG_ERROR("[DrmLicence] Network job canceled by caller.");
      return DRM_E_NETWORK_CANCELED;
    } else {
      LOG_ERROR("[DrmLicence] Failed from curl, curl message : %s",
                curl_easy_strerror(f_res));
      return DRM_E_NETWORK_CURL;
    }
  }

ErrorExit:
  if (headers != nullptr) {
    INFO_CURL_HEADERS(headers);
    curl_slist_free_all(headers);
  }

  if (f_res != CURLE_OK) {
    if (f_res == CURLE_OUT_OF_MEMORY) {
      LOG_ERROR("[DrmLicence] Failed to alloc from curl.");
      return DRM_E_POINTER;
    } else {
      LOG_ERROR("[DrmLicence] Failed from curl, curl message : %s",
                curl_easy_strerror(f_res));
      return DRM_E_NETWORK_CURL;
    }
  }

  return DRM_SUCCESS;
}

void HttpClose(SHttpSession* h_session) {
  if (!h_session) {
    return;
  }

  if (h_session->curl_handle != nullptr) {
    curl_easy_cleanup(h_session->curl_handle);
  }

  if (h_session->post_data) {
    free(h_session->post_data);
  }

  if (h_session->body.i_data) {
    free(h_session->body.i_data);
  }

  if (h_session->header.i_data) {
    free(h_session->header.i_data);
  }

  free(h_session);
}

size_t ReceiveHeader(void* ptr, size_t size, size_t nmemb, void* p_stream) {
  LOG_DEBUG("[DrmLicence] size:%d nmemb:%d", (int)size, (int)nmemb);

  size_t data_size = size * nmemb;

  if (data_size > 0) {
    SHttpSession* p_session = (SHttpSession*)p_stream;

    if (!AppendData(&p_session->header, ptr, data_size)) {
      return 0;
    }
  }
  return data_size;
}

size_t ReceiveBody(void* ptr, size_t size, size_t nmemb, void* p_stream) {
  LOG_DEBUG("[DrmLicence] size:%d nmemb:%d", (int)size, (int)nmemb);

  size_t data_size = size * nmemb;

  if (data_size > 0) {
    SHttpSession* p_session = (SHttpSession*)p_stream;

    if (!AppendData(&p_session->body, ptr, data_size)) {
      return 0;
    }
  }
  return data_size;
}

size_t SendBody(void* ptr, size_t size, size_t nmemb, void* p_stream) {
  LOG_DEBUG("[DrmLicence] size:%d nmemb:%d", (int)size, (int)nmemb);

  SHttpSession* p_session = (SHttpSession*)p_stream;

  size_t avail_data = p_session->post_data_len - p_session->send_data_len;
  size_t can_send = size * nmemb;

  if (avail_data == 0) {
    return 0;
  }

  if (can_send > avail_data) {
    can_send = avail_data;
  }

  memcpy(ptr, p_session->post_data + p_session->send_data_len, can_send);
  p_session->send_data_len += can_send;
  return can_send;
}
}  // namespace

DRM_RESULT DrmLicenseHelper::DoTransactionTZ(
    const char* p_server_url, const void* pb_challenge,
    unsigned long cb_challenge, unsigned char** ppb_response,
    unsigned long* pcb_response, DrmLicenseHelper::EDrmType type,
    const char* p_cookie, SExtensionCtxTZ* p_ext_ctx) {
  *ppb_response = nullptr;
  *pcb_response = 0;

  const char* p_url = p_server_url;
  SHttpSession* p_session;
  char* sz_redirect_url = nullptr;

  DRM_RESULT dr = DRM_SUCCESS;

  // Redirection 3 times..
  for (int i = 0; i < 3; i++) {
    if (!(p_session = HttpOpen())) {
      LOG_ERROR("[DrmLicence] Failed to open HTTP session.");
      break;
    }

    char* p_soap_hdr = nullptr;
    char* p_http_hdr = nullptr;
    char* p_user_agent = nullptr;
    bool* p_cancel_request = nullptr;

    if (p_ext_ctx != nullptr) {
      if (p_ext_ctx->p_soap_header) {
        p_soap_hdr = p_ext_ctx->p_soap_header;
      }

      if (p_ext_ctx->p_http_header) {
        p_http_hdr = p_ext_ctx->p_http_header;
      }

      if (p_ext_ctx->p_user_agent) {
        p_user_agent = p_ext_ctx->p_user_agent;
      }

      p_cancel_request = &(p_ext_ctx->cancel_request);
    }

    dr = HttpStartTransaction(p_session, p_url, pb_challenge, cb_challenge,
                              type, p_cookie, p_soap_hdr, p_http_hdr,
                              p_user_agent, p_cancel_request);
    if (dr != DRM_SUCCESS) {
      LOG_ERROR("[DrmLicence] Failed on network transaction(%d/%d), dr : 0x%lx",
                i + 1, 3, dr);
      break;
    }

    if (p_session->res_code == 301 || p_session->res_code == 302) {
      // Convert https to http for GETSECURECLOCKSERVER_URL
      sz_redirect_url =
          GetRedirectLocation((const char*)p_session->header.i_data, true);

      HttpClose(p_session);
      p_session = nullptr;
      if (!sz_redirect_url) {
        LOG_ERROR("[DrmLicence] Failed to get redirect URL");
        break;
      }
      p_url = sz_redirect_url;
    } else {
      if (p_session->res_code != 200) {
        LOG_ERROR("[DrmLicence] Server returns response Code %ld [%s][%d]",
                  p_session->res_code, p_session->body.i_data,
                  p_session->body.i_size);

        if (p_session->res_code >= 400 && p_session->res_code < 500) {
          dr = DRM_E_NETWORK_CLIENT;
        } else if (p_session->res_code >= 500 && p_session->res_code < 600) {
          dr = DRM_E_NETWORK_SERVER;
        } else {
          dr = DRM_E_NETWORK;
        }
        break;
      }

      *ppb_response = p_session->body.i_data;
      *pcb_response = p_session->body.i_size;

      p_session->body.i_data = nullptr;
      p_session->body.i_size = 0;
      p_session->body.i_allocated = 0;
      dr = DRM_SUCCESS;
      break;
    }
  }

  if (sz_redirect_url) {
    free(sz_redirect_url);
    sz_redirect_url = nullptr;
  }

  HttpClose(p_session);

  if (dr != DRM_SUCCESS) {
    LOG_ERROR("[DrmLicence] Failed on network transaction, dr : 0x%lx", dr);
  }

  return dr;
}
