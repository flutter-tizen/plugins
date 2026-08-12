## 0.2.0

- Fix `onTitleChanged` to also fire when the page's title changes after the
  initial load (e.g. when JavaScript updates `document.title`), instead of
  only once when loading finishes.
- Fix a race where `getUrl()` could return the URL of a navigation that was
  cancelled via `shouldOverrideUrlLoading`, and skip the
  `shouldOverrideUrlLoading` round-trip for app-initiated navigations
  (`loadUrl`, `goBack`, `reload`, etc.).
- Fix `scrollBy`/`getScrollX`/`getScrollY` occasionally returning a stale
  scroll position right after `scrollTo`/`scrollBy`.

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
