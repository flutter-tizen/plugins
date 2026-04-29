// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_inappwebview_platform_interface/flutter_inappwebview_platform_interface.dart';
import 'package:flutter_tizen/widgets.dart';

import 'in_app_webview_controller.dart';

/// Object specifying creation parameters for creating a [PlatformInAppWebViewWidget].
///
/// Platform specific implementations can add additional fields by extending
/// this class.
class TizenInAppWebViewWidgetCreationParams
    extends PlatformInAppWebViewWidgetCreationParams {
  /// Creates a new [TizenInAppWebViewWidgetCreationParams] instance.
  TizenInAppWebViewWidgetCreationParams({
    super.controllerFromPlatform,
    super.key,
    super.layoutDirection,
    super.gestureRecognizers,
    super.headlessWebView,
    super.keepAlive,
    super.preventGestureDelay,
    super.windowId,
    super.webViewEnvironment,
    super.onWebViewCreated,
    super.onLoadStart,
    super.onLoadStop,
    @Deprecated('Use onReceivedError instead') super.onLoadError,
    super.onReceivedError,
    @Deprecated('Use onReceivedHttpError instead') super.onLoadHttpError,
    super.onReceivedHttpError,
    super.onProgressChanged,
    super.onConsoleMessage,
    super.shouldOverrideUrlLoading,
    super.onLoadResource,
    super.onScrollChanged,
    @Deprecated('Use onDownloadStartRequest instead') super.onDownloadStart,
    super.onDownloadStartRequest,
    @Deprecated('Use onLoadResourceWithCustomScheme instead')
    super.onLoadResourceCustomScheme,
    super.onLoadResourceWithCustomScheme,
    super.onCreateWindow,
    super.onCloseWindow,
    super.onJsAlert,
    super.onJsConfirm,
    super.onJsPrompt,
    super.onReceivedHttpAuthRequest,
    super.onReceivedServerTrustAuthRequest,
    super.onReceivedClientCertRequest,
    @Deprecated('Use FindInteractionController.onFindResultReceived instead')
    super.onFindResultReceived,
    super.shouldInterceptAjaxRequest,
    super.onAjaxReadyStateChange,
    super.onAjaxProgress,
    super.shouldInterceptFetchRequest,
    super.onUpdateVisitedHistory,
    @Deprecated('Use onPrintRequest instead') super.onPrint,
    super.onPrintRequest,
    super.onLongPressHitTestResult,
    super.onEnterFullscreen,
    super.onExitFullscreen,
    super.onPageCommitVisible,
    super.onTitleChanged,
    super.onWindowFocus,
    super.onWindowBlur,
    super.onOverScrolled,
    super.onZoomScaleChanged,
    @Deprecated('Use onSafeBrowsingHit instead') super.androidOnSafeBrowsingHit,
    super.onSafeBrowsingHit,
    @Deprecated('Use onPermissionRequest instead')
    super.androidOnPermissionRequest,
    super.onPermissionRequest,
    @Deprecated('Use onGeolocationPermissionsShowPrompt instead')
    super.androidOnGeolocationPermissionsShowPrompt,
    super.onGeolocationPermissionsShowPrompt,
    @Deprecated('Use onGeolocationPermissionsHidePrompt instead')
    super.androidOnGeolocationPermissionsHidePrompt,
    super.onGeolocationPermissionsHidePrompt,
    @Deprecated('Use shouldInterceptRequest instead')
    super.androidShouldInterceptRequest,
    super.shouldInterceptRequest,
    @Deprecated('Use onRenderProcessGone instead')
    super.androidOnRenderProcessGone,
    super.onRenderProcessGone,
    @Deprecated('Use onRenderProcessResponsive instead')
    super.androidOnRenderProcessResponsive,
    super.onRenderProcessResponsive,
    @Deprecated('Use onRenderProcessUnresponsive instead')
    super.androidOnRenderProcessUnresponsive,
    super.onRenderProcessUnresponsive,
    @Deprecated('Use onFormResubmission instead')
    super.androidOnFormResubmission,
    super.onFormResubmission,
    @Deprecated('Use onZoomScaleChanged instead') super.androidOnScaleChanged,
    @Deprecated('Use onReceivedIcon instead') super.androidOnReceivedIcon,
    super.onReceivedIcon,
    @Deprecated('Use onReceivedTouchIconUrl instead')
    super.androidOnReceivedTouchIconUrl,
    super.onReceivedTouchIconUrl,
    @Deprecated('Use onJsBeforeUnload instead') super.androidOnJsBeforeUnload,
    super.onJsBeforeUnload,
    @Deprecated('Use onReceivedLoginRequest instead')
    super.androidOnReceivedLoginRequest,
    super.onReceivedLoginRequest,
    super.onPermissionRequestCanceled,
    super.onRequestFocus,
    @Deprecated('Use onWebContentProcessDidTerminate instead')
    super.iosOnWebContentProcessDidTerminate,
    super.onWebContentProcessDidTerminate,
    @Deprecated(
      'Use onDidReceiveServerRedirectForProvisionalNavigation instead',
    )
    super.iosOnDidReceiveServerRedirectForProvisionalNavigation,
    super.onDidReceiveServerRedirectForProvisionalNavigation,
    @Deprecated('Use onNavigationResponse instead')
    super.iosOnNavigationResponse,
    super.onNavigationResponse,
    @Deprecated('Use shouldAllowDeprecatedTLS instead')
    super.iosShouldAllowDeprecatedTLS,
    super.shouldAllowDeprecatedTLS,
    super.onCameraCaptureStateChanged,
    super.onMicrophoneCaptureStateChanged,
    super.onContentSizeChanged,
    super.initialUrlRequest,
    super.initialFile,
    super.initialData,
    @Deprecated('Use initialSettings instead') super.initialOptions,
    super.initialSettings,
    super.contextMenu,
    super.initialUserScripts,
    super.pullToRefreshController,
    super.findInteractionController,
  });

  /// Constructs a [TizenInAppWebViewWidgetCreationParams] using a
  /// [PlatformInAppWebViewWidgetCreationParams].
  TizenInAppWebViewWidgetCreationParams.fromPlatformInAppWebViewWidgetCreationParams(
    PlatformInAppWebViewWidgetCreationParams params,
  ) : this(
        controllerFromPlatform: params.controllerFromPlatform,
        key: params.key,
        layoutDirection: params.layoutDirection,
        gestureRecognizers: params.gestureRecognizers,
        headlessWebView: params.headlessWebView,
        keepAlive: params.keepAlive,
        preventGestureDelay: params.preventGestureDelay,
        windowId: params.windowId,
        webViewEnvironment: params.webViewEnvironment,
        onWebViewCreated: params.onWebViewCreated,
        onLoadStart: params.onLoadStart,
        onLoadStop: params.onLoadStop,
        onLoadError: params.onLoadError,
        onReceivedError: params.onReceivedError,
        onLoadHttpError: params.onLoadHttpError,
        onReceivedHttpError: params.onReceivedHttpError,
        onProgressChanged: params.onProgressChanged,
        onConsoleMessage: params.onConsoleMessage,
        shouldOverrideUrlLoading: params.shouldOverrideUrlLoading,
        onLoadResource: params.onLoadResource,
        onScrollChanged: params.onScrollChanged,
        onDownloadStart: params.onDownloadStart,
        onDownloadStartRequest: params.onDownloadStartRequest,
        onLoadResourceCustomScheme: params.onLoadResourceCustomScheme,
        onLoadResourceWithCustomScheme: params.onLoadResourceWithCustomScheme,
        onCreateWindow: params.onCreateWindow,
        onCloseWindow: params.onCloseWindow,
        onJsAlert: params.onJsAlert,
        onJsConfirm: params.onJsConfirm,
        onJsPrompt: params.onJsPrompt,
        onReceivedHttpAuthRequest: params.onReceivedHttpAuthRequest,
        onReceivedServerTrustAuthRequest:
            params.onReceivedServerTrustAuthRequest,
        onReceivedClientCertRequest: params.onReceivedClientCertRequest,
        onFindResultReceived: params.onFindResultReceived,
        shouldInterceptAjaxRequest: params.shouldInterceptAjaxRequest,
        onAjaxReadyStateChange: params.onAjaxReadyStateChange,
        onAjaxProgress: params.onAjaxProgress,
        shouldInterceptFetchRequest: params.shouldInterceptFetchRequest,
        onUpdateVisitedHistory: params.onUpdateVisitedHistory,
        onPrint: params.onPrint,
        onPrintRequest: params.onPrintRequest,
        onLongPressHitTestResult: params.onLongPressHitTestResult,
        onEnterFullscreen: params.onEnterFullscreen,
        onExitFullscreen: params.onExitFullscreen,
        onPageCommitVisible: params.onPageCommitVisible,
        onTitleChanged: params.onTitleChanged,
        onWindowFocus: params.onWindowFocus,
        onWindowBlur: params.onWindowBlur,
        onOverScrolled: params.onOverScrolled,
        onZoomScaleChanged: params.onZoomScaleChanged,
        androidOnSafeBrowsingHit: params.androidOnSafeBrowsingHit,
        onSafeBrowsingHit: params.onSafeBrowsingHit,
        androidOnPermissionRequest: params.androidOnPermissionRequest,
        onPermissionRequest: params.onPermissionRequest,
        androidOnGeolocationPermissionsShowPrompt:
            params.androidOnGeolocationPermissionsShowPrompt,
        onGeolocationPermissionsShowPrompt:
            params.onGeolocationPermissionsShowPrompt,
        androidOnGeolocationPermissionsHidePrompt:
            params.androidOnGeolocationPermissionsHidePrompt,
        onGeolocationPermissionsHidePrompt:
            params.onGeolocationPermissionsHidePrompt,
        androidShouldInterceptRequest: params.androidShouldInterceptRequest,
        shouldInterceptRequest: params.shouldInterceptRequest,
        androidOnRenderProcessGone: params.androidOnRenderProcessGone,
        onRenderProcessGone: params.onRenderProcessGone,
        androidOnRenderProcessResponsive:
            params.androidOnRenderProcessResponsive,
        onRenderProcessResponsive: params.onRenderProcessResponsive,
        androidOnRenderProcessUnresponsive:
            params.androidOnRenderProcessUnresponsive,
        onRenderProcessUnresponsive: params.onRenderProcessUnresponsive,
        androidOnFormResubmission: params.androidOnFormResubmission,
        onFormResubmission: params.onFormResubmission,
        androidOnScaleChanged: params.androidOnScaleChanged,
        androidOnReceivedIcon: params.androidOnReceivedIcon,
        onReceivedIcon: params.onReceivedIcon,
        androidOnReceivedTouchIconUrl: params.androidOnReceivedTouchIconUrl,
        onReceivedTouchIconUrl: params.onReceivedTouchIconUrl,
        androidOnJsBeforeUnload: params.androidOnJsBeforeUnload,
        onJsBeforeUnload: params.onJsBeforeUnload,
        androidOnReceivedLoginRequest: params.androidOnReceivedLoginRequest,
        onReceivedLoginRequest: params.onReceivedLoginRequest,
        onPermissionRequestCanceled: params.onPermissionRequestCanceled,
        onRequestFocus: params.onRequestFocus,
        iosOnWebContentProcessDidTerminate:
            params.iosOnWebContentProcessDidTerminate,
        onWebContentProcessDidTerminate: params.onWebContentProcessDidTerminate,
        iosOnDidReceiveServerRedirectForProvisionalNavigation:
            params.iosOnDidReceiveServerRedirectForProvisionalNavigation,
        onDidReceiveServerRedirectForProvisionalNavigation:
            params.onDidReceiveServerRedirectForProvisionalNavigation,
        iosOnNavigationResponse: params.iosOnNavigationResponse,
        onNavigationResponse: params.onNavigationResponse,
        iosShouldAllowDeprecatedTLS: params.iosShouldAllowDeprecatedTLS,
        shouldAllowDeprecatedTLS: params.shouldAllowDeprecatedTLS,
        onCameraCaptureStateChanged: params.onCameraCaptureStateChanged,
        onMicrophoneCaptureStateChanged: params.onMicrophoneCaptureStateChanged,
        onContentSizeChanged: params.onContentSizeChanged,
        initialUrlRequest: params.initialUrlRequest,
        initialFile: params.initialFile,
        initialData: params.initialData,
        initialOptions: params.initialOptions,
        initialSettings: params.initialSettings,
        contextMenu: params.contextMenu,
        initialUserScripts: params.initialUserScripts,
        pullToRefreshController: params.pullToRefreshController,
        findInteractionController: params.findInteractionController,
      );
}

