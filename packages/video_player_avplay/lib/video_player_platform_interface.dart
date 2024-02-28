// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart';
import 'package:flutter/widgets.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'src/drm_configs.dart';
import 'src/tracks.dart';
import 'src/video_player_tizen.dart';

/// The interface that implementations of video_player must implement.
///
/// Platform implementations should extend this class rather than implement it as `video_player`
/// does not consider newly added methods to be breaking changes. Extending this class
/// (using `extends`) ensures that the subclass will get the default implementation, while
/// platform implementations that `implements` this interface will be broken by newly added
/// [VideoPlayerPlatform] methods.
abstract class VideoPlayerPlatform extends PlatformInterface {
  /// Constructs a VideoPlayerPlatform.
  VideoPlayerPlatform() : super(token: _token);

  static final Object _token = Object();

  static VideoPlayerPlatform _instance = VideoPlayerTizen();

  /// The default instance of [VideoPlayerPlatform] to use.
  ///
  /// Defaults to [VideoPlayerTizen].
  static VideoPlayerPlatform get instance => _instance;

  /// Platform-specific plugins should override this with their own
  /// platform-specific class that extends [VideoPlayerPlatform] when they
  /// register themselves.
  static set instance(VideoPlayerPlatform instance) {
    PlatformInterface.verify(instance, _token);
    _instance = instance;
  }

  /// Initializes the platform interface and disposes all existing players.
  ///
  /// This method is called when the plugin is first initialized
  /// and on every full restart.
  Future<void> init() {
    throw UnimplementedError('init() has not been implemented.');
  }

  /// Clears one video.
  Future<void> dispose(int playerId) {
    throw UnimplementedError('dispose() has not been implemented.');
  }

  /// Creates an instance of a video player and returns its playerId.
  Future<int?> create(DataSource dataSource) {
    throw UnimplementedError('create() has not been implemented.');
  }

  /// Returns a Stream of [VideoEventType]s.
  Stream<VideoEvent> videoEventsFor(int playerId) {
    throw UnimplementedError('videoEventsFor() has not been implemented.');
  }

  /// Sets the looping attribute of the video.
  Future<void> setLooping(int playerId, bool looping) {
    throw UnimplementedError('setLooping() has not been implemented.');
  }

  /// Starts the video playback.
  Future<void> play(int playerId) {
    throw UnimplementedError('play() has not been implemented.');
  }

  /// Stops the video playback.
  Future<void> pause(int playerId) {
    throw UnimplementedError('pause() has not been implemented.');
  }

  /// Set the video activated.
  Future<bool> setActivate(int playerId) {
    throw UnimplementedError('setActivate() has not been implemented.');
  }

  /// Set the video deactivated.
  Future<bool> setDeactivate(int playerId) {
    throw UnimplementedError('setDeactivate() has not been implemented.');
  }

  /// Sets the volume to a range between 0.0 and 1.0.
  Future<void> setVolume(int playerId, double volume) {
    throw UnimplementedError('setVolume() has not been implemented.');
  }

  /// Sets the video position to a [Duration] from the start.
  Future<void> seekTo(int playerId, Duration position) {
    throw UnimplementedError('seekTo() has not been implemented.');
  }

  /// Gets the video tracks as a list of [VideoTrack].
  Future<List<VideoTrack>> getVideoTracks(int playerId) {
    throw UnimplementedError('getVideoTracks() has not been implemented.');
  }

  /// Gets the audio tracks as a list of [AudioTrack].
  Future<List<AudioTrack>> getAudioTracks(int playerId) {
    throw UnimplementedError('getAudioTracks() has not been implemented.');
  }

  /// Gets the text tracks as a list of [TextTrack].
  Future<List<TextTrack>> getTextTracks(int playerId) {
    throw UnimplementedError('getTextTracks() has not been implemented.');
  }

  /// Sets the selected track.
  Future<bool> setTrackSelection(int playerId, Track track) {
    throw UnimplementedError('setTrackSelection() has not been implemented.');
  }

