## 2.5.4

* Update video_player to 2.9.2.
* Update video_player_platform_interface to 6.2.3.

## 2.5.3

* Support `httpHeaders` option of `VideoPlayerController.network`.

## 2.5.2

* Live streaming content starts playing immediately when SeekTo() is called.

## 2.5.1

* Update pigeon to 22.3.0.
* Temporarily set duration to 1, if duration is 0.

## 2.5.0

* Update video_player to 2.9.1.
* Update video_player_platform_interface to 6.2.0.
* Update the example app and integration_test.
* Fix new lint warnings.
* Update minimum Flutter and Dart version to 3.19 and 3.3.
* Synchronize VideoPlayerValue.isPlaying with underlying video player.

## 2.4.9

* Fix event channel issue, sending messages from native to Flutter on the platform thread.

## 2.4.8

* Disable screensaver when player is playing.

## 2.4.7

* Update pigeon to 10.0.0.
* Minor cleanups.

## 2.4.6

* Fix memory leak issue.
* Increase the minimum Flutter version to 3.3.

## 2.4.5

* Update README with supported devices information.

## 2.4.4

* Update pigeon to 6.0.1.

## 2.4.3

* Migrate to in-package pigeon-generated code.

## 2.4.2

* Add a queue for storing decoded media packet.

## 2.4.1

* Apply new texture APIs.

## 2.4.0

* Update video_player to 2.4.2.
* Update video_player_platform_interface to 5.1.2.
* Update the example app and integration_test.
* Migrate to new analysis options.
* Remove obsolete dependency on pedantic.
* Code cleanups.

## 2.3.2

* Apply texture common release callback.

## 2.3.1

* Show first frame after player is prepared.
* Assign `on_completed_cb_` of player before calling `player_set_play_position` to fix intermittent issue.

## 2.3.0

* Fix a freezing issue on Flutter 2.5 or above.
* Never return empty error messages to avoid null reference exceptions.
* Update video_player to 2.2.6 and update the example app.
* Minor cleanups.

## 2.2.2

* Fix `seekTo` so that it returns when seeking is completed.

## 2.2.1

* Update README.

## 2.2.0

* Update video_player to 2.2.3.
* Update video_player_platform_interface to 4.2.0.

## 2.0.1

* Update integration_test.

## 2.0.0

* Appaly new common texture APIs.

## 1.0.0

* Initial release.
