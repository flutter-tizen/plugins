## 0.5.8

* Support Tizen 9.0

## 0.5.7

* Update plusplayer
  1. [PLAYER] Feeder Threads for Audio and Video boosted.
  2. [HLS] hls_trickplay : for rates (0-2) I-frame dependency removed.
  3. [HLS] ss_trickplay : trickplay by seek enabled.
  4. [HLS] Trickplay feature update for playbackrates (<0 and >2).
  5. [HLS] Unused variable removed and indentation fix.
  6. [HLS] Outputmanager logs getting skipped for some parts, Incorrect usage of GetPart(), GetPartOutput() is used instead of GetPart().
  7. [HLS] playlist refresh rate reduced.
  8. [HLS] MA LOG: curl operation time out case.
  9. [HLS] Fix Garbage Frame Issue due to new implementation of dynamic storage of init data.
  10. [HLS] Removed unnecessary logs.
  11. [HLS] Special handling added to reset in-process curl download request during reset operation.
  12. [HLS] Fix Garbage Frame Issue due to new implementation of dynamic storage of init data.
  13. [HLS] Initialise catchupInstance only once in catchuptask.
  14. [HLS] Handling EXT-X-SESSION-KEY info and trigger early license acquisition when available and also removing the redundant code but merging the related functions for sending drm data.
  15. [HLS] Fix added: playback gets stuck when next download is preload-hinted-part in case of bitrate change with no new part, or new discontinuity with preload-hinted-part.
  16. [HLS] Free pluginlist after setting state to null.
  17. [HLS] Removed [-Wunused-variable] warning from HLS engine.
  18. [HLS] Removed [-Wswitch-default] warning from HLS engine.
  19. [HLS] Curl Reset flag to avoid wait in Process().
  20. [HLS] InitDataS vector was changed to Map.
  21. [HLS] LL Playlist Blocking Reload Fix + Improved Logging
  22. [HLS] optimise subplaylist downlaod rate.
  23. [HLS] Redundant gst Buffer unref removed.
  24. [HLS] "Avoid Last Viewed Scene" Feature.
  25. [HLS] AE-SDK code changes merging on onemain.
  26. [HLS] External Media track change fix.
  27. [HLS] Correction for MA Log Task.
  28. [HLS] Added ut_empty_cue_before_endlist.
  29. [HLS] LL-HLS Multitrack change support.
  30. [HLS] Pause resume enabled in live property added in player.
  31. [PLAYER] Enable MA_LOG in plusplayer.
  32. [HLS] Set ABR property : Added functionality in engine and player to toggle bitrate change functionality.
  33. [HLS] Increase disc pool size to handle more discontinuties in TVPLus VOD content.
  34. [HLS] Fix for drm content playback and enabling MA_Info_Event macro.
  35. [PLAYER] Fix plusplyaer solo build error.
  36. [DASH] apply libxml patch for tpk cross use.

## 0.5.6

* Make startPosition support int32_t type.

## 0.5.5

* Fix select audio channel failed issue.

## 0.5.4

* Fix start position out of range issue.

## 0.5.3

* Add 'isCompleted' event to 'VideoPlayerEvent'.

## 0.5.2

* Add start position in player options when creating player. This is useful for resuming playback from last viewed position.

## 0.5.1

* Fix getVideoTracks out of bounds issue.

## 0.5.0

* Fix DashEngine crash issue.

## 0.4.9

* Fix DashEngine stream property "STARTBITRATE" can't be set correctly.
* Fix DashEngine stream property "USER_AGENT" can't be set correctly.

## 0.4.8

* Call the open before calling the SetStreamingProperty.
* Change getStreamingProperty API return type from StreamingPropertyMessage to String.
* Add setStreamingProperty API.

## 0.4.7

* Add SetBufferConfig interface.

## 0.4.6

* Upgrade plusplayer
  1. [HLS] Disabling playback rate change and reducing max latency threshold.
  2. [HLS] Bitrate switching optimization and seek to live handling.
  3. [DASH] Fixing LL-DASH live channel stuck issue.
  4. [PLUSPLAYER] Fixing svace issue cast from int to uint64_t.
  5. [DASH] Optimization for LL-DASH catch-up.
  6. [HLS] Null check before dereferencing pointer.
  7. [HLS] Fix Live stream getting stucked after 15-20 min.

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
