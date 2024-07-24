## 0.4.5

* Update plusplayer.ini for using platform gst-ffmpeg library.

## 0.4.4

* Remove wrong information in README.
* Add plusplayer ini.
* Fix crash issue when error message is empty.

## 0.4.3

* Update plusplayer
  1. [libav-common] Fixing libav-common SVACE issue.
  2. [Dash] Fix when app freeze , cpu usage is high.
  3. [Dash] Fix issue that report two eos msg to app.
  4. [Dash] Fix no EOS event of dashplayer.
  5. [Dash] Fix app freeze issue when calling seek method.
  6. [Dash] Fix issue that live stream start at the beginning not at live postion.

## 0.4.2

* Add notes for creating dash player.
* [Dash] Fix no EOS event of dashplayer.
* Remove set looping failed message.
* Support Tizen 8.0.

## 0.4.1

* Fix new lint warnings.
* Update minimum Flutter and Dart version to 3.13 and 3.1.
* Update plusplayer
  1. [HLS] Sending dummy audio pkts to minimize pts gap between audio pkts across discontinuity.
  2. [HLS] Low Latency implementation for external track (audio/subtitle).
  3. [HLS] Segment list improvement using index offset.
  4. [SS] Fix for memory leak.
  5. [DASH] For dash case, adding samplerate change limitation for seamless audio track changes.
  6. [DASH] Adding dash DRM case for HbbTV fault key test case.
  7. [DASH] Fixing app_id dummy exception case.
  8. [DASH] Support ATSC3 L1 time.

## 0.4.0

* Minor refactor.
* Add getStreamingProperty interface.

## 0.3.3

* Check httper_headers pointer when creating media player.
* Fix state check in SetVolume.
* Add a case that plays the video of assets to the example.
* Convert volume range to [0,100] for plusplayer.

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
