## 0.2.0

- Fix `onTitleChanged` to fire on title changes after the initial load, not just once at load finish.
- Fix `getUrl()` returning a cancelled navigation's URL, and skip `shouldOverrideUrlLoading` for app-initiated navigations.
- Fix `shouldOverrideUrlLoading` not being invoked for a hardware/remote Back-key navigation.
- Fix `scrollBy`/`getScrollX`/`getScrollY` occasionally returning a stale scroll position.
- Fix `onUpdateVisitedHistory`/`getUrl()` staying pinned to a cancelled navigation's URL after a later same-document URL change, and bound how long a pending `scrollTo`/`scrollBy` target is trusted.

## 0.1.4

- Fix a SIGTRAP crash on TV app teardown by calling `ewk_init()`/`ewk_shutdown()` exactly once per process.

## 0.1.3

* Add an `implements` entry to the pubspec to improve discoverability on pub.dev.

## 0.1.2

* Update analysis_options.yaml for Flutter 3.47.0.
* Temporarily suppress new analyze issues via analysis_options.yaml rules after the Flutter 3.47.0 upgrade.
* Update the repository URL to use the `main` branch.

## 0.1.1

- Fix a crash when a webview is disposed.

## 0.1.0

- Initial release of `flutter_inappwebview_tizen`.
