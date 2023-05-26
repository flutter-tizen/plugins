## 1.3.1

* Update flutter_tts to 3.6.3.
* Update the example app.
* Add getDefaultVoice and getMaxSpeechInputLength APIs.
* Increase the minimum Flutter version to 3.3.

## 1.3.0

* Limit the range of `setSpeechRate` to be between 0.0 and 1.0.
* Unsupport `getSpeechRateValidRange`.
* Update flutter_tts to 3.5.0.
* Update the example app.

## 1.2.1

* Refactor the C++ code.

## 1.2.0

* Fix a bug where `setLanguage()` wasn't invoked due to typo in `setLanguage`.
* Update flutter_tts to 3.2.2 and update the example app.
* Support `setVolume()`.
* Minor cleanups.

## 1.1.1

* Fix calling onCancel in a successful situation and refactor native implementations.

## 1.1.0

* All APIs now return valid values (0, 1, empty list, and etc.).

## 1.0.1

* Stabilize speaking states management.

## 1.0.0

* Initial release.