  /// Sets the playback speed to a [speed] value indicating the playback rate.
  Future<void> setPlaybackSpeed(int playerId, double speed) {
    throw UnimplementedError('setPlaybackSpeed() has not been implemented.');
  }

  /// Gets the video position as [Duration] from the start.
  Future<Duration> getPosition(int playerId) {
    throw UnimplementedError('getPosition() has not been implemented.');
  }

  /// Gets the video duration as [DurationRange].
  Future<DurationRange> getDuration(int playerId) {
    throw UnimplementedError('getDuration() has not been implemented.');
  }

  /// Retrieves a specific property value obtained by the streaming engine (Smooth Streaming, HLS, DASH, or Widevine).
  Future<String> getStreamingProperty(
      int playerId, StreamingPropertyType type) {
    throw UnimplementedError(
        'getStreamingProperty() has not been implemented.');
  }

  /// Returns a widget displaying the video with a given playerId.
  Widget buildView(int playerId) {
    throw UnimplementedError('buildView() has not been implemented.');
  }

  /// Sets the audio mode to mix with other sources.
  Future<void> setMixWithOthers(bool mixWithOthers) {
    throw UnimplementedError('setMixWithOthers() has not been implemented.');
  }

  /// Sets the video display geometry.
  Future<void> setDisplayGeometry(
    int playerId,
    int x,
    int y,
    int width,
    int height,
  ) {
    throw UnimplementedError('setDisplayGeometry() has not been implemented.');
  }
}

/// Description of the data source used to create an instance of
/// the video player.
class DataSource {
  /// Constructs an instance of [DataSource].
  ///
  /// The [sourceType] is always required.
  ///
  /// The [uri] argument takes the form of `'https://example.com/video.mp4'` or
  /// `'file://${file.path}'`.
  ///
  /// The [formatHint] argument can be null.
  ///
  /// The [asset] argument takes the form of `'assets/video.mp4'`.
  ///
  /// The [package] argument must be non-null when the asset comes from a
  /// package and null otherwise.
  DataSource({
    required this.sourceType,
    this.uri,
    this.formatHint,
    this.asset,
    this.package,
    this.httpHeaders = const <String, String>{},
    this.drmConfigs,
    this.playerOptions,
    this.streamingProperty,
  });

  /// The way in which the video was originally loaded.
  ///
  /// This has nothing to do with the video's file type. It's just the place
  /// from which the video is fetched from.
  final DataSourceType sourceType;

  /// The URI to the video file.
  ///
  /// This will be in different formats depending on the [DataSourceType] of
  /// the original video.
  final String? uri;

  /// **Android only**. Will override the platform's generic file format
  /// detection with whatever is set here.
  final VideoFormat? formatHint;

  /// HTTP headers used for the request to the [uri].
  /// Only for [DataSourceType.network] videos.
  /// Always empty for other video types.
  Map<String, String> httpHeaders;

  /// The name of the asset. Only set for [DataSourceType.asset] videos.
  final String? asset;

  /// The package that the asset was loaded from. Only set for
  /// [DataSourceType.asset] videos.
  final String? package;

  /// Configurations for playing DRM content.
  DrmConfigs? drmConfigs;

  /// Set additional optional player settings.
  Map<String, dynamic>? playerOptions;

  /// Sets specific feature values for HTTP, MMS, or specific streaming engine
  Map<String, String>? streamingProperty;
}

/// The way in which the video was originally loaded.
///
/// This has nothing to do with the video's file type. It's just the place
/// from which the video is fetched from.
enum DataSourceType {
  /// The video was included in the app's asset files.
  asset,

  /// The video was downloaded from the internet.
  network,

  /// The video was loaded off of the local filesystem.
  file,

  /// The video is available via contentUri. Android only.
  contentUri,
}

/// The file format of the given video.
enum VideoFormat {
  /// Dynamic Adaptive Streaming over HTTP, also known as MPEG-DASH.
  dash,

  /// HTTP Live Streaming.
  hls,

  /// Smooth Streaming.
  ss,

