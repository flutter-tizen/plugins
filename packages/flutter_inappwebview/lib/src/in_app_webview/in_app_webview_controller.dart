// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_dynamic_calls

import 'dart:convert';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:flutter_inappwebview_platform_interface/flutter_inappwebview_platform_interface.dart';

import '_static_channel.dart';

/// Object specifying creation parameters for creating a [TizenInAppWebViewController].
///
/// When adding additional fields make sure they can be null or have a default
/// value to avoid breaking changes. See [PlatformInAppWebViewControllerCreationParams] for
/// more information.
@immutable
class TizenInAppWebViewControllerCreationParams
    extends PlatformInAppWebViewControllerCreationParams {
  /// Creates a new [TizenInAppWebViewControllerCreationParams] instance.
  const TizenInAppWebViewControllerCreationParams({
    required super.id,
    super.webviewParams,
  });

  /// Creates a [TizenInAppWebViewControllerCreationParams] instance based on [PlatformInAppWebViewControllerCreationParams].
  factory TizenInAppWebViewControllerCreationParams.fromPlatformInAppWebViewControllerCreationParams(
    // Recommended placeholder to prevent being broken by platform interface.
    // ignore: avoid_unused_constructor_parameters
    PlatformInAppWebViewControllerCreationParams params,
  ) {
    return TizenInAppWebViewControllerCreationParams(
      id: params.id,
      webviewParams: params.webviewParams,
    );
  }
}

/// Error returned in `InAppWebView.onReceivedError` when a Tizen web resource
/// loading error occurs.
class TizenWebResourceError extends WebResourceError {
  /// Creates a new [TizenWebResourceError].
  TizenWebResourceError._({required this.errorCode, required super.description})
    : super(type: _errorCodeToErrorType(errorCode));

  /// Unknown error.
  static const int unknown = 0;

  /// Failed to file I/O.
  static const int failedFileIO = 3;

  /// Cannot connect to Network.
  static const int cantConnect = 4;

  /// Fail to look up host from DNS.
  static const int cantHostLookup = 5;

  /// Fail to SSL/TLS handshake.
  static const int failedSslHandshake = 6;

  /// Connection timeout.
  static const int requestTimeout = 8;

  /// Too many redirects.
  static const int tooManyRedirect = 9;

  /// Too many requests during this load.
  static const int tooManyRequests = 10;

  /// Malformed url.
  static const int badUrl = 11;

  /// Unsupported scheme.
  static const int unsupportedScheme = 12;

  /// User authentication failed on server.
  static const int authenticationFailed = 13;

  /// Raw EWK error code.
  final int errorCode;

  static WebResourceErrorType _errorCodeToErrorType(int errorCode) {
    switch (errorCode) {
      case unknown:
        return WebResourceErrorType.UNKNOWN;
      case failedFileIO:
        return WebResourceErrorType.GENERIC_FILE_ERROR;
      case cantConnect:
        return WebResourceErrorType.CANNOT_CONNECT_TO_HOST;
      case cantHostLookup:
        return WebResourceErrorType.HOST_LOOKUP;
      case failedSslHandshake:
        return WebResourceErrorType.FAILED_SSL_HANDSHAKE;
      case requestTimeout:
        return WebResourceErrorType.TIMEOUT;
      case tooManyRedirect:
        return WebResourceErrorType.TOO_MANY_REDIRECTS;
      case tooManyRequests:
        return WebResourceErrorType.TOO_MANY_REQUESTS;
      case badUrl:
        return WebResourceErrorType.BAD_URL;
      case unsupportedScheme:
        return WebResourceErrorType.UNSUPPORTED_SCHEME;
      case authenticationFailed:
        return WebResourceErrorType.USER_AUTHENTICATION_FAILED;
    }

    return WebResourceErrorType.UNKNOWN;
  }
}

