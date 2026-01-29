## NEXT

* Adds compatibility with `http` 1.0 in example.

## 3.1.2

* Update code format.
* Handle player resource cleanup in destructor
* Rename Release() to ReleaseMediaSource() for clearer responsibility
* Handle sending null values when exceptions occur in GetDuration() and GetPosition()
* Add logging for unsupported features
* Fix range limitation of SetVolume() and SetPlaybackRate()


## 3.1.1

* Update audioplayers to 6.4.0.
* Update audioplayers_platform_interface to 7.1.0.
* Add 'xyz.luan/audioplayers.global' Method Channel.
* Add 'xyz.luan/audioplayers.global/events' Event Channel.
* Update example app

## 3.1.0

* Update audioplayers to 6.1.0.
* Update audioplayers_platform_interface to 7.0.0.
* Changed to create a player when AudioPlayer created.
* Remove 'audio.onCurrentPosition' method event.

## 3.0.2

* Update minimum Flutter and Dart version to 3.13 and 3.1.
* Fix play() didn't work after setSource().

## 3.0.1

* Update audioplayers to 5.1.0.
* Update audioplayers_platform_interface to 6.0.0.
* Update example app.

## 3.0.0

* Update audioplayers to 4.1.0.
* Update audioplayers_platform_interface to 5.0.1.
* Update example app.
* Update README.
* Increase the minimum Flutter version to 3.3.

## 2.0.0

* Add audioplayers_platform_interface dependency and update method calls.
* Add flame_lint dependency and update analysis_options.
* Update audioplayers to 1.0.1.
* Update example app and integration_test.
* Update minimal sdk version to 2.14.0.
* Update minimal flutter version to 2.5.0.
* Update README.
* (TV device) Fix seek to work on different playrates.
* (TV device) Fix changing playrates at anytime during audio play.
* (TV device) Fix not returning complete event when audio play finishes.

## 1.1.0

* Update README and analysis_options.
* Update audioplayers to 0.20.1.
* Update the example app and integration_test.
* Initialize variables properly.

## 1.0.2

* Update README.

## 1.0.1

* Add integration test.

## 1.0.0

* Initial release.
