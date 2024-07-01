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
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "PlatformIntegrationData.h"

using LWEDelegateRef = std::unique_ptr<void, std::function<void(void*)>>;

namespace LWE {

/**
 * \brief Perform initialization or cleanup of lightweight web engine.
 */
class LWE_EXPORT LWE {
 public:
  /**
   * \brief Initialize a lightweight web engine.
   * It performs tasks (thread initialization, GC preparation) necessary for
   * the operation of a lightweight web engine. You must call Initialize
   * function before using WebContainer or WebView
   *
   * \code{.cpp}
   *     LWE::LWE::Initialize("/tmp/Starfish_localStorage.txt",
   *                    "/tmp/Starfish_Cookies.txt", "/tmp/Starfish-cache");
   * \endcode
   *
   * \param localStorageDataFilePath File path for local storage.
   * \param cookieStoreDataFilePath File path for cookie storage.
   * \param httpCacheDataDirectorypath Directory path for http cache.
   *
   */
  static void Initialize(const char* localStorageDataFilePath,
                         const char* cookieStoreDataFilePath,
                         const char* httpCacheDataDirectorypath);

  /**
   * \brief Returns the initialization status of lightweight web engine.
   * Before performing an initialization, this function can be used to check
   * if an initialization has already been performed.
   *
   * \return Initialization status of lightweight web engine.
   */
  static bool IsInitialized();

  /**
   * \brief Perform lightweight web engine cleanup.
   * Called once when the lightweight web engine is no longer in use.
   *
   * \code{.cpp}
   *     LWE::LWE::Finalize();
   * \endcode
   *
   */
  static void Finalize();

  /**
   * \brief Returns GC frequency. default value is 12
   * If you want to cause more GC while running,
   * you can increase this value.
   */
  static unsigned char GetGCFrequency();

  /**
   * \brief Set GC frequency. default value is 12
   * If you want to cause more GC while running,
   * you can increase this value.
   */
  static void SetGCFrequency(unsigned char freq);

  /**
   * \brief Returns LWE version number if the parameter is not null.
   */
  static void GetVersion(int* major, int* minor, int* patch);
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

  CookieManager(const CookieManager& other) = delete;
  CookieManager(CookieManager&& other) = delete;
  CookieManager& operator=(const CookieManager& other) = delete;

 private:
  CookieManager();
  ~CookieManager();

  LWEDelegateRef m_delegate;
};

class LWE_EXPORT Settings {
 public:
  Settings();
  Settings(const std::string& defaultUA, const std::string& ua);
  ~Settings();
  Settings(const Settings& other);

  bool UpdateSetting(const std::string& key, const std::string& value);
  std::string GetSetting(std::string key) const;

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
  bool UseSpatialNavigation() const;

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
  void SetUseSpatialNavigation(bool useSpatialNavigation);

  void IterateSettings(
      std::function<void(const std::string&, const std::string&)> callback)
      const;

 private:
  LWEDelegateRef m_delegate;
};

class LWE_EXPORT ResourceError {
 public:
  ResourceError(int code, const std::string& description,
                const std::string& url);
  ResourceError(const ResourceError& other);
  ~ResourceError();

  int GetErrorCode() const;
  std::string GetDescription() const;
  std::string GetUrl() const;

 private:
  LWEDelegateRef m_delegate;
};

class LWE_EXPORT WebContainer {
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

  struct WebContainerArguments {
    unsigned width;
    unsigned height;
    float devicePixelRatio;
    const char* defaultFontName;
    const char* locale;
    const char* timezoneID;
  };

  struct RendererGLConfiguration {
    std::function<void(WebContainer*)> onMakeCurrent;
    std::function<void(WebContainer*, bool mayNeedsSync)> onSwapBuffers;
    std::function<uintptr_t(WebContainer*)> onCreateSharedContext;
    std::function<bool(WebContainer*, uintptr_t)> onDestroyContext;
    std::function<bool(WebContainer*)> onClearCurrentContext;
    std::function<bool(WebContainer*, uintptr_t)> onMakeCurrentWithContext;
    std::function<void*(WebContainer*, const char*)> onGetProcAddress;
    std::function<bool(WebContainer*, const char*)> onIsSupportedExtension;
  };

  struct TransformationMatrix {
    double scaleX;
    double skewX;
    double translateX;
    double skewY;
    double scaleY;
    double translateY;
    double perspectiveX;
    double perspectiveY;
    double perspectiveScale;
  };