  /// Any format other than the other ones defined in this enum.
  other,
}

/// The file format of the given video.
enum StreamingPropertyType {
  /// HTTP request cookie used to establish the session with the HTTP server.
  COOKIE,

  /// HTTP user agent, used in the HTTP request header.
  USER_AGENT,

  /// Property to initiate prebuffering mode. The second parameter indicates start-time for prebuffered content, in milliseconds.
  PREBUFFER_MODE,

  /// Sets a custom streaming URL with various streaming parameters, such as "BITRATES", "STARTBITRATE", or "SKIPBITRATE".
  /// String containing custom attributes for adaptive streaming playback.
  /// "STARTBITRATE=" Valid values are "LOWEST", "HIGHEST", and "AVERAGE". You can also define a specific bandwidth for the start of playback.
  /// "BITRATES=" Use '~' to define a bandwidth range (5000 ~ 20000). You can also define a specific bandwidth for playback.
  /// "SKIPBITRATE=" Defines the bandwidth to use after a skip operation.
  /// "STARTFRAGMENT=" For live content playback, defines the start fragment number.
  /// "FIXED_MAX_RESOLUTION=max_widthXmax_height". Only if the given media URI such as mpd in MPEG-DASH or m3u8 in HLS through open()
  ///  method doesn't describe entire required video resolutions,application should use this attribute to complete the resolution information for the player.

  ADAPTIVE_INFO,

  /// Forces the player to use the 4K UHD decoder. Its parameter can be the string "TRUE" or "FALSE".
  /// In the case of adaptive streaming which requires stream-change for different video resolution during the playback,
  /// Only if the given media URI such as mpd in MPEG-DASH or m3u8 in HLS through open() method doesn't describe entire required video resolutions,
  /// pass TRUE with this property in IDLE state.

  SET_MODE_4K,

  /// For the Smooth Streaming case, configures the player to listen for a "Sparse name" configured through "propertyParam" . The sparse track name is a string.

  LISTEN_SPARSE_TRACK,

  /// Whether the stream is LIVE or VOD. Applicable to all streaming types.

  IS_LIVE,

  /// String listing the available bit-rates for the currently-playing stream.

  AVAILABLE_BITRATE,

  /// String describing the duration of live content.

  GET_LIVE_DURATION,

  /// String describing the current streaming bandwidth.

  CURRENT_BANDWIDTH,

  /// Property used for enabling/initializing video mixer feature on B2B product only. It should be set before
  /// setting SET_MIXEDFRAME property on the player.

  USE_VIDEOMIXER,

  /// : Property to set the position of mixed frame. setDisplayRect with required position on corresponding
  ///  player instance to be called before setting this property.

  SET_MIXEDFRAME,

  /// Property to force the playback the video in potrait mode on B2B proudct only.

  PORTRAIT_MODE,

  /// Property to select the Scaler type, By Default MAIN Scaler selected.
  IN_APP_MULTIVIEW,
}

/// Event emitted from the platform implementation.
@immutable
class VideoEvent {
  /// Creates an instance of [VideoEvent].
  ///
  /// The [eventType] argument is required.
  ///
  /// Depending on the [eventType], the [duration], [size] and [buffered]
  /// arguments can be null.
  // TODO(stuartmorgan): Temporarily suppress warnings about not using const
  // in all of the other video player packages, fix this, and then update
  // the other packages to use const.
  // ignore: prefer_const_constructors_in_immutables
  VideoEvent({
    required this.eventType,
    this.duration,
    this.size,
    this.buffered,
    this.text,
  });

  /// The type of the event.
  final VideoEventType eventType;

  /// Duration of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.initialized].
  final DurationRange? duration;

  /// Size of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.initialized].
  final Size? size;

  /// Buffered size of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.bufferingUpdate].
  final int? buffered;

  /// Subtitle text of the video.
  ///
  /// Only used if [eventType] is [VideoEventType.subtitleUpdate].
  final String? text;

