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

struct SDynamicBuf {
  unsigned char* iData;
  size_t iSize;
  size_t iAllocated;
};

struct SHttpSession {
  void* curl_handle;        // curl_handle
  unsigned char* postData;  // request body
  size_t postDataLen;       // length of request body
  CBmsDrmLicenseHelper::EDrmType type;
  size_t sendDataLen;  // length of send already
  SDynamicBuf header;  // response header
  SDynamicBuf body;    // response body
  long resCode;
};

// Internal Static Functions
static size_t g_ReceiveHeader(void* ptr, size_t size, size_t nmemb,
                              void* stream);
static size_t g_ReceiveBody(void* ptr, size_t size, size_t nmemb, void* stream);
static size_t g_SendBody(void* ptr, size_t size, size_t nmemb, void* stream);
static bool g_AppendData(SDynamicBuf* aBuf, const void* aData, size_t aSize);
static char* g_GetRedirectLocation(const char* aHeaders, bool bSupportHttps);
static struct curl_slist* g_CurlSlistAppend(struct curl_slist* aList,
                                            const char* aString);
static DRM_RESULT g_ComposePostData_TZ(SHttpSession* hSession,
                                       const char* f_pbPostData,
                                       int f_cbPostData,
                                       const char* f_extSoapHeader);
static struct curl_slist* g_SetHttpHeader(CURL* pCurl,
                                          CBmsDrmLicenseHelper::EDrmType f_type,
                                          const char* f_pCookie,
                                          const char* f_pHttpHeader,
                                          const char* f_pUserAgent);
static SHttpSession* g_HttpOpen(void);
static int g_CbCurlProgress(void* ptr, double TotalToDownload,
                            double NowDownloaded, double TotalToUpload,
                            double NowUploaded);
static DRM_RESULT g_HttpStartTransaction(
    SHttpSession* hSession, const char* f_pUrl, const void* f_pbPostData,
    unsigned f_cbPostData, CBmsDrmLicenseHelper::EDrmType f_type,
    const char* f_pCookie, const char* f_pSoapHeader, const char* f_pHttpHeader,
    const char* f_pUserAgent, bool* pCancelRequest);
static void g_HttpClose(SHttpSession* hSession);

bool g_AppendData(SDynamicBuf* aBuf, const void* aData, size_t aSize) {
  size_t newSize = aBuf->iSize + aSize;
  if (aBuf->iAllocated < newSize) {
    newSize += 1024;
    unsigned char* buf = (unsigned char*)realloc(aBuf->iData, newSize);
    if (!buf) {
      LOG_ERROR("g_AppendData : realloc fail ");
      return false;
    }
    aBuf->iData = buf;
    aBuf->iAllocated = newSize;
    LOG_INFO("g_AppendData : realloc aSize(%d), iSize(%d) aBuf->iAllocated(%d)",
             aSize, aBuf->iSize, aBuf->iAllocated);
  }
  memcpy(aBuf->iData + aBuf->iSize, aData, aSize);
  aBuf->iSize += aSize;

  return true;
}