  void RegisterPreRenderingHandler(const std::function<RenderInfo(void)>& cb);
  void RegisterOnRenderedHandler(
      const std::function<void(WebContainer*,
                               const RenderResult& renderResult)>& cb);

  static WebContainer* CreateWithPlatformImage(
      const WebContainerArguments& args,
      const std::function<ExternalImageInfo(void)>& prepareImageCb,
      const std::function<void(WebContainer*, bool needsFlush)>& flushCb);
  // <--- end of function set for render to buffer

  // Function set for render with OpenGL
  static WebContainer* CreateGL(const WebContainerArguments& args,
                                const RendererGLConfiguration& config);

  static WebContainer* CreateGLWithPlatformImage(
      const WebContainerArguments& args, const RendererGLConfiguration& config,
      const std::function<ExternalImageInfo(void)>& prepareImageCb,
      const std::function<void(WebContainer*, bool needsFlush)>& flushCb);

  static WebContainer* CreateWebContainer(void* delegate);

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
  void RegisterOnIdleHandler(const std::function<void(WebContainer*)>& cb);

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
  void SetDevicePixelRatio(float dpr);
  float GetDevicePixelRatio();

  void RegisterGetScreenMatrixHandler(
      const std::function<TransformationMatrix(WebContainer*)>& cb);

 private:
  WebContainer();

  // use Destroy function instead of using delete operator
  ~WebContainer();

  LWEDelegateRef m_delegate;
};

/**
 * \brief WebView of lightweight web engine.
 */
class LWE_EXPORT WebView {
 public:
  /**
   * \brief Create a Webview instance.
   * The webview instance should be obtained through this function, not
   * separately.
   *
   * \code{.cpp}
   *
   * LWE::WebView* webView = LWE::WebView::Create(wndObj, 0, 0, 800, 600, 1.0,
   * "serif", "ko-KR", "Asia/Seoul");
   *
   * \endcode
   *
   * \param win Window object where the content will be rendered. It accepts
   * different window objects depending on which platform the LWE was compiled
   * for. For EFL, Pass an Win(Elementary Widget) object.
   *
   * \param x The value of x among the initial positions of the webview.
   *
   * \param y The value of y among the initial positions of the webview.
   *
   * \param width The width value of the webview.
   *
   * \param height The height value of the webview.
   *
   * \param devicePixelRatio The device pixel ratio value. Width, height
   * divided by this value is used as the logical width, height for web
   * content. This means it can be utilized like a scale value.
   *
   * \param defaultFontName The font value, which will try to match this font
   * first if there is not given font family.
   *
   * \param locale Default locale used by the javascript engine.
   *
   * \param timezoneID Default time zone used by the javascript engine.
   *
   * \return new webview instance.
   */
  static WebView* Create(void* win, unsigned x, unsigned y, unsigned width,
                         unsigned height, float devicePixelRatio,
                         const char* defaultFontName, const char* locale,
                         const char* timezoneID);

  /**
   * \brief Destory a webview instance.
   * Use Destroy function instead of using delete operator
   *
   * \code{.cpp}
   *
   * webView->Destroy();
   *
   * \endcode
   */
  void Destroy();

  /**
   * \brief Gets the settings used by the webview.
   *
   * \return Current webview's setting value.
   */
  Settings GetSettings();

  /**
   * \brief Loads the given URL.
   *
   * \code{.cpp}
   *
   * webView->LoadURL("https://www.w3.org/");
   *
   * \endcode
   *
   * \param url the URL of the resource to load.
   *
   */
  void LoadURL(const std::string& url);

  /**
   * \brief Gets the URL for the current page.
   *
   * \code{.cpp}
   *
   * std::string currentURL = webView->GetURL();
   *
   * \endcode
   *
   * \return The URL for the current page.
   */
  std::string GetURL();

  /**
   * \brief Loads the given data into this WebView using a 'data' scheme URL.
   *
   * \code{.cpp}
   *
   * webView->LoadData("<html><body>TEST!</body></html>");
   *
   * \endcode
   *
   * \param data String of data.
   *
   */
  void LoadData(const std::string& data);

  /**
   * \brief Reloads the current URL.
   *
   * \code{.cpp}
   *
   * webView->Reload();
   *
   * \endcode
   *
   */
  void Reload();

  /**
   * \brief Stops the current load.
   *
   * \code{.cpp}
   *
   * webView->StopLoading();
   *
   * \endcode
   *
   */
  void StopLoading();