///{@macro flutter_inappwebview_platform_interface.PlatformInAppWebViewWidget}
class TizenInAppWebViewWidget extends PlatformInAppWebViewWidget {
  /// Constructs a [TizenInAppWebViewWidget].
  ///
  ///{@macro flutter_inappwebview_platform_interface.PlatformInAppWebViewWidget}
  TizenInAppWebViewWidget(PlatformInAppWebViewWidgetCreationParams params)
    : super.implementation(
        params is TizenInAppWebViewWidgetCreationParams
            ? params
            : TizenInAppWebViewWidgetCreationParams.fromPlatformInAppWebViewWidgetCreationParams(
                params,
              ),
      );

  TizenInAppWebViewController? _controller;

  @override
  Widget build(BuildContext context) {
    _ensureSupportedCreationParams();

    final InAppWebViewSettings initialSettings =
        params.initialSettings ?? InAppWebViewSettings();
    _inferInitialSettings(initialSettings);

    final Map<String, dynamic> settingsMap =
        (params.initialSettings != null ? initialSettings.toMap() : null) ??
        // ignore: deprecated_member_use_from_same_package
        params.initialOptions?.toMap() ??
        initialSettings.toMap();

    return TizenView(
      key: params.key,
      viewType: 'com.pichillilorenzo/flutter_inappwebview',
      layoutDirection: params.layoutDirection,
      gestureRecognizers: params.gestureRecognizers,
      onPlatformViewCreated: _onPlatformViewCreated,
      creationParamsCodec: const StandardMessageCodec(),
      creationParams: <String, dynamic>{
        'initialUrlRequest': params.initialUrlRequest?.toMap(),
        'initialFile': params.initialFile,
        'initialData': params.initialData?.toMap(),
        'initialSettings': settingsMap,
      },
    );
  }

