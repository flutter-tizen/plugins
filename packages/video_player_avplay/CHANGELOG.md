## 0.3.2

* [VVC] Add VVC decoder, disable parse for mp4/vvc, create a new vvc decoder to try decode
* Adding InBandEvent json format for data, same as MPD event
* when codec change support max resolution change
* fix location tag problem for different format
* support check role in prefer audio language logic
* fix error in location tag of relative path, regarding Period judgement error when reloading MPD.
* fix wrong audio sample rate , which is mismatch with real codec decConfig

## 0.3.1

* Resolve drm manager proxy doesn't support multiple instances issue.

## 0.3.0

* Support ADAPTIVE_INFO property.

## 0.2.3

* [dash] DASH unique event including index tag.
* [hls] Fix network set time bug(SSL Verification).
* [hls] Fix audio language to set in caps even in case of hls having mp3 audio. also done for other audio codecs.
* [http] Improved mmhttpsrc logging.
* [libav] Support VVC in ffmpeg side.
* [plusplayer] Stop feeder before renderer stop.
* [plusplayer] Update max resolution selection logic for multiview (portrait mode).
* [plusplayer] Fix set playing task crash.
* [plusplayer] Send Eos on Stop if trackrenderer prepare stuck.
* [plusplayer] Handling race condition between onmultiview cb and changesource.
* [plusplayer] Tracksource Seek only when Pause return success.
* [VVC/H.266] Add framerate force correction.
* [VVC/H.266] Bitrate change support (no drm).

## 0.2.2

* Fix can not resume issue when re-launch app.

## 0.2.1

* Replace g_idle_add with ecore_pipe.

## 0.2.0

* Add get duration API for live stream.

## 0.1.3

* Fix issue of not display video when start play.

## 0.1.2

* Replace surface id with resource id for fixing overlap issue.

## 0.1.1

* Fix load gstream libs fail issue when package name not same with app id.

## 0.1.0

* Initial release.
