## 0.2.1

* Revise the example app.
* Fix app launch event not being fired.

## 0.2.0

* Migrate FFI implementation to tizen_interop.
* Remove `AppRunningContext.dispose`.
* Move `app_manager.dart` into the `src` directory.

## 0.1.5

* Bump ffi dependency.

## 0.1.4

* Fix arm64 build error.

## 0.1.3

* Code refactoring.

## 0.1.2

* Implement a Dart finalizer for `AppRunningContext`.
* Deprecate `AppRunningContext.dispose`.
* Clean up README.

## 0.1.1

* Add the main exporter file `tizen_app_manager.dart`.
* Add an optional parameter `background` to `AppRunningContext.terminate()`
  to support terminating background applications.

## 0.1.0

* Initial release.