  /**
   * \brief Goes back in the history of this WebView.
   *
   * \code{.cpp}
   *
   * webView->GoBack();
   *
   * \endcode
   *
   */
  void GoBack();

  /**
   * \brief Goes forward in the history of this WebView.
   *
   * \code{.cpp}
   *
   * webView->GoForward();
   *
   * \endcode
   *
   */
  void GoForward();

  /**
   * \brief Gets whether this WebView has a back history item.
   *
   * \code{.cpp}
   *
   * if(webView->CanGoBack())
   * {
   *      webView->GoBack();
   * }
   *
   * \endcode
   *
   * \return true if this WebView has a back history item.
   */
  bool CanGoBack();

  /**
   * \brief Gets whether this WebView has a forward history item.
   *
   * \code{.cpp}
   *
   * if(webView->CanGoForward())
   * {
   *      webView->GoForward();
   * }
   *
   * \endcode
   *
   * \return true if this WebView has a forward history item.
   */
  bool CanGoForward();

  /**
   * \brief Change the visibilityState property of the Document to hidden.
   * This has the effect of stopping the rendering of web contents.
   *
   */
  void Pause();

  /**
   * \brief Change the visibilityState property of the Document to visible.
   * This has the effect of resuming the rendering of web contents.
   *
   */
  void Resume();

  /**
   * \brief Injects the supplied native callback into this webview.
   * ....
   *
   * \code{.cpp}
   *
   * webView->AddJavaScriptInterface("TEST", "set", [](std::string param) ->
   * std::string { return ""; });
   *
   * \endcode
   *
   * \param exposedObjectName Global object names exposed in javascript.
   *
   * \param jsFunctionName Function name to call in javascript.
   *
   * \param cb Native callback which wants to be called from javascript side.
   *
   */
  void AddJavaScriptInterface(
      const std::string& exposedObjectName, const std::string& jsFunctionName,
      std::function<std::string(const std::string&)> cb);

  /**
   * \brief Synchronously evaluates JavaScript in the context of the currently
   * displayed page.
   *
   * \code{.cpp}
   *
   * std::string result = webView->EvaluateJavaScript("1+1");
   * printf("%s\n",result.c_str());
   *
   * \endcode
   *
   * \param script Javascript string to execute.
   *
   * \return The result of running the javascript(only supported string).
   */
  std::string EvaluateJavaScript(const std::string& script);

  /**
   * \brief Asynchronously evaluates JavaScript in the context of the
   * currently displayed page.
   * ....
   *
   * \code{.cpp}
   *
   * webView->EvaluateJavaScript("1+1",[](const std::string& result) -> void{
   * printf("%s\n",result.c_str()); });
   *
   * \endcode
   *
   * \param script Javascript string to execute.
   *
   * \param cb A callback that is called at the end of performing javascript .
   *
   */
  void EvaluateJavaScript(const std::string& script,
                          std::function<void(const std::string&)> cb);

  /**
   * \brief Tells this webview to clear its internal back/forward list.
   *
   * \code{.cpp}
   *
   * webView->ClearHistory();
   *
   * \endcode
   *
   */
  void ClearHistory();

  /**
   * \brief Set the settings for current webview.
   *
   * \param settings Settings value for current webview.
   *
   */
  void SetSettings(const Settings& settings);

  /**
   * \brief Removes a previously injected native callback from this webview.
   *
   * \code{.cpp}
   *
   * webView->RemoveJavascriptInterface("TEST", "set");
   *
   * \endcode
   *
   * \param exposedObjectName Global object names exposed in javascript.
   *
   * \param jsFunctionName Function name to call in javascript.
   *
   */
  void RemoveJavascriptInterface(const std::string& exposedObjectName,
                                 const std::string& jsFunctionName);

  /**
   * \brief Clears the resource cache.
   *
   * \code{.cpp}
   *
   * webView->ClearCache();
   *
   * \endcode
   *
   */
  void ClearCache();

  /**
   * \brief Register callbacks that will be used to handle web resource
   * loading error.
   *
   * \code{.cpp}
   *
   * webView->RegisterOnReceivedErrorHandler(
   *  [](LWE::WebView* webview, LWE::ResourceError err) -> void {
   *      printf("Error URL : %s\n", err.GetUrl().c_str());
   *      printf("Error Code : %d\n", err.GetErrorCode());
   *      printf("Error Description : %s\n", err.GetDescription().c_str());
   *   }
   *  );
   *
   * \endcode
   *
   * \param cb error handling callback.
   *
   */
  void RegisterOnReceivedErrorHandler(
      std::function<void(WebView*, ResourceError)> cb);