char* g_GetRedirectLocation(const char* aHeaders, bool bSupportHttps) {
  if (!aHeaders) {
    return nullptr;
  }

  const char* pLocation = strcasestr(aHeaders, "Location");
  if (!pLocation) {
    return nullptr;
  }

  const char* ptr = pLocation + strlen("Location");

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

  if (bSupportHttps) {
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

struct curl_slist* g_CurlSlistAppend(struct curl_slist* aList,
                                     const char* aString) {
  if (!aList) {
    return nullptr;
  }

  struct curl_slist* newList = curl_slist_append(aList, aString);
  if (!newList) {
    curl_slist_free_all(aList);
  }

  return newList;
}

DRM_RESULT g_ComposePostData_TZ(SHttpSession* hSession,
                                const char* f_pbPostData, int f_cbPostData,
                                const char* f_extSoapHeader) {
  DRM_RESULT dr = DRM_SUCCESS;
  const char* p;
  char* dest;
  int dest_len;
  int remain;

  free(hSession->postData);
  hSession->postData = nullptr;
  hSession->postDataLen = 0;

  int extSoapHeaderLen = f_extSoapHeader ? strlen(f_extSoapHeader) : 0;

  dest_len = f_cbPostData;

  if (extSoapHeaderLen > 0) {
    dest_len += extSoapHeaderLen + sizeof("<soap:Header>\r\n</soap:Header>\r");
  }

  hSession->postData = (unsigned char*)malloc(dest_len + 1);
  if (hSession->postData == nullptr) {
    LOG_ERROR("Failed to alloc post data.");
    return DRM_E_POINTER;
  }
  dest = (char*)hSession->postData;
  remain = f_cbPostData;

  if (extSoapHeaderLen > 0) {
    /* append to the last in an existing soap header */
    p = strstr(f_pbPostData, "</soap:Header>");
    if (p > f_pbPostData && p < f_pbPostData + remain) {
      int hd_len = p - f_pbPostData;
      memcpy(dest, f_pbPostData, hd_len);
      dest += hd_len;
      dest_len -= hd_len;
      remain -= hd_len;

      memcpy(dest, f_extSoapHeader, extSoapHeaderLen);
      dest += extSoapHeaderLen;
      if (*dest == '\0') {
        dest--;
      }
    } else {
      /* insert soap header in front of soap body */
      p = strstr(f_pbPostData, "<soap:Body>");
      if (p > f_pbPostData && p < f_pbPostData + remain) {
        int hd_len = p - f_pbPostData;
        memcpy(dest, f_pbPostData, hd_len);
        dest += hd_len;
        dest_len -= hd_len;
        remain -= hd_len;
        *dest = '\0';
        strncat(dest, "<soap:Header>", dest_len);
        hd_len = strlen(dest);
        dest += hd_len;
        dest_len -= hd_len;

        memcpy(dest, f_extSoapHeader, extSoapHeaderLen);
        hd_len = extSoapHeaderLen;
        dest += hd_len;
        dest_len -= hd_len;

        *dest = '\0';
        strncat(dest, "</soap:Header>", dest_len);
        hd_len = strlen(dest);
        dest += hd_len;
        dest_len -= hd_len;
      } else {
        /* not a SOAP message */
        p = f_pbPostData;
      }
    }
  } else {
    p = f_pbPostData;
  }

  memcpy(dest, p, remain);
  dest += remain;
  *dest = '\0';

  hSession->postDataLen = dest - (char*)hSession->postData;
  if (extSoapHeaderLen > 0) {
    LOG_INFO("[soap header added %d ] %s ", hSession->postDataLen,
             hSession->postData);
  }

  return dr;
}

struct curl_slist* g_SetHttpHeader(CURL* pCurl,
                                   CBmsDrmLicenseHelper::EDrmType f_type,
                                   const char* f_pCookie,
                                   const char* f_pHttpHeader,
                                   const char* f_pUserAgent) {
  const char* userAgent = nullptr;
  const char* hdr = nullptr;

  if (f_type == CBmsDrmLicenseHelper::DRM_TYPE_PLAYREADY) {
    userAgent = DEFAULT_USER_AGENT_PLAYREADY;
    hdr = HTTP_HEADER_PLAYREADY_LICGET;
  } else if (f_type == CBmsDrmLicenseHelper::DRM_TYPE_WIDEVINE) {
    userAgent = DEFAULT_USER_AGENT_WIDEVINE;
    hdr = HTTP_HEADER_WIDEVINE_LICGET;
  } else {
    LOG_ERROR("Invalid DRM Type");
    return nullptr;
  }

  struct curl_slist* headers = nullptr;
  if (f_pUserAgent) {
    const char* userAgentPrefix = "User-Agent: ";
    unsigned prefixLen = strlen(userAgentPrefix);
    unsigned userAgentLen = strlen(f_pUserAgent);

    char* userAgentString = (char*)malloc(prefixLen + userAgentLen + 1);
    if (nullptr == userAgentString) {
      LOG_ERROR("Memory allocation failed.");
      return nullptr;
    }

    memcpy(userAgentString, userAgentPrefix, prefixLen);
    memcpy(userAgentString + prefixLen, f_pUserAgent, userAgentLen);
    userAgentString[prefixLen + userAgentLen] = 0;
    LOG_INFO("g_SetHttpHeader :  user-agent added to header --- (%s)",
             userAgentString);

    headers = curl_slist_append(nullptr, userAgentString);
    free(userAgentString);
  } else {
    headers = curl_slist_append(nullptr, userAgent);
  }

  if (nullptr == headers) {
    LOG_ERROR("UserAgent attach failed.");
    return nullptr;
  }

  LOG_INFO("g_SetHttpHeader : f_type(%d), f_pCookie(%s), f_pHttpHeader(%s)",
           f_type, f_pCookie, f_pHttpHeader);

  headers = g_CurlSlistAppend(headers, hdr);

  if (f_pCookie) {
    const char* cookiePrefix = "Cookie: ";
    unsigned prefixLen = strlen(cookiePrefix);
    unsigned cookieLen = strlen(f_pCookie);

    char* cookie = (char*)malloc(prefixLen + cookieLen + 1);

    if (cookie) {
      memcpy(cookie, cookiePrefix, prefixLen);
      memcpy(cookie + prefixLen, f_pCookie, cookieLen);
      cookie[prefixLen + cookieLen] = '\0';

      headers = g_CurlSlistAppend(headers, cookie);

      LOG_INFO("g_SetHttpHeader :  cookie added to header --- (%s)", cookie);

      free(cookie);
    }
  }

  if (f_pHttpHeader) {
    LOG_INFO("g_SetHttpHeader :  HttpHeader added to header --- (%s)",
             f_pHttpHeader);
    headers = g_CurlSlistAppend(headers, f_pHttpHeader);
  }

  if (headers) {
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, headers);
  }

  return headers;
}

static SHttpSession* g_HttpOpen(void) {
  SHttpSession* pSession = nullptr;

  CURL* pCurl = curl_easy_init();
  if (pCurl) {
    pSession = (SHttpSession*)malloc(sizeof(SHttpSession));
    if (pSession) {
      memset(pSession, 0, sizeof(SHttpSession));
      pSession->curl_handle = pCurl;
      return pSession;
    }
    curl_easy_cleanup(pCurl);
  }
  LOG_ERROR("Can't create CURL object, curl_global_init missed");
  return nullptr;
}

int g_CbCurlProgress(void* ptr, double TotalToDownload, double NowDownloaded,
                     double TotalToUpload, double NowUploaded) {
  bool* pCancelRequest = (bool*)ptr;

  if (pCancelRequest) {
    LOG_INFO("pCancelRequest : (%d)", *pCancelRequest);

    if (*pCancelRequest) {
      LOG_INFO("%s:%d curl works canceled.", __FUNCTION__, __LINE__);
      return 1;
    }
  }

  return 0;
}

DRM_RESULT g_HttpStartTransaction(
    SHttpSession* hSession, const char* f_pUrl, const void* f_pbPostData,
    unsigned f_cbPostData, CBmsDrmLicenseHelper::EDrmType f_type,
    const char* f_pCookie, const char* f_pSoapHeader, const char* f_pHttpHeader,
    const char* f_pUserAgent, bool* pCancelRequest) {
  CURLcode fRes = CURLE_OK;
  struct curl_slist* headers = nullptr;
  CURL* pCurl = hSession->curl_handle;

  // 1. Set Post Data
  hSession->postDataLen = f_cbPostData;
  hSession->sendDataLen = 0;
  hSession->body.iSize = 0;
  hSession->header.iSize = 0;

  LOG_INFO("g_HttpStartTransaction : f_type(%d)", f_type);
  if (f_pUrl) {
    LOG_INFO("f_pUrl : %s", f_pUrl);
  }

  // 2. Set Header type
  hSession->type = f_type;
  headers =
      g_SetHttpHeader(pCurl, f_type, f_pCookie, f_pHttpHeader, f_pUserAgent);
  if (!headers) {
    LOG_ERROR("Failed to set HTTP header.");
    return DRM_E_NETWORK_HEADER;
  }

  curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);

  // Check
  curl_easy_setopt(pCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

  int soap_flag = 0;

  if (f_pbPostData && f_cbPostData > 0) {
    if (f_pSoapHeader != nullptr) {
      DRM_RESULT dr = g_ComposePostData_TZ(hSession, (char*)f_pbPostData,
                                           f_cbPostData, f_pSoapHeader);
      if (dr != DRM_SUCCESS) {
        LOG_ERROR("Failed to compose post data, dr : 0x%lx", dr);
        return dr;
      } else if (dr == DRM_SUCCESS) {
        soap_flag = 1;
      }
    }

    fRes = curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
    CHECK_CURL_FAIL(fRes);

    if (soap_flag == 0) {
      if (!(hSession->postData = (unsigned char*)malloc(f_cbPostData))) {
        if (headers != nullptr) {
          curl_slist_free_all(headers);
        }
        LOG_ERROR("Failed to alloc post data.");
        return DRM_E_POINTER;
      }

      if (hSession->postData) {
        memcpy(hSession->postData, f_pbPostData, f_cbPostData);
        hSession->postDataLen = f_cbPostData;
      }
    }

    fRes = curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, g_SendBody);
    CHECK_CURL_FAIL(fRes);

    fRes =
        curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, hSession->postDataLen);
    CHECK_CURL_FAIL(fRes);

    fRes = curl_easy_setopt(pCurl, CURLOPT_READDATA, hSession);
    CHECK_CURL_FAIL(fRes);
  } else {
    curl_easy_setopt(pCurl, CURLOPT_HTTPGET, 1L);
  }

  curl_easy_setopt(pCurl, CURLOPT_USE_SSL, 1L);
  curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 1L);  // 0L
  curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, 2L);  // 0L

  // set timeout 10 seconds
  curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10);

  fRes = curl_easy_setopt(pCurl, CURLOPT_URL, f_pUrl);
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
  CHECK_CURL_FAIL(fRes);
  fRes = curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, g_CbCurlProgress);
  CHECK_CURL_FAIL(fRes);
  fRes = curl_easy_setopt(pCurl, CURLOPT_PROGRESSDATA, pCancelRequest);
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, g_ReceiveHeader);
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_setopt(pCurl, CURLOPT_BUFFERSIZE, 1024L * 20L);
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, g_ReceiveBody);
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_setopt(pCurl, CURLOPT_WRITEHEADER, hSession);
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, hSession);
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_setopt(
      pCurl, CURLOPT_NOSIGNAL,
      1);  // Add by SJKIM 2013.12.18 for signal safe [according to guide]
  CHECK_CURL_FAIL(fRes);

  fRes = curl_easy_perform(pCurl);

  if (fRes == CURLE_OK) {
    INFO(" after curl_easy_perform : fRes(%d)", fRes);
    curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, (long*)&hSession->resCode);
    INFO(" after curl_easy_perform : hSession->resCode(%ld)",
         hSession->resCode);
  } else if (fRes == CURLE_PARTIAL_FILE) {
    INFO(" after curl_easy_perform : fRes(%d)", fRes);
    curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, (long*)&hSession->resCode);
    INFO(" after curl_easy_perform : hSession->resCode(%ld)",
         hSession->resCode);
    fRes = CURLE_OK;
  } else if (fRes == CURLE_SEND_ERROR) {
    INFO(" after curl_easy_perform : fRes(%d)", fRes);
    curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, (long*)&hSession->resCode);
    INFO(" after curl_easy_perform : hSession->resCode(%ld)",
         hSession->resCode);
    fRes = CURLE_OK;
  } else {
    INFO(" after curl_easy_perform : fRes(%d)", fRes);
    curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, (long*)&hSession->resCode);
    INFO(" after curl_easy_perform : hSession->resCode(%ld)",
         hSession->resCode);
    if (fRes == CURLE_OPERATION_TIMEDOUT) {
      INFO("CURLE_OPERATION_TIMEDOUT occurred");
    }

    if (headers != nullptr) {
      curl_slist_free_all(headers);
    }

    if (fRes == CURLE_OUT_OF_MEMORY) {
      LOG_ERROR("Failed to alloc from curl.");
      return DRM_E_POINTER;
    } else if (fRes == CURLE_ABORTED_BY_CALLBACK) {
      *pCancelRequest = false;
      LOG_ERROR("Network job canceled by caller.");
      return DRM_E_NETWORK_CANCELED;
    } else {
      LOG_ERROR("Failed from curl, curl message : %s",
                curl_easy_strerror(fRes));
      return DRM_E_NETWORK_CURL;
    }
  }