  void _onPlatformViewCreated(int id) {
    _controller = TizenInAppWebViewController(
      PlatformInAppWebViewControllerCreationParams(
        id: id,
        webviewParams: params,
      ),
    );
    debugLog(
      className: 'TizenInAppWebViewWidget',
      id: id.toString(),
      debugLoggingSettings: PlatformInAppWebViewController.debugLoggingSettings,
      method: 'onWebViewCreated',
      args: <dynamic>[],
    );
    if (params.onWebViewCreated != null) {
      params.onWebViewCreated!(
        params.controllerFromPlatform?.call(_controller!) ?? _controller!,
      );
    }
  }

  void _ensureSupportedCreationParams() {
    _rejectIf(params.headlessWebView != null, 'HeadlessInAppWebView');
    _rejectIf(params.keepAlive != null, 'InAppWebViewKeepAlive');
    _rejectIf(params.windowId != null, 'windowId');
    _rejectIf(params.webViewEnvironment != null, 'WebViewEnvironment');
    _rejectIf(
      params.findInteractionController != null,
      'FindInteractionController',
    );
    _rejectIf(params.pullToRefreshController != null, 'PullToRefresh');
    _rejectIf(params.contextMenu != null, 'contextMenu');
    _rejectIf(
      params.initialUserScripts?.isNotEmpty ?? false,
      'initialUserScripts',
    );

    _rejectIf(params.onReceivedHttpError != null, 'onReceivedHttpError');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(params.onLoadHttpError != null, 'onLoadHttpError');
    _rejectIf(params.onLoadResource != null, 'onLoadResource');
    _rejectIf(
      params.onLoadResourceWithCustomScheme != null,
      'onLoadResourceWithCustomScheme',
    );
    _rejectIf(
      // ignore: deprecated_member_use_from_same_package
      params.onLoadResourceCustomScheme != null,
      'onLoadResourceCustomScheme',
    );
    _rejectIf(params.onDownloadStartRequest != null, 'onDownloadStartRequest');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(params.onDownloadStart != null, 'onDownloadStart');
    _rejectIf(params.onCreateWindow != null, 'onCreateWindow');
    _rejectIf(params.onCloseWindow != null, 'onCloseWindow');
    _rejectIf(
      params.onReceivedHttpAuthRequest != null,
      'onReceivedHttpAuthRequest',
    );
    _rejectIf(
      params.onReceivedServerTrustAuthRequest != null,
      'onReceivedServerTrustAuthRequest',
    );
    _rejectIf(
      params.onReceivedClientCertRequest != null,
      'onReceivedClientCertRequest',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(params.onFindResultReceived != null, 'onFindResultReceived');
    _rejectIf(
      params.shouldInterceptAjaxRequest != null,
      'shouldInterceptAjaxRequest',
    );
    _rejectIf(params.onAjaxReadyStateChange != null, 'onAjaxReadyStateChange');
    _rejectIf(params.onAjaxProgress != null, 'onAjaxProgress');
    _rejectIf(
      params.shouldInterceptFetchRequest != null,
      'shouldInterceptFetchRequest',
    );
    _rejectIf(params.onPrintRequest != null, 'onPrintRequest');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(params.onPrint != null, 'onPrint');
    _rejectIf(
      params.onLongPressHitTestResult != null,
      'onLongPressHitTestResult',
    );
    _rejectIf(params.onEnterFullscreen != null, 'onEnterFullscreen');
    _rejectIf(params.onExitFullscreen != null, 'onExitFullscreen');
    _rejectIf(params.onPageCommitVisible != null, 'onPageCommitVisible');
    _rejectIf(params.onWindowFocus != null, 'onWindowFocus');
    _rejectIf(params.onWindowBlur != null, 'onWindowBlur');
    _rejectIf(params.onOverScrolled != null, 'onOverScrolled');
    _rejectIf(params.onSafeBrowsingHit != null, 'onSafeBrowsingHit');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnSafeBrowsingHit != null,
      'androidOnSafeBrowsingHit',
    );
    _rejectIf(params.onPermissionRequest != null, 'onPermissionRequest');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnPermissionRequest != null,
      'androidOnPermissionRequest',
    );
    _rejectIf(
      params.onPermissionRequestCanceled != null,
      'onPermissionRequestCanceled',
    );
    _rejectIf(
      params.onGeolocationPermissionsShowPrompt != null,
      'onGeolocationPermissionsShowPrompt',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnGeolocationPermissionsShowPrompt != null,
      'androidOnGeolocationPermissionsShowPrompt',
    );
    _rejectIf(
      params.onGeolocationPermissionsHidePrompt != null,
      'onGeolocationPermissionsHidePrompt',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnGeolocationPermissionsHidePrompt != null,
      'androidOnGeolocationPermissionsHidePrompt',
    );
    _rejectIf(params.shouldInterceptRequest != null, 'shouldInterceptRequest');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidShouldInterceptRequest != null,
      'androidShouldInterceptRequest',
    );
    _rejectIf(params.onRenderProcessGone != null, 'onRenderProcessGone');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnRenderProcessGone != null,
      'androidOnRenderProcessGone',
    );
    _rejectIf(
      params.onRenderProcessResponsive != null,
      'onRenderProcessResponsive',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnRenderProcessResponsive != null,
      'androidOnRenderProcessResponsive',
    );
    _rejectIf(
      params.onRenderProcessUnresponsive != null,
      'onRenderProcessUnresponsive',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnRenderProcessUnresponsive != null,
      'androidOnRenderProcessUnresponsive',
    );
    _rejectIf(params.onFormResubmission != null, 'onFormResubmission');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnFormResubmission != null,
      'androidOnFormResubmission',
    );
    _rejectIf(params.onReceivedIcon != null, 'onReceivedIcon');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(params.androidOnReceivedIcon != null, 'androidOnReceivedIcon');
    _rejectIf(params.onReceivedTouchIconUrl != null, 'onReceivedTouchIconUrl');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnReceivedTouchIconUrl != null,
      'androidOnReceivedTouchIconUrl',
    );
    _rejectIf(params.onJsBeforeUnload != null, 'onJsBeforeUnload');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnJsBeforeUnload != null,
      'androidOnJsBeforeUnload',
    );
    _rejectIf(params.onReceivedLoginRequest != null, 'onReceivedLoginRequest');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.androidOnReceivedLoginRequest != null,
      'androidOnReceivedLoginRequest',
    );
    _rejectIf(params.onRequestFocus != null, 'onRequestFocus');
    _rejectIf(
      params.onWebContentProcessDidTerminate != null,
      'onWebContentProcessDidTerminate',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.iosOnWebContentProcessDidTerminate != null,
      'iosOnWebContentProcessDidTerminate',
    );
    _rejectIf(
      params.onDidReceiveServerRedirectForProvisionalNavigation != null,
      'onDidReceiveServerRedirectForProvisionalNavigation',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.iosOnDidReceiveServerRedirectForProvisionalNavigation != null,
      'iosOnDidReceiveServerRedirectForProvisionalNavigation',
    );
    _rejectIf(params.onNavigationResponse != null, 'onNavigationResponse');
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.iosOnNavigationResponse != null,
      'iosOnNavigationResponse',
    );
    _rejectIf(
      params.shouldAllowDeprecatedTLS != null,
      'shouldAllowDeprecatedTLS',
    );
    // ignore: deprecated_member_use_from_same_package
    _rejectIf(
      params.iosShouldAllowDeprecatedTLS != null,
      'iosShouldAllowDeprecatedTLS',
    );
    _rejectIf(
      params.onCameraCaptureStateChanged != null,
      'onCameraCaptureStateChanged',
    );
    _rejectIf(
      params.onMicrophoneCaptureStateChanged != null,
      'onMicrophoneCaptureStateChanged',
    );
    _rejectIf(params.onContentSizeChanged != null, 'onContentSizeChanged');
  }

  void _rejectIf(bool condition, String feature) {
    if (condition) {
      throw UnsupportedError(
        '$feature is not implemented on flutter_inappwebview_tizen.',
      );
    }
  }

  void _inferInitialSettings(InAppWebViewSettings settings) {
    if (params.shouldOverrideUrlLoading != null &&
        settings.useShouldOverrideUrlLoading == null) {
      settings.useShouldOverrideUrlLoading = true;
    }
  }

  @override
  void dispose() {
    debugLog(
      className: 'TizenInAppWebViewWidget',
      id: _controller?.getViewId()?.toString(),
      debugLoggingSettings: PlatformInAppWebViewController.debugLoggingSettings,
      method: 'dispose',
      args: <dynamic>[],
    );
    _controller?.dispose();
    _controller = null;
  }

  @override
  T controllerFromPlatform<T>(PlatformInAppWebViewController controller) {
    return (params.controllerFromPlatform?.call(controller) ?? controller) as T;
  }
}