  /**
   * \brief Register a callback to be called when the DOMContentLoaded event
   * occurs.
   *
   * \code{.cpp}
   *
   * webView->RegisterOnPageParsedHandler(
   *  [](LWE::WebView* webview, const std::string& url) -> void {
   *      printf("URL : %s\n", url.c_str());
   *   }
   *  );
   *
   * \endcode
   *
   * \param cb Event handling callback.
   *
   */
  void RegisterOnPageParsedHandler(
      std::function<void(WebView*, const std::string&)> cb);

  /**
   * \brief Register a callback to be called when the document load event
   * occurs.
   *
   * \code{.cpp}
   *
   * webView->RegisterOnPageLoadedHandler(
   *  [](LWE::WebView* webview, const std::string& url) -> void {
   *      printf("URL : %s\n", url.c_str());
   *   }
   *  );
   *
   * \endcode
   *
   * \param cb Event handling callback.
   *
   */
  void RegisterOnPageLoadedHandler(
      std::function<void(WebView*, const std::string&)> cb);

  /**
   * \brief Register a callback to be called when url navigation start.
   *
   * \code{.cpp}
   *
   * webView->RegisterOnPageStartedHandler(
   *  [](LWE::WebView* webview, const std::string& url) -> void {
   *      printf("URL : %s\n", url.c_str());
   *   }
   *  );
   *
   * \endcode
   *
   * \param cb Event handling callback.
   *
   */
  void RegisterOnPageStartedHandler(
      std::function<void(WebView*, const std::string&)> cb);

  /**
   * \brief Register a callback to be called at the end of loading individual
   * resources.
   *
   * \code{.cpp}
   *
   * webView->RegisterOnLoadResourceHandler(
   *  [](LWE::WebView* webview, const std::string& url) -> void {
   *      printf("URL : %s\n", url.c_str());
   *   }
   *  );
   *
   * \endcode
   *
   * \param cb Event handling callback.
   *
   */
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

  /**
   * \brief Storing separate user data(pointers) in webview object.
   *
   * \param key User data key.
   *
   * \param data User data pointer.
   *
   */
  void SetUserData(const std::string& key, void* data);

  /**
   * \brief Getting user data(pointers) from webview object.
   *
   * \param key User data key.
   *
   * \return User data pointer.
   *
   */
  void* GetUserData(const std::string& key);

  /**
   * \brief Get the document's title value.
   *
   * \return document's title value.
   */
  std::string GetTitle();

  /**
   * \brief Scrolls to a absolute position in the current document.
   *
   * \param x The x-coordinate value of the position that user want to scroll.
   *
   * \param y The x-coordinate value of the position that user want to scroll.
   *
   */
  void ScrollTo(int x, int y);

  /**
   * \brief Scrolls to a relative position in the current document.
   *
   * \param x The x-coordinate value of the position that user want to scroll.
   *
   * \param y The x-coordinate value of the position that user want to scroll.
   *
   */
  void ScrollBy(int x, int y);

  /**
   * \brief Get x-coordinate of the scrolled position for the current
   * document.
   *
   * \return The x-coordinate value of position.
   *
   */
  int GetScrollX();

  /**
   * \brief Get y-coordinate of the scrolled position for the current
   * document.
   *
   * \return The y-coordinate value of position.
   *
   */
  int GetScrollY();

  /**
   * \brief Get platform native handle.
   * This is intended for specialized use. It is platform dependent. For
   * example, For EFL ports, the object you get by this API is an Evas_Object
   * handle. You can use EFL API to resize, show, hide with this handle.
   *
   * \return platform native handle.
   */
  void* Unwrap();

  /**
   * \brief Give focus to current webview.
   *
   */
  void Focus();

  /**
   * \brief Blur the current webview.
   *
   */
  void Blur();

  /**
   * \brief Change DPR value at current webview.
   *
   * \param dpr Device pixel ratio.
   *
   */
  void SetDevicePixelRatio(float dpr);

  /**
   * \brief DPR value of current webview.
   *
   * \return Device pixel ratio.
   *
   */
  float GetDevicePixelRatio();

 private:
  WebView();
  ~WebView();

  LWEDelegateRef m_delegate;
};

}  // namespace LWE

#endif