ErrorExit:
  if (headers != nullptr) {
    INFO_CURL_HEADERS(headers);
    curl_slist_free_all(headers);
  }

  if (fRes != CURLE_OK) {
    if (fRes == CURLE_OUT_OF_MEMORY) {
      LOG_ERROR("Failed to alloc from curl.");
      return DRM_E_POINTER;
    } else {
      LOG_ERROR("Failed from curl, curl message : %s",
                curl_easy_strerror(fRes));
      return DRM_E_NETWORK_CURL;
    }
  }

  return DRM_SUCCESS;
}

void g_HttpClose(SHttpSession* hSession) {
  if (!hSession) {
    return;
  }

  if (hSession->curl_handle != nullptr) {
    curl_easy_cleanup(hSession->curl_handle);
  }

  if (hSession->postData) {
    free(hSession->postData);
  }

  if (hSession->body.iData) {
    free(hSession->body.iData);
  }

  if (hSession->header.iData) {
    free(hSession->header.iData);
  }

  free(hSession);
}

size_t g_ReceiveHeader(void* ptr, size_t size, size_t nmemb, void* pStream) {
  LOG_INFO("size:%d nmemb:%d", (int)size, (int)nmemb);

  size_t dataSize = size * nmemb;

  if (dataSize > 0) {
    SHttpSession* pSession = (SHttpSession*)pStream;

    if (!g_AppendData(&pSession->header, ptr, dataSize)) {
      return 0;
    }
  }
  return dataSize;
}

