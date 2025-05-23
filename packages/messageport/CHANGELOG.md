## NEXT

* Fix new lint warnings.
* Update minimum Flutter and Dart version to 3.13 and 3.1.
* Update code format.

## 0.3.2

* Remove unnecessary `StreamHandlerError` implementation.
* Increase the minimum Flutter version to 3.3.

## 0.3.1

* Fix unexpected `PlatformException` when sending a null message.

## 0.3.0

* Remove the deprecated class `TizenMessagePort`.
* Refactor the C++ code.
* Code cleanups.

## 0.2.0

* Deprecate `TizenMessagePort.createLocalPort` and `TizenMessagePort.connectToRemotePort`
  in favor of newly added `LocalPort.create` and `RemotePort.connect`.
* Minor cleanups.

## 0.1.0

* Initial release.
