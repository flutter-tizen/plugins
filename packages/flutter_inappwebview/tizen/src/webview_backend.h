// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WEBVIEW_BACKEND_H_
#define FLUTTER_PLUGIN_WEBVIEW_BACKEND_H_

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class BufferPool;

class WebViewBackend {
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    virtual void OnFrameRendered(void* tbm_surface) = 0;
    virtual void OnLoadStarted(const std::string& url) = 0;
    virtual void OnLoadFinished(const std::string& url) = 0;
    virtual void OnProgress(int32_t progress) = 0;
    virtual void OnLoadError(int32_t error_code, const std::string& description,
                             const std::string& failing_url) = 0;
    virtual void OnConsoleMessage(const std::string& level,
                                  const std::string& message) = 0;
    // NOTE: Capture current_url before accepting; it may emit "url,changed".
    virtual void OnNavigationPolicyDecide(const std::string& url,
                                          const std::string& current_url) = 0;
    virtual void OnUrlChanged(const std::string& url) = 0;
    virtual void OnTitleChanged(const std::string& title) = 0;
    virtual void OnJavaScriptAlertDialog(const std::string& message,
                                         const std::string& url) = 0;
    virtual void OnJavaScriptConfirmDialog(const std::string& message,
                                           const std::string& url) = 0;
    virtual void OnJavaScriptPromptDialog(const std::string& message,
                                          const std::string& default_text,
                                          const std::string& url) = 0;
  };

  virtual ~WebViewBackend() {}

  virtual bool Create(double width, double height, void* window) = 0;

  virtual std::function<void()> PrepareTeardown(
      std::shared_ptr<BufferPool> pool) = 0;

  virtual void Offset(double left, double top) = 0;
  virtual void Resize(double width, double height) = 0;
  virtual void Touch(int type, int button, double x, double y, double dx,
                     double dy) = 0;
  virtual bool SendKey(const char* key, const char* string, const char* compose,
                       uint32_t modifiers, uint32_t scan_code,
                       bool is_down) = 0;
  virtual void Resume() = 0;
  virtual void Stop() = 0;

  virtual void SetJavaScriptEnabled(bool enabled) = 0;

  virtual bool LoadUrl(const std::string& url) = 0;
  // NOTE: method is 0 for GET and 1 for POST; body has no trailing NUL.
  virtual bool LoadUrlRequest(const std::string& url, int32_t method,
                              const std::map<std::string, std::string>& headers,
                              const std::vector<uint8_t>& body) = 0;
  virtual bool LoadHtmlString(const std::string& html,
                              const std::string& base_url) = 0;
  virtual bool CanGoBack() = 0;
  virtual bool CanGoForward() = 0;
  virtual bool GoBack() = 0;
  virtual bool GoForward() = 0;
  virtual bool Reload() = 0;
  virtual std::string GetCurrentUrl() = 0;
  virtual void EvaluateJavaScript(
      const std::string& javascript,
      std::function<void(bool success, const char* result_value)> callback) = 0;
  virtual void ClearCache() = 0;
  virtual std::string GetTitle() = 0;
  virtual void ScrollTo(int32_t x, int32_t y) = 0;
  virtual void GetScrollPosition(int32_t* x, int32_t* y) = 0;
  virtual void SetBackgroundColor(int r, int g, int b, int a) = 0;
  virtual void SetUserAgent(const std::string& user_agent) = 0;
  virtual std::string GetUserAgent() = 0;
  virtual void EnableZoom(bool enabled) = 0;
  virtual void JavaScriptAlertReply() = 0;
  virtual void JavaScriptConfirmReply(bool result) = 0;
  // NOTE: nullptr cancels the prompt; an empty string confirms an empty value.
  virtual void JavaScriptPromptReply(const char* result) = 0;
  virtual bool ClearCookies() = 0;

  virtual void Suspend() = 0;
  virtual int32_t GetProgress() = 0;
  virtual double GetScale() = 0;
  virtual void SetScale(double scale, int32_t x, int32_t y) = 0;

 protected:
  static std::function<void()> RegisterPendingTeardown(
      std::shared_ptr<BufferPool> pool, std::function<void()> destroy);

  static void FlushPendingTeardowns();
};

#endif  // FLUTTER_PLUGIN_WEBVIEW_BACKEND_H_
