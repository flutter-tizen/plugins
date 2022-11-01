/*
 * Copyright (c) 2018-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __LWEWebView__
#define __LWEWebView__

#ifndef LWE_EXPORT
#ifdef _MSC_VER
#define LWE_EXPORT __declspec(dllexport)
#else
#define LWE_EXPORT __attribute__((visibility("default")))
#endif
#endif

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "PlatformIntegrationData.h"

namespace LWE {

class LWE_EXPORT LWE {
 public:
  // You must call Initialize function before using WebContainer or WebView
  static void Initialize(const char* localStorageDataFilePath,
                         const char* cookieStoreDataFilePath,
                         const char* httpCacheDataDirectorypath);
  static bool IsInitialized();
  static void Finalize();
};

#define LWE_DEFAULT_FONT_SIZE 16
#define LWE_MIN_FONT_SIZE 1
#define LWE_MAX_FONT_SIZE 72

enum class WebSecurityMode;

class LWE_EXPORT CookieManager {
 public:
  std::string GetCookie(std::string url);
  bool HasCookies();
  void ClearCookies();

  static CookieManager* GetInstance();
  static void Destroy();

 private:
  CookieManager();
  ~CookieManager();
};

class LWE_EXPORT Settings {
 public:
  Settings(const std::string& defaultUA, const std::string& ua);
  std::string GetDefaultUserAgent() const;
  std::string GetUserAgentString() const;
  std::string GetProxyURL() const;
  int GetCacheMode() const;
  TTSMode GetTTSMode() const;
  std::string GetTTSLanguage() const;
  WebSecurityMode GetWebSecurityMode() const;
  IdleModeJob GetIdleModeJob() const;
  uint32_t GetIdleModeCheckIntervalInMS() const;
  void GetBaseBackgroundColor(unsigned char& r, unsigned char& g,
                              unsigned char& b, unsigned char& a) const;
  void GetBaseForegroundColor(unsigned char& r, unsigned char& g,
                              unsigned char& b, unsigned char& a) const;
  bool NeedsDownloadWebFontsEarly() const;
  bool UseHttp2() const;
  uint32_t NeedsDownScaleImageResourceLargerThan() const;
  bool ScrollbarVisible() const;
  bool UseExternalPopup() const;
  void SetUserAgentString(const std::string& ua);
  void SetCacheMode(int mode);
  void SetProxyURL(const std::string& proxyURL);
  void setDefaultFontSize(int size);
  void SetTTSMode(TTSMode value);
  void SetTTSLanguage(const std::string& language);
  void SetBaseBackgroundColor(unsigned char r, unsigned char g, unsigned char b,
                              unsigned char a);
  void SetBaseForegroundColor(unsigned char r, unsigned char g, unsigned char b,
                              unsigned char a);
  void SetWebSecurityMode(WebSecurityMode value);
  void SetIdleModeJob(IdleModeJob j);
  void SetIdleModeCheckIntervalInMS(uint32_t intervalInMS);
  void SetNeedsDownloadWebFontsEarly(bool b);
  void SetUseHttp2(bool b);
  void SetNeedsDownScaleImageResourceLargerThan(
      uint32_t demention);  // Experimental
  void SetScrollbarVisible(bool visible);
  void SetUseExternalPopup(bool useExternalPopup);

 private:
  std::string m_defaultUserAgent;
  std::string m_userAgent;
  std::string m_proxyURL;
  int m_cacheMode;
  uint32_t m_defaultFontSize;
  TTSMode m_ttsMode;
  std::string m_ttsLanguage;
  unsigned char m_bgR, m_bgG, m_bgB, m_bgA;
  unsigned char m_fgR, m_fgG, m_fgB, m_fgA;
  WebSecurityMode m_webSecurityMode;
  IdleModeJob m_idleModeJob;  // default value is IdleModeJob::IdleModeFull
  uint32_t m_idleModeCheckIntervalInMS;  // default value is 3000(ms)
  bool m_needsDownloadWebFontsEarly;
  bool m_useHttp2;  // default value is false
  uint32_t m_needsDownScaleImageResourceLargerThan;
  bool m_scrollbarVisible;
  bool m_useExternalPopup;
};

class LWE_EXPORT ResourceError {
 public:
  ResourceError(int code, const std::string& description,
                const std::string& url);
  int GetErrorCode();
  std::string GetDescription();
  std::string GetUrl();

 private:
  int m_errorCode;
  std::string m_description;
  std::string m_url;
};

class LWE_EXPORT WebContainer {
 private:
  // use Destroy function instead of using delete operator
  ~WebContainer() {}

 public:
  // Function set for render to buffer
  static WebContainer* Create(unsigned width, unsigned height,
                              float devicePixelRatio,
                              const char* defaultFontName, const char* locale,
                              const char* timezoneID);
  struct RenderInfo {
    void* updatedBufferAddress;
    size_t bufferStride;
  };

  struct ExternalImageInfo {
    void* imageAddress;
  };

  struct RenderResult {
    size_t updatedX;
    size_t updatedY;
    size_t updatedWidth;
    size_t updatedHeight;

    void* updatedBufferAddress;
    size_t bufferImageWidth;
    size_t bufferImageHeight;
  };
  void RegisterPreRenderingHandler(const std::function<RenderInfo(void)>& cb);
  void RegisterOnRenderedHandler(
      const std::function<void(WebContainer*,
                               const RenderResult& renderResult)>& cb);

  static WebContainer* CreateWithPlatformImage(
      unsigned width, unsigned height,
      const std::function<ExternalImageInfo(void)>& prepareImageCb,
      const std::function<void(WebContainer*, bool needsFlush)>& flushCb,
      float devicePixelRatio, const char* defaultFontName, const char* locale,
      const char* timezoneID);
  // <--- end of function set for render to buffer

  // Function set for render with OpenGL
  static WebContainer* CreateGL(
      unsigned width, unsigned height,
      const std::function<void(WebContainer*)>& onGLMakeCurrent,
      const std::function<void(WebContainer*, bool mayNeedsSync)>&
          onGLSwapBuffers,
      float devicePixelRatio, const char* defaultFontName, const char* locale,
      const char* timezoneID);

  static WebContainer* CreateGLWithPlatformImage(
      unsigned width, unsigned height,
      const std::function<void(WebContainer*)>& onGLMakeCurrent,
      const std::function<void(WebContainer*, bool mayNeedsSync)>&
          onGLSwapBuffers,
      const std::function<ExternalImageInfo(void)>& prepareImageCb,
      const std::function<void(WebContainer*, bool needsFlush)>& flushCb,
      float devicePixelRatio, const char* defaultFontName, const char* locale,
      const char* timezoneID);

  // <--- end of function set for render with OpenGL

  // Function set for headless
  static WebContainer* CreateHeadless(unsigned width, unsigned height,
                                      float devicePixelRatio,
                                      const char* defaultFontName,
                                      const char* locale,
                                      const char* timezoneID);
  // <--- end of function set for headless

  void AddIdleCallback(void (*callback)(void*), void* data);
  size_t AddTimeout(void (*callback)(void*), void* data, size_t timeoutInMS);
  void ClearTimeout(size_t handle);

  void RegisterCanRenderingHandler(
      const std::function<bool(WebContainer*)>& cb);

  Settings GetSettings();
  void LoadURL(const std::string& url);
  std::string GetURL();
  void LoadData(const std::string& data);
  void Reload();
  void StopLoading();
  void GoBack();
  void GoForward();
  bool CanGoBack();
  bool CanGoForward();
  void AddJavaScriptInterface(
      const std::string& exposedObjectName, const std::string& jsFunctionName,
      std::function<std::string(const std::string&)> cb);
  std::string EvaluateJavaScript(const std::string& script);
  void EvaluateJavaScript(const std::string& script,
                          std::function<void(const std::string&)> cb);
  void ClearHistory();
  void Destroy();
  void Pause();
  void Resume();

  void ResizeTo(size_t width, size_t height);

  void Focus();
  void Blur();

  void SetSettings(const Settings& settings);
  void RemoveJavascriptInterface(const std::string& exposedObjectName,
                                 const std::string& jsFunctionName);
  void ClearCache();

  void RegisterOnReceivedErrorHandler(
      const std::function<void(WebContainer*, ResourceError)>& cb);
  void RegisterOnPageParsedHandler(
      std::function<void(WebContainer*, const std::string&)> cb);
  void RegisterOnPageLoadedHandler(
      std::function<void(WebContainer*, const std::string&)> cb);
  void RegisterOnPageStartedHandler(
      const std::function<void(WebContainer*, const std::string&)>& cb);
  void RegisterOnLoadResourceHandler(
      const std::function<void(WebContainer*, const std::string&)>& cb);
  void RegisterShouldOverrideUrlLoadingHandler(
      const std::function<bool(WebContainer*, const std::string&)>& cb);
  void RegisterOnProgressChangedHandler(
      const std::function<void(WebContainer*, int progress)>& cb);
  void RegisterOnDownloadStartHandler(
      const std::function<void(WebContainer*, const std::string&,
                               const std::string&, const std::string&,
                               const std::string&, long)>& cb);

  void RegisterShowDropdownMenuHandler(
      const std::function<void(WebContainer*, const std::vector<std::string>*,
                               int)>& cb);
  void RegisterShowAlertHandler(
      const std::function<void(WebContainer*, const std::string&,
                               const std::string&)>& cb);

  void RegisterCustomFileResourceRequestHandlers(
      std::function<const char*(const char* path)> resolveFilePathCallback,
      std::function<void*(const char* path)> fileOpenCallback,
      std::function<size_t(uint8_t* destBuffer, size_t size, void* handle)>
          fileReadCallback,
      std::function<long int(void* handle)> fileLengthCallback,
      std::function<void(void* handle)> fileCloseCallback);

  void RegisterDebuggerShouldInitHandler(
      const std::function<void(const std::string& url, int port,
                               bool& shouldInit)>& cb);
  void RegisterDebuggerShouldContinueWaitingHandler(
      const std::function<void(const std::string& url, int port,
                               bool& shouldWait)>& cb);

  void CallHandler(const std::string& handler, void* param);

  void SetUserAgentString(const std::string& userAgent);
  std::string GetUserAgentString();
  void SetCacheMode(int mode);
  int GetCacheMode();
  void SetDefaultFontSize(uint32_t size);
  uint32_t GetDefaultFontSize();

  void DispatchMouseMoveEvent(MouseButtonValue button,
                              MouseButtonsValue buttons, double x, double y);
  void DispatchMouseDownEvent(MouseButtonValue button,
                              MouseButtonsValue buttons, double x, double y);
  void DispatchMouseUpEvent(MouseButtonValue button, MouseButtonsValue buttons,
                            double x, double y);
  void DispatchMouseWheelEvent(double x, double y, int delta);
  void DispatchKeyDownEvent(KeyValue keyCode);
  void DispatchKeyPressEvent(KeyValue keyCode);
  void DispatchKeyUpEvent(KeyValue keyCode);

  void DispatchCompositionStartEvent(const std::string& soFarCompositiedString);
  void DispatchCompositionUpdateEvent(
      const std::string& soFarCompositiedString);
  void DispatchCompositionEndEvent(const std::string& soFarCompositiedString);
  void RegisterOnShowSoftwareKeyboardIfPossibleHandler(
      const std::function<void(WebContainer*)>& cb);
  void RegisterOnHideSoftwareKeyboardIfPossibleHandler(
      const std::function<void(WebContainer*)>& cb);

  void SetUserData(const std::string& key, void* data);
  void* GetUserData(const std::string& key);

  std::string GetTitle();
  void ScrollTo(int x, int y);
  void ScrollBy(int x, int y);
  int GetScrollX();
  int GetScrollY();

  size_t Width();
  size_t Height();

  // You can control rendering flow through this function
  // If you got callback, you must call `doRenderingFunction` after
  void RegisterSetNeedsRenderingCallback(
      const std::function<void(WebContainer*, const std::function<void()>&
                                                  doRenderingFunction)>& cb);

 protected:
  WebContainer(void* webView);

 private:
  void* m_impl;
};

class LWE_EXPORT WebView {
 protected:
  // use Destroy function instead of using delete operator
  virtual ~WebView() {}

 public:
  static WebView* Create(void* win, unsigned x, unsigned y, unsigned width,
                         unsigned height, float devicePixelRatio,
                         const char* defaultFontName, const char* locale,
                         const char* timezoneID);

  virtual void Destroy();

  Settings GetSettings();
  virtual void LoadURL(const std::string& url);
  std::string GetURL();
  void LoadData(const std::string& data);
  void Reload();
  void StopLoading();
  void GoBack();
  void GoForward();
  bool CanGoBack();
  bool CanGoForward();
  void Pause();
  void Resume();
  void AddJavaScriptInterface(
      const std::string& exposedObjectName, const std::string& jsFunctionName,
      std::function<std::string(const std::string&)> cb);
  std::string EvaluateJavaScript(const std::string& script);
  void EvaluateJavaScript(const std::string& script,
                          std::function<void(const std::string&)> cb);
  void ClearHistory();
  void SetSettings(const Settings& settings);
  void RemoveJavascriptInterface(const std::string& exposedObjectName,
                                 const std::string& jsFunctionName);
  void ClearCache();
  void RegisterOnReceivedErrorHandler(
      std::function<void(WebView*, ResourceError)> cb);
  void RegisterOnPageParsedHandler(
      std::function<void(WebView*, const std::string&)> cb);
  void RegisterOnPageLoadedHandler(
      std::function<void(WebView*, const std::string&)> cb);
  void RegisterOnPageStartedHandler(
      std::function<void(WebView*, const std::string&)> cb);
  void RegisterOnLoadResourceHandler(
      std::function<void(WebView*, const std::string&)> cb);

  void RegisterCustomFileResourceRequestHandlers(
      std::function<const char*(const char* path)> resolveFilePathCallback,
      std::function<void*(const char* path)> fileOpenCallback,
      std::function<size_t(uint8_t* destBuffer, size_t size, void* handle)>
          fileReadCallback,
      std::function<long int(void* handle)> fileLengthCallback,
      std::function<void(void* handle)> fileCloseCallback);

  void RegisterDebuggerShouldInitHandler(
      const std::function<void(const std::string& url, int port,
                               bool& shouldInit)>& cb);
  void RegisterDebuggerShouldContinueWaitingHandler(
      const std::function<void(const std::string& url, int port,
                               bool& shouldWait)>& cb);

  void SetUserData(const std::string& key, void* data);
  void* GetUserData(const std::string& key);

  std::string GetTitle();
  void ScrollTo(int x, int y);
  void ScrollBy(int x, int y);
  int GetScrollX();
  int GetScrollY();

  virtual void* Unwrap() {
    // Some platform returns associated native handle ex) Evas_Object*
    return nullptr;
  }
  virtual void Focus();
  virtual void Blur();

 protected:
  WebView(void* impl) : m_impl(impl) {}

  virtual WebContainer* FetchWebContainer() = 0;

  void* m_impl;
};

}  // namespace LWE

#endif