size_t g_ReceiveBody(void* ptr, size_t size, size_t nmemb, void* pStream) {
  LOG_INFO("size:%d nmemb:%d", (int)size, (int)nmemb);

  size_t dataSize = size * nmemb;

  if (dataSize > 0) {
    SHttpSession* pSession = (SHttpSession*)pStream;

    if (!g_AppendData(&pSession->body, ptr, dataSize)) {
      return 0;
    }
  }
  return dataSize;
}

size_t g_SendBody(void* ptr, size_t size, size_t nmemb, void* pStream) {
  LOG_INFO("size:%d nmemb:%d", (int)size, (int)nmemb);

  SHttpSession* pSession = (SHttpSession*)pStream;

  size_t availData = pSession->postDataLen - pSession->sendDataLen;
  size_t canSend = size * nmemb;

  if (availData == 0) {
    return 0;
  }

  if (canSend > availData) {
    canSend = availData;
  }

  memcpy(ptr, pSession->postData + pSession->sendDataLen, canSend);
  pSession->sendDataLen += canSend;
  return canSend;
}

DRM_RESULT CBmsDrmLicenseHelper::DoTransaction_TZ(
    const char* pServerUrl, const void* f_pbChallenge,
    unsigned long f_cbChallenge, unsigned char** f_ppbResponse,
    unsigned long* f_pcbResponse, CBmsDrmLicenseHelper::EDrmType f_type,
    const char* f_pCookie, SExtensionCtxTZ* pExtCtx) {
  *f_ppbResponse = nullptr;
  *f_pcbResponse = 0;

  const char* pUrl = pServerUrl;
  SHttpSession* pSession;
  char* szRedirectUrl = nullptr;

  DRM_RESULT dr = DRM_SUCCESS;

  // Redirection 3 times..
  for (int i = 0; i < 3; i++) {
    if (!(pSession = g_HttpOpen())) {
      LOG_ERROR("Failed to open HTTP session.");
      break;
    }

    char* pSoapHdr = nullptr;
    char* pHttpHdr = nullptr;
    char* pUserAgent = nullptr;
    bool* pCancelRequest = nullptr;

    if (pExtCtx != nullptr) {
      if (pExtCtx->pSoapHeader) {
        pSoapHdr = pExtCtx->pSoapHeader;
      }

      if (pExtCtx->pHttpHeader) {
        pHttpHdr = pExtCtx->pHttpHeader;
      }

      if (pExtCtx->pUserAgent) {
        pUserAgent = pExtCtx->pUserAgent;
      }

      pCancelRequest = &(pExtCtx->cancelRequest);
    }

    dr = g_HttpStartTransaction(pSession, pUrl, f_pbChallenge, f_cbChallenge,
                                f_type, f_pCookie, pSoapHdr, pHttpHdr,
                                pUserAgent, pCancelRequest);
    if (dr != DRM_SUCCESS) {
      LOG_ERROR("Failed on network transaction(%d/%d), dr : 0x%lx", i + 1, 3,
                dr);
      break;
    }

    if (pSession->resCode == 301 || pSession->resCode == 302) {
      if (szRedirectUrl) {
        free(szRedirectUrl);
        szRedirectUrl = nullptr;
      }

      // Convert https to http for GETSECURECLOCKSERVER_URL
      szRedirectUrl =
          g_GetRedirectLocation((const char*)pSession->header.iData, true);

      g_HttpClose(pSession);
      pSession = nullptr;
      if (!szRedirectUrl) {
        LOG_ERROR("Failed to get redirect URL");
        break;
      }
      pUrl = szRedirectUrl;
    } else {
      if (pSession->resCode != 200) {
        LOG_ERROR("Server returns response Code %ld [%s][%d]",
                  pSession->resCode, pSession->body.iData,
                  pSession->body.iSize);

        if (pSession->resCode >= 400 && pSession->resCode < 500) {
          dr = DRM_E_NETWORK_CLIENT;
        } else if (pSession->resCode >= 500 && pSession->resCode < 600) {
          dr = DRM_E_NETWORK_SERVER;
        } else {
          dr = DRM_E_NETWORK;
        }
        break;
      }

      *f_ppbResponse = pSession->body.iData;
      *f_pcbResponse = pSession->body.iSize;

      pSession->body.iData = nullptr;
      pSession->body.iSize = 0;
      pSession->body.iAllocated = 0;
      dr = DRM_SUCCESS;
      break;
    }
  }

  if (szRedirectUrl) {
    free(szRedirectUrl);
    szRedirectUrl = nullptr;
  }

  g_HttpClose(pSession);

  if (dr != DRM_SUCCESS) {
    LOG_ERROR("Failed on network transaction, dr : 0x%lx", dr);
  }

  return dr;
}