  @override
  bool operator ==(Object other) {
    return identical(this, other) ||
        other is VideoEvent &&
            runtimeType == other.runtimeType &&
            eventType == other.eventType &&
            duration == other.duration &&
            size == other.size &&
            buffered == other.buffered &&
            text == other.text;
  }

  @override
  int get hashCode =>
      eventType.hashCode ^
      duration.hashCode ^
      size.hashCode ^
      buffered.hashCode ^
      text.hashCode;
}

/// Type of the event.
///
/// Emitted by the platform implementation when the video is initialized or
/// completed or to communicate buffering events.
enum VideoEventType {
  /// The video has been initialized.
  initialized,

  /// The playback has ended.
  completed,

  /// Updated information on the buffering state.
  bufferingUpdate,

  /// The video started to buffer.
  bufferingStart,

  /// The video stopped to buffer.
  bufferingEnd,

  /// Updated the video subtitle text.
  subtitleUpdate,

  /// An unknown event has been received.
  unknown,
}

/// Describes a discrete segment of time within a video using a [start] and
/// [end] [Duration].
@immutable
class DurationRange {
  /// Trusts that the given [start] and [end] are actually in order. They should
  /// both be non-null.
  // TODO(stuartmorgan): Temporarily suppress warnings about not using const
  // in all of the other video player packages, fix this, and then update
  // the other packages to use const.
  // ignore: prefer_const_constructors_in_immutables
  DurationRange(this.start, this.end);

  /// The beginning of the segment described relative to the beginning of the
  /// entire video. Should be shorter than or equal to [end].
  ///
  /// For example, if the entire video is 4 minutes long and the range is from
  /// 1:00-2:00, this should be a `Duration` of one minute.
  final Duration start;

  /// The end of the segment described as a duration relative to the beginning of
  /// the entire video. This is expected to be non-null and longer than or equal
  /// to [start].
  ///
  /// For example, if the entire video is 4 minutes long and the range is from
  /// 1:00-2:00, this should be a `Duration` of two minutes.
  final Duration end;

  /// Assumes that [duration] is the total length of the video that this
  /// DurationRange is a segment form. It returns the percentage that [start] is
  /// through the entire video.
  ///
  /// For example, assume that the entire video is 4 minutes long. If [start] has
  /// a duration of one minute, this will return `0.25` since the DurationRange
  /// starts 25% of the way through the video's total length.
  double startFraction(Duration duration) {
    return start.inMilliseconds / duration.inMilliseconds;
  }

  /// Assumes that [duration] is the total length of the video that this
  /// DurationRange is a segment form. It returns the percentage that [start] is
  /// through the entire video.
  ///
  /// For example, assume that the entire video is 4 minutes long. If [end] has a
  /// duration of two minutes, this will return `0.5` since the DurationRange
  /// ends 50% of the way through the video's total length.
  double endFraction(Duration duration) {
    return end.inMilliseconds / duration.inMilliseconds;
  }

  @override
  String toString() =>
      '${objectRuntimeType(this, 'DurationRange')}(start: $start, end: $end)';

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is DurationRange &&
          runtimeType == other.runtimeType &&
          start == other.start &&
          end == other.end;

  @override
  int get hashCode => start.hashCode ^ end.hashCode;
}

/// [VideoPlayerOptions] can be optionally used to set additional player settings
@immutable
class VideoPlayerOptions {
  /// set additional optional player settings
  // TODO(stuartmorgan): Temporarily suppress warnings about not using const
  // in all of the other video player packages, fix this, and then update
  // the other packages to use const.
  // ignore: prefer_const_constructors_in_immutables
  VideoPlayerOptions({
    this.mixWithOthers = false,
    this.allowBackgroundPlayback = false,
  });

  /// Set this to true to keep playing video in background, when app goes in background.
  /// The default value is false.
  final bool allowBackgroundPlayback;

  /// Set this to true to mix the video players audio with other audio sources.
  /// The default value is false
  ///
  /// Note: This option will be silently ignored in the web platform (there is
  /// currently no way to implement this feature in this platform).
  final bool mixWithOthers;
}