///Controls an [InAppWebView] widget instance.
///
///If you are using the [InAppWebView] widget, an [InAppWebViewController] instance can be obtained by setting the [InAppWebView.onWebViewCreated]
///callback.
class TizenInAppWebViewController extends PlatformInAppWebViewController
    with ChannelController {
  /// Creates a new [TizenInAppWebViewController].
  TizenInAppWebViewController(
    PlatformInAppWebViewControllerCreationParams params,
  ) : super.implementation(
        params is TizenInAppWebViewControllerCreationParams
            ? params
            : TizenInAppWebViewControllerCreationParams.fromPlatformInAppWebViewControllerCreationParams(
                params,
              ),
      ) {
    channel = MethodChannel('com.pichillilorenzo/flutter_inappwebview_$id');
    handler = handleMethod;
    initMethodCallHandler();

    _init(params);
  }

  /// Returns the lazily-constructed singleton used for static channel calls.
  factory TizenInAppWebViewController.static() {
    return _staticValue;
  }
  static const MethodChannel _staticChannel = IN_APP_WEBVIEW_STATIC_CHANNEL;

  dynamic _controllerFromPlatform;

  static final TizenInAppWebViewController _staticValue =
      TizenInAppWebViewController(
        const TizenInAppWebViewControllerCreationParams(id: null),
      );

  void _init(PlatformInAppWebViewControllerCreationParams params) {
    _controllerFromPlatform =
        params.webviewParams?.controllerFromPlatform?.call(this) ?? this;
  }

  void _debugLog(String method, dynamic args) {
    debugLog(
      className: 'TizenInAppWebViewController',
      name: 'WebView',
      id: getViewId().toString(),
      debugLoggingSettings: PlatformInAppWebViewController.debugLoggingSettings,
      method: method,
      args: args,
    );
  }

  Never _unsupported(String method) {
    throw UnsupportedError(
      '$method is not implemented on flutter_inappwebview_tizen.',
    );
  }

  WebResourceError? _parseWebResourceError(Map<String, dynamic>? errorMap) {
    if (errorMap == null) {
      return null;
    }

    final String description =
        errorMap['description']?.toString() ?? 'Unknown error';
    final dynamic nativeType = errorMap['type'];
    final dynamic nativeErrorCode = errorMap['errorCode'] ?? nativeType;
    final int? errorCode = nativeErrorCode is int ? nativeErrorCode : null;
    if (errorCode != null) {
      return TizenWebResourceError._(
        errorCode: errorCode,
        description: description,
      );
    }

    final WebResourceErrorType type =
        WebResourceErrorType.fromNativeValue(
          nativeType is int ? nativeType : null,
        ) ??
        WebResourceErrorType.UNKNOWN;
    return WebResourceError(description: description, type: type);
  }

  Future<dynamic> _handleMethod(MethodCall call) async {
    if (PlatformInAppWebViewController.debugLoggingSettings.enabled) {
      _debugLog(call.method, call.arguments);
    }

    switch (call.method) {
      case 'onLoadStart':
        if (webviewParams != null && webviewParams!.onLoadStart != null) {
          final String? url = call.arguments['url'];
          webviewParams!.onLoadStart!(
            _controllerFromPlatform,
            url != null ? WebUri(url) : null,
          );
        }
      case 'onLoadStop':
        if (webviewParams != null && webviewParams!.onLoadStop != null) {
          final String? url = call.arguments['url'];
          webviewParams!.onLoadStop!(
            _controllerFromPlatform,
            url != null ? WebUri(url) : null,
          );
        }
      case 'onReceivedError':
        if (webviewParams != null &&
            (webviewParams!.onReceivedError != null ||
                // ignore: deprecated_member_use_from_same_package
                webviewParams!.onLoadError != null)) {
          final Map<String, dynamic>? requestMap = call.arguments['request']
              ?.cast<String, dynamic>();
          final Map<String, dynamic>? errorMap = call.arguments['error']
              ?.cast<String, dynamic>();
          if (requestMap == null || errorMap == null) {
            break;
          }
          final WebResourceRequest? request = WebResourceRequest.fromMap(
            requestMap,
          );
          final WebResourceError? error = _parseWebResourceError(errorMap);
          if (request == null || error == null) {
            break;
          }
          final bool isForMainFrame = request.isForMainFrame ?? false;
          if (webviewParams!.onReceivedError != null) {
            webviewParams!.onReceivedError!(
              _controllerFromPlatform,
              request,
              error,
            );
          } else if (isForMainFrame) {
            final int errorCode = error is TizenWebResourceError
                ? error.errorCode
                : error.type.toNativeValue() ?? -1;
            // ignore: deprecated_member_use_from_same_package
            webviewParams!.onLoadError!(
              _controllerFromPlatform,
              request.url,
              errorCode,
              error.description,
            );
          }
        }
      case 'onProgressChanged':
        if (webviewParams != null && webviewParams!.onProgressChanged != null) {
          final int progress = call.arguments['progress'];
          webviewParams!.onProgressChanged!(_controllerFromPlatform, progress);
        }
      case 'shouldOverrideUrlLoading':
        if (webviewParams != null &&
            webviewParams!.shouldOverrideUrlLoading != null) {
          final NavigationAction navigationAction = NavigationAction.fromMap(
            call.arguments.cast<String, dynamic>(),
          )!;
          return (await webviewParams!.shouldOverrideUrlLoading!(
            _controllerFromPlatform,
            navigationAction,
          ))?.toNativeValue();
        }
      case 'onConsoleMessage':
        if (webviewParams != null && webviewParams!.onConsoleMessage != null) {
          final ConsoleMessage consoleMessage = ConsoleMessage.fromMap(
            call.arguments.cast<String, dynamic>(),
          )!;
          webviewParams!.onConsoleMessage!(
            _controllerFromPlatform,
            consoleMessage,
          );
        }
      case 'onScrollChanged':
        if (webviewParams != null && webviewParams!.onScrollChanged != null) {
          final int x = call.arguments['x'];
          final int y = call.arguments['y'];
          webviewParams!.onScrollChanged!(_controllerFromPlatform, x, y);
        }
      case 'onTitleChanged':
        if (webviewParams != null && webviewParams!.onTitleChanged != null) {
          final String? title = call.arguments['title'];
          webviewParams!.onTitleChanged!(_controllerFromPlatform, title);
        }
      case 'onZoomScaleChanged':
        if (webviewParams != null &&
            webviewParams!.onZoomScaleChanged != null) {
          final double oldScale = call.arguments['oldScale'];
          final double newScale = call.arguments['newScale'];
          webviewParams!.onZoomScaleChanged!(
            _controllerFromPlatform,
            oldScale,
            newScale,
          );
        }
      case 'onJsAlert':
        final JsAlertRequest alertRequest = JsAlertRequest.fromMap(
          call.arguments.cast<String, dynamic>(),
        )!;
        if (webviewParams != null && webviewParams!.onJsAlert != null) {
          await webviewParams!.onJsAlert!(
            _controllerFromPlatform,
            alertRequest,
          );
        }
        // Always reply: EWK suspends JS execution until the alert is acknowledged.
        await channel?.invokeMethod<void>('javaScriptAlertReply');
      case 'onJsConfirm':
        final JsConfirmRequest confirmRequest = JsConfirmRequest.fromMap(
          call.arguments.cast<String, dynamic>(),
        )!;
        bool confirmed = true;
        if (webviewParams != null && webviewParams!.onJsConfirm != null) {
          final JsConfirmResponse? response = await webviewParams!.onJsConfirm!(
            _controllerFromPlatform,
            confirmRequest,
          );
          if (response?.action == JsConfirmResponseAction.CANCEL) {
            confirmed = false;
          }
        }
        await channel?.invokeMethod<void>('javaScriptConfirmReply', confirmed);
      case 'onJsPrompt':
        final JsPromptRequest promptRequest = JsPromptRequest.fromMap(
          call.arguments.cast<String, dynamic>(),
        )!;
        String? reply;
        if (webviewParams != null && webviewParams!.onJsPrompt != null) {
          final JsPromptResponse? response = await webviewParams!.onJsPrompt!(
            _controllerFromPlatform,
            promptRequest,
          );
          if (response != null &&
              response.action != JsPromptResponseAction.CANCEL) {
            reply = response.value ?? promptRequest.defaultValue ?? '';
          }
        } else {
          reply = promptRequest.defaultValue ?? '';
        }
        // A null reply signals that the prompt was cancelled.
        await channel?.invokeMethod<void>('javaScriptPromptReply', reply);
      case 'onUpdateVisitedHistory':
        if (webviewParams != null &&
            webviewParams!.onUpdateVisitedHistory != null) {
          final String? url = call.arguments['url'];
          final bool? isReload = call.arguments['isReload'];
          webviewParams!.onUpdateVisitedHistory!(
            _controllerFromPlatform,
            url != null ? WebUri(url) : null,
            isReload,
          );
        }
      default:
        throw UnimplementedError('Unimplemented ${call.method} method');
    }
    return null;
  }

  @override
  Future<WebUri?> getUrl() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    final String? url = await channel?.invokeMethod<String?>('getUrl', args);
    return url != null ? WebUri(url) : null;
  }

  @override
  Future<String?> getTitle() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    return await channel?.invokeMethod<String?>('getTitle', args);
  }

  @override
  Future<int?> getProgress() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    return await channel?.invokeMethod<int?>('getProgress', args);
  }

  @override
  Future<String?> getHtml() async {
    _unsupported('getHtml');
  }

  @override
  Future<List<Favicon>> getFavicons() async {
    _unsupported('getFavicons');
  }

  @override
  Future<void> loadUrl({
    required URLRequest urlRequest,
    @Deprecated('Use allowingReadAccessTo instead')
    Uri? iosAllowingReadAccessTo,
    WebUri? allowingReadAccessTo,
  }) async {
    assert(urlRequest.url != null && urlRequest.url.toString().isNotEmpty);
    assert(
      allowingReadAccessTo == null || allowingReadAccessTo.isScheme('file'),
    );
    assert(
      iosAllowingReadAccessTo == null ||
          iosAllowingReadAccessTo.isScheme('file'),
    );

    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('urlRequest', () => urlRequest.toMap());
    args.putIfAbsent(
      'allowingReadAccessTo',
      () =>
          allowingReadAccessTo?.toString() ??
          iosAllowingReadAccessTo?.toString(),
    );
    await channel?.invokeMethod('loadUrl', args);
  }

  @override
  Future<void> postUrl({
    required WebUri url,
    required Uint8List postData,
  }) async {
    assert(url.toString().isNotEmpty);
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('url', () => url.toString());
    args.putIfAbsent('postData', () => postData);
    await channel?.invokeMethod('postUrl', args);
  }

  @override
  Future<void> loadData({
    required String data,
    String mimeType = 'text/html',
    String encoding = 'utf8',
    WebUri? baseUrl,
    @Deprecated('Use historyUrl instead') Uri? androidHistoryUrl,
    WebUri? historyUrl,
    @Deprecated('Use allowingReadAccessTo instead')
    Uri? iosAllowingReadAccessTo,
    WebUri? allowingReadAccessTo,
  }) async {
    assert(
      allowingReadAccessTo == null || allowingReadAccessTo.isScheme('file'),
    );
    assert(
      iosAllowingReadAccessTo == null ||
          iosAllowingReadAccessTo.isScheme('file'),
    );

    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('data', () => data);
    args.putIfAbsent('mimeType', () => mimeType);
    args.putIfAbsent('encoding', () => encoding);
    args.putIfAbsent('baseUrl', () => baseUrl?.toString() ?? 'about:blank');
    args.putIfAbsent(
      'historyUrl',
      () =>
          historyUrl?.toString() ??
          androidHistoryUrl?.toString() ??
          'about:blank',
    );
    args.putIfAbsent(
      'allowingReadAccessTo',
      () =>
          allowingReadAccessTo?.toString() ??
          iosAllowingReadAccessTo?.toString(),
    );
    await channel?.invokeMethod('loadData', args);
  }

  @override
  Future<void> loadFile({required String assetFilePath}) async {
    assert(assetFilePath.isNotEmpty);
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('assetFilePath', () => assetFilePath);
    await channel?.invokeMethod('loadFile', args);
  }

  @override
  Future<void> reload() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    await channel?.invokeMethod('reload', args);
  }

  @override
  Future<void> goBack() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    await channel?.invokeMethod('goBack', args);
  }

  @override
  Future<bool> canGoBack() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    return await channel?.invokeMethod<bool>('canGoBack', args) ?? false;
  }

  @override
  Future<void> goForward() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    await channel?.invokeMethod('goForward', args);
  }

  @override
  Future<bool> canGoForward() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    return await channel?.invokeMethod<bool>('canGoForward', args) ?? false;
  }

  @override
  Future<void> goBackOrForward({required int steps}) async {
    _unsupported('goBackOrForward');
  }

  @override
  Future<bool> canGoBackOrForward({required int steps}) async {
    _unsupported('canGoBackOrForward');
  }

  @override
  Future<void> goTo({required WebHistoryItem historyItem}) async {
    _unsupported('goTo');
  }

  @override
  Future<bool> isLoading() async {
    _unsupported('isLoading');
  }

  @override
  Future<void> stopLoading() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    await channel?.invokeMethod('stopLoading', args);
  }

  @override
  Future<dynamic> evaluateJavascript({
    required String source,
    ContentWorld? contentWorld,
  }) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('source', () => source);
    args.putIfAbsent('contentWorld', () => contentWorld?.toMap());
    dynamic data = await channel?.invokeMethod('evaluateJavascript', args);
    if (data is String) {
      try {
        // Try to JSON-decode the value coming from JavaScript; if it isn't
        // valid JSON, return the raw string.
        data = json.decode(data);
      } on FormatException {
        // Not valid JSON; keep the original string.
      }
    }
    return data;
  }

  @override
  Future<void> injectJavascriptFileFromUrl({
    required WebUri urlFile,
    ScriptHtmlTagAttributes? scriptHtmlTagAttributes,
  }) async {
    _unsupported('injectJavascriptFileFromUrl');
  }

  @override
  Future<dynamic> injectJavascriptFileFromAsset({
    required String assetFilePath,
  }) async {
    _unsupported('injectJavascriptFileFromAsset');
  }

  @override
  Future<void> injectCSSCode({required String source}) async {
    _unsupported('injectCSSCode');
  }

  @override
  Future<void> injectCSSFileFromUrl({
    required WebUri urlFile,
    CSSLinkHtmlTagAttributes? cssLinkHtmlTagAttributes,
  }) async {
    _unsupported('injectCSSFileFromUrl');
  }

  @override
  Future<void> injectCSSFileFromAsset({required String assetFilePath}) async {
    _unsupported('injectCSSFileFromAsset');
  }

  @override
  void addJavaScriptHandler({
    required String handlerName,
    required JavaScriptHandlerCallback callback,
  }) {
    _unsupported('addJavaScriptHandler');
  }

  @override
  JavaScriptHandlerCallback? removeJavaScriptHandler({
    required String handlerName,
  }) {
    return null;
  }

  @override
  bool hasJavaScriptHandler({required String handlerName}) {
    return false;
  }

  @override
  Future<Uint8List?> takeScreenshot({
    ScreenshotConfiguration? screenshotConfiguration,
  }) async {
    _unsupported('takeScreenshot');
  }

  @override
  @Deprecated('Use setSettings instead')
  Future<void> setOptions({required InAppWebViewGroupOptions options}) async {
    final InAppWebViewSettings settings =
        InAppWebViewSettings.fromMap(options.toMap()) ?? InAppWebViewSettings();
    await setSettings(settings: settings);
  }

  @override
  @Deprecated('Use getSettings instead')
  Future<InAppWebViewGroupOptions?> getOptions() async {
    final InAppWebViewSettings? settings = await getSettings();

    Map<dynamic, dynamic>? options = settings?.toMap();
    if (options != null) {
      options = options.cast<String, dynamic>();
      return InAppWebViewGroupOptions.fromMap(options as Map<String, dynamic>);
    }

    return null;
  }

  @override
  Future<void> setSettings({required InAppWebViewSettings settings}) async {
    final Map<String, dynamic> args = <String, dynamic>{};

    args.putIfAbsent('settings', () => settings.toMap());
    await channel?.invokeMethod('setSettings', args);
  }

  @override
  Future<InAppWebViewSettings?> getSettings() async {
    _unsupported('getSettings');
  }

  @override
  Future<WebHistory?> getCopyBackForwardList() async {
    _unsupported('getCopyBackForwardList');
  }

  @override
  @Deprecated('Use InAppWebViewController.clearAllCache instead')
  Future<void> clearCache() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    await channel?.invokeMethod('clearCache', args);
  }

  @override
  @Deprecated('Use FindInteractionController.findAll instead')
  Future<void> findAllAsync({required String find}) async {
    _unsupported('findAllAsync');
  }

  @override
  @Deprecated('Use FindInteractionController.findNext instead')
  Future<void> findNext({required bool forward}) async {
    _unsupported('findNext');
  }

  @override
  @Deprecated('Use FindInteractionController.clearMatches instead')
  Future<void> clearMatches() async {
    _unsupported('clearMatches');
  }

  @override
  @Deprecated('Use tRexRunnerHtml instead')
  Future<String> getTRexRunnerHtml() async {
    return tRexRunnerHtml;
  }

  @override
  @Deprecated('Use tRexRunnerCss instead')
  Future<String> getTRexRunnerCss() async {
    return tRexRunnerCss;
  }

  @override
  Future<void> scrollTo({
    required int x,
    required int y,
    bool animated = false,
  }) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('x', () => x);
    args.putIfAbsent('y', () => y);
    args.putIfAbsent('animated', () => animated);
    await channel?.invokeMethod('scrollTo', args);
  }

  @override
  Future<void> scrollBy({
    required int x,
    required int y,
    bool animated = false,
  }) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('x', () => x);
    args.putIfAbsent('y', () => y);
    args.putIfAbsent('animated', () => animated);
    await channel?.invokeMethod('scrollBy', args);
  }

  @override
  Future<void> pauseTimers() async {
    _unsupported('pauseTimers');
  }

  @override
  Future<void> resumeTimers() async {
    _unsupported('resumeTimers');
  }

  @override
  Future<PlatformPrintJobController?> printCurrentPage({
    PrintJobSettings? settings,
  }) async {
    _unsupported('printCurrentPage');
  }

  @override
  Future<int?> getContentHeight() async {
    _unsupported('getContentHeight');
  }

  @override
  Future<int?> getContentWidth() async {
    _unsupported('getContentWidth');
  }

  @override
  Future<void> zoomBy({
    required double zoomFactor,
    @Deprecated('Use animated instead') bool? iosAnimated,
    bool animated = false,
  }) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('zoomFactor', () => zoomFactor);
    args.putIfAbsent('animated', () => iosAnimated ?? animated);
    return await channel?.invokeMethod('zoomBy', args);
  }

  @override
  Future<WebUri?> getOriginalUrl() async {
    _unsupported('getOriginalUrl');
  }

  @override
  @Deprecated('Use getZoomScale instead')
  Future<double?> getScale() async {
    return getZoomScale();
  }

  @override
  Future<String?> getSelectedText() async {
    _unsupported('getSelectedText');
  }

  @override
  Future<List<MetaTag>> getMetaTags() async {
    _unsupported('getMetaTags');
  }

  @override
  Future<Color?> getMetaThemeColor() async {
    _unsupported('getMetaThemeColor');
  }

  @override
  Future<int?> getScrollX() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    return await channel?.invokeMethod<int?>('getScrollX', args);
  }

  @override
  Future<int?> getScrollY() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    return await channel?.invokeMethod<int?>('getScrollY', args);
  }

  @override
  Future<SslCertificate?> getCertificate() async {
    _unsupported('getCertificate');
  }

  @override
  Future<void> addUserScript({required UserScript userScript}) async {
    _unsupported('addUserScript');
  }

  @override
  Future<void> addUserScripts({required List<UserScript> userScripts}) async {
    _unsupported('addUserScripts');
  }

  @override
  Future<bool> removeUserScript({required UserScript userScript}) async {
    _unsupported('removeUserScript');
  }

  @override
  Future<void> removeUserScriptsByGroupName({required String groupName}) async {
    _unsupported('removeUserScriptsByGroupName');
  }

  @override
  Future<void> removeUserScripts({
    required List<UserScript> userScripts,
  }) async {
    _unsupported('removeUserScripts');
  }

  @override
  Future<void> removeAllUserScripts() async {
    _unsupported('removeAllUserScripts');
  }

  @override
  bool hasUserScript({required UserScript userScript}) {
    return false;
  }

  @override
  Future<CallAsyncJavaScriptResult?> callAsyncJavaScript({
    required String functionBody,
    Map<String, dynamic> arguments = const <String, dynamic>{},
    ContentWorld? contentWorld,
  }) async {
    _unsupported('callAsyncJavaScript');
  }

  @override
  Future<String?> saveWebArchive({
    required String filePath,
    bool autoname = false,
  }) async {
    _unsupported('saveWebArchive');
  }

  @override
  Future<bool> isSecureContext() async {
    _unsupported('isSecureContext');
  }

  @override
  Future<PlatformWebMessageChannel?> createWebMessageChannel() async {
    _unsupported('createWebMessageChannel');
  }

  @override
  Future<void> postWebMessage({
    required WebMessage message,
    WebUri? targetOrigin,
  }) async {
    _unsupported('postWebMessage');
  }

  @override
  Future<void> addWebMessageListener(
    PlatformWebMessageListener webMessageListener,
  ) async {
    _unsupported('addWebMessageListener');
  }

  @override
  bool hasWebMessageListener(PlatformWebMessageListener webMessageListener) {
    return false;
  }

  @override
  Future<bool> canScrollVertically() async {
    _unsupported('canScrollVertically');
  }

  @override
  Future<bool> canScrollHorizontally() async {
    _unsupported('canScrollHorizontally');
  }

  @override
  Future<void> reloadFromOrigin() async {
    _unsupported('reloadFromOrigin');
  }

  @override
  Future<Uint8List?> createPdf({
    @Deprecated('Use pdfConfiguration instead')
    // ignore: deprecated_member_use_from_same_package
    IOSWKPDFConfiguration? iosWKPdfConfiguration,
    PDFConfiguration? pdfConfiguration,
  }) async {
    _unsupported('createPdf');
  }

  @override
  Future<Uint8List?> createWebArchiveData() async {
    _unsupported('createWebArchiveData');
  }

  @override
  Future<bool> hasOnlySecureContent() async {
    _unsupported('hasOnlySecureContent');
  }

  @override
  Future<void> pauseAllMediaPlayback() async {
    _unsupported('pauseAllMediaPlayback');
  }

  @override
  Future<void> setAllMediaPlaybackSuspended({required bool suspended}) async {
    _unsupported('setAllMediaPlaybackSuspended');
  }

  @override
  Future<void> closeAllMediaPresentations() async {
    _unsupported('closeAllMediaPresentations');
  }

  @override
  Future<MediaPlaybackState?> requestMediaPlaybackState() async {
    _unsupported('requestMediaPlaybackState');
  }

  @override
  Future<bool> isInFullscreen() async {
    _unsupported('isInFullscreen');
  }

  @override
  Future<MediaCaptureState?> getCameraCaptureState() async {
    _unsupported('getCameraCaptureState');
  }

  @override
  Future<void> setCameraCaptureState({required MediaCaptureState state}) async {
    _unsupported('setCameraCaptureState');
  }

  @override
  Future<MediaCaptureState?> getMicrophoneCaptureState() async {
    _unsupported('getMicrophoneCaptureState');
  }

  @override
  Future<void> setMicrophoneCaptureState({
    required MediaCaptureState state,
  }) async {
    _unsupported('setMicrophoneCaptureState');
  }

  @override
  Future<void> loadSimulatedRequest({
    required URLRequest urlRequest,
    required Uint8List data,
    URLResponse? urlResponse,
  }) async {
    _unsupported('loadSimulatedRequest');
  }

  @override
  Future<void> openDevTools() async {
    _unsupported('openDevTools');
  }

  @override
  Future<dynamic> callDevToolsProtocolMethod({
    required String methodName,
    Map<String, dynamic>? parameters,
  }) async {
    _unsupported('callDevToolsProtocolMethod');
  }

  @override
  Future<void> addDevToolsProtocolEventListener({
    required String eventName,
    required Function(dynamic data) callback,
  }) async {
    _unsupported('addDevToolsProtocolEventListener');
  }

  @override
  Future<void> removeDevToolsProtocolEventListener({
    required String eventName,
  }) async {
    _unsupported('removeDevToolsProtocolEventListener');
  }

  @override
  Future<void> pause() async {
    _unsupported('pause');
  }

  @override
  Future<void> resume() async {
    _unsupported('resume');
  }

  @override
  Future<String> getDefaultUserAgent() async {
    final Map<String, dynamic> args = <String, dynamic>{};
    return await _staticChannel.invokeMethod<String>(
          'getDefaultUserAgent',
          args,
        ) ??
        '';
  }

  @override
  Future<bool> handlesURLScheme(String urlScheme) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('urlScheme', () => urlScheme);
    return await _staticChannel.invokeMethod('handlesURLScheme', args);
  }

  @override
  Future<void> disposeKeepAlive(InAppWebViewKeepAlive keepAlive) async {
    _unsupported('disposeKeepAlive');
  }

  @override
  Future<void> clearAllCache({bool includeDiskFiles = true}) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args.putIfAbsent('includeDiskFiles', () => includeDiskFiles);
    await _staticChannel.invokeMethod('clearAllCache', args);
  }

  @override
  Future<String> get tRexRunnerHtml async => rootBundle.loadString(
    'packages/flutter_inappwebview/assets/t_rex_runner/t-rex.html',
  );

  @override
  Future<String> get tRexRunnerCss async => rootBundle.loadString(
    'packages/flutter_inappwebview/assets/t_rex_runner/t-rex.css',
  );

  @override
  dynamic getViewId() {
    return id;
  }

  @override
  void dispose({bool isKeepAlive = false}) {
    disposeChannel();
    _controllerFromPlatform = null;
  }
}

/// Internal hook that exposes [_handleMethod] to the constructor without
/// making it part of the public controller surface.
extension InternalInAppWebViewController on TizenInAppWebViewController {
  /// Returns the channel handler bound to [_handleMethod].
  Future<dynamic> Function(MethodCall call) get handleMethod => _handleMethod;
}
