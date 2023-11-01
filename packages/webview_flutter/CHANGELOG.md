## 0.8.0

* Update webivew_flutter to 4.2.3.
* Update webview_flutter_platform_interface to 2.3.0.
* Update integration_test.
* Implement `NavigationDelegate(onUrlChange)`.
* Increase the minimum Flutter version to 3.3.

## 0.7.1

* Remove inapplicable integration test cases.
* Remove unused code.
* Implements `LoadRequestMethod.post` of `PlatformWebViewController.loadRequest`.

## 0.7.0

* Update webivew_flutter to 4.0.2.
* Update webview_flutter_platform_interface to 2.0.1.

## 0.6.3

* Use only error type names defined in `web_resource_error.dart`.
* Remove unused dependencies.

## 0.6.2

* Remove the use of internal API `ewk_settings_viewport_meta_tag_set`.
* Fix crash when JavaScript evaluation result is null.
* Change the project type to staticLib.
* Enable back key navigation.
* Implement `WebSettings.javascriptMode` and `WebViewController.clearCache`.
* Redirect console messages to stdout/stderr.
* Minor code cleanups.

## 0.6.1

* Fix canGoBack/Forward error.

## 0.6.0

* Change the backing web engine from LWE to EFL WebKit (EWK).

## 0.5.6

* Update LWE binary (9af6ea4101d173935fe6e6cd3f2c91ca17ed451e).

## 0.5.5

* Update README.
* Add `-Wl,-rpath=$ORIGIN` linker option.

## 0.5.4

* Change the project type to sharedLib.

## 0.5.3

* Apply new texture APIs.

## 0.5.2

* Add back key handling.

## 0.5.1

* Apply PlatformView API change.
* Code refactoring.

## 0.5.0

* Code refactoring.
* Update the example app and integration_test.
* Sync with the latest framework code.
* Migrate to new analysis options.
* Update LWE binary (f0ca15ee41d2fc96b59fd57b63b6c32cf6c1906b).

## 0.4.4

* Update LWE binary (645719ed084d899ec7d53de1758db71a8974e446).

## 0.4.3

* Remove unused things.
* Fix build warnings.

## 0.4.2

* Support background color.

## 0.4.1

* Apply texture api change.

## 0.4.0

* Support emulator.
* Update LWE binary (b22fd0c4e50cde2b9203150d80e9d0bd1a1b0602).
* Update webivew_flutter to 3.0.1.

## 0.3.11

* Organize includes.

## 0.3.10

* Apply `PlatformView` and `PlatformViewFactory` API changes.

## 0.3.9

* Update LWE binary (6bae13cb915bd41c5aac4aaaae72865f20924c03).

## 0.3.8

* Update webivew_flutter to 2.3.0.

## 0.3.7

* Update webivew_flutter to 2.1.1.

## 0.3.6

* Update LWE binary (3dff8724bfb4b2b0b9e7c4e3976a9b02e74ee13c).
* Fix various issues.

## 0.3.5

* Update LWE binary (b2fad69f50d693c86abc45b363a39b0625f5e95f).
* Fix crash issue.

## 0.3.4

* Fix buffer synchronization issue.

## 0.3.3

* Update LWE binary (c57d045a513455115a8a4c66517e5e51f5a4dfbd).
* Fix issue of multiple webviews.

## 0.3.2

* Update LWE binary (2226c28429391407d7c875c3af7531f5e1d5dfa7) for supporting google_map_flutter_tizen.

## 0.3.1

* Update lightweight web engine binary (ad0e77631f96180e19a11c3dc80b6b72c32bdffb).
* Fix bug on handling parameter of `loadUrl` API.

## 0.3.0

* Apply `PlatformView` and `PlatformViewFactory` API changes.

## 0.2.2

* Update lightweight web engine binary & header file (6263be6c888d5cb9dcca5466dfc3941a70b424eb).
* Activate resizing function.
* Apply embedder's texture API changes.

## 0.2.1

* Add lightweight web engine binary for arm64.

## 0.2.0

* Update Dart and Flutter SDK constraints.
* Update Flutter and Samsung copyright information.
* Update webview_flutter_tizen to use platform view interface.
* Update example and integration_test.
* Update webivew_flutter to 2.0.4.

## 0.1.0

* Initial release.
