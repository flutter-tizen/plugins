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
    int playerId,
    StreamingPropertyType type,
  ) {
    throw UnimplementedError(
      'getStreamingProperty() has not been implemented.',
    );
  }

  /// Sets the buffer size for the player.
  Future<bool> setBufferConfig(int playerId, BufferConfigType type, int value) {
    throw UnimplementedError('setBufferConfig() has not been implemented.');
  }

  /// Set the rotate angle of display.
  Future<bool> setDisplayRotate(int playerId, DisplayRotation rotation) {
    throw UnimplementedError('setDisplayRotate() has not been implemented.');
  }

  /// Set the video display mode.
  Future<bool> setDisplayMode(int playerId, DisplayMode displayMode) {
    throw UnimplementedError('setDisplayMode() has not been implemented.');
  }

  /// Set dashplayer properties.
  Future<bool> setData(int playerId, Map<DashPlayerProperty, Object> data) {
    throw UnimplementedError('setData() has not been implemented.');
  }

  /// Get dashplayer properties.
  Future<Map<DashPlayerProperty, Object>> getData(
    int playerId,
    Set<DashPlayerProperty> keys,
  ) {
    throw UnimplementedError('getData() has not been implemented.');
  }

  /// Update token.
  Future<bool> updateDashToken(int playerId, String dashToken) {
    throw UnimplementedError('updateDashToken() has not been implemented.');
  }

  /// Get activated(selected) track infomation of the associated media.
  Future<List<Track>> getActiveTrackInfo(int playerId) {
    throw UnimplementedError('getActiveTrackInfo() has not been implemented.');
  }

  /// Set streamingengine property.
  Future<void> setStreamingProperty(
    int playerId,
    StreamingPropertyType type,
    String value,
  ) {
    throw UnimplementedError(
      'setStreamingProperty() has not been implemented.',
    );
  }

  /// Pauses the player when the application is sent to the background.
  Future<void> suspend(int playerId) {
    throw UnimplementedError('suspend() has not been implemented.');
  }

  /// Restores the player state when the application is resumed.
  Future<void> restore(
    int playerId, {
    DataSource? dataSource,
    int resumeTime = -1,
  }) {
    throw UnimplementedError('restore() has not been implemented.');
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
  Map<StreamingPropertyType, String>? streamingProperty;
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

/// The streaming property type.
enum StreamingPropertyType {
  /// HTTP request cookie used to establish the session with the HTTP server.
  cookie,

  /// HTTP user agent, used in the HTTP request header.
  userAgent,

  /// Property to initiate prebuffering mode. The second parameter indicates start-time for prebuffered content, in milliseconds.
  prebufferMode,

  /// Sets a custom streaming URL with various streaming parameters, such as "BITRATES", "STARTBITRATE", or "SKIPBITRATE".
  /// String containing custom attributes for adaptive streaming playback.
  /// "STARTBITRATE=" Valid values are "LOWEST", "HIGHEST", and "AVERAGE". You can also define a specific bandwidth for the start of playback.
  /// "BITRATES=" Use '~' to define a bandwidth range (5000 ~ 20000). You can also define a specific bandwidth for playback.
  /// "SKIPBITRATE=" Defines the bandwidth to use after a skip operation.
  /// "STARTFRAGMENT=" For live content playback, defines the start fragment number.
  /// "FIXED_MAX_RESOLUTION=max_widthXmax_height". Only if the given media URI such as mpd in MPEG-DASH or m3u8 in HLS through open()
  ///  method doesn't describe entire required video resolutions,application should use this attribute to complete the resolution information for the player.
  adaptiveInfo,

  /// Forces the player to use the 4K UHD decoder. Its parameter can be the string "TRUE" or "FALSE".
  /// In the case of adaptive streaming which requires stream-change for different video resolution during the playback,
  /// Only if the given media URI such as mpd in MPEG-DASH or m3u8 in HLS through open() method doesn't describe entire required video resolutions,
  /// pass TRUE with this property in IDLE state.
  setMode4K,

  /// For the Smooth Streaming case, configures the player to listen for a "Sparse name" configured through "propertyParam" . The sparse track name is a string.
  listenSparseTrack,

  /// Whether the stream is LIVE or VOD. Applicable to all streaming types.
  isLive,

  /// String listing the available bit-rates for the currently-playing stream.
  availableBitrate,

  /// String describing the duration of live content.
  getLiveDuration,

  /// String describing the current streaming bandwidth.
  currentBandwidth,

  /// Property used for enabling/initializing video mixer feature on B2B product only. It should be set before
  /// setting SET_MIXEDFRAME property on the player.
  useVideoMixer,

  /// Property to set the position of mixed frame. setDisplayRect with required position on corresponding
  ///  player instance to be called before setting this property.
  setMixedFrame,

  /// Property to force the playback the video in potrait mode on B2B proudct only.
  portraitMode,

  /// Property to select the Scaler type, By Default MAIN Scaler selected.
  inAppMultiView,

  /// Specifies the maximum acceptable video resolution for DASH adaptive streaming.
  ///
  /// This property allows you to set an upper limit on the video resolution
  /// (width x height) that the DASH player can select during playback.
  /// The player will not choose any resolution higher than the specified maximum.
  ///
  /// The value for this property must be a string in the format 'widthXheight',
  /// for example, '1920X1080' to set the maximum acceptable resolution to 1080p.
  /// The player will then select from resolutions up to and including 1080p.
  ///
  /// **Important**: The set maximum resolution cannot be lower than the minimum
  /// resolution available in the stream's manifest.
  unwantedResolution,

  /// Specifies the maximum acceptable video framerate for DASH adaptive streaming.
  ///
  /// This property allows you to set an upper limit on the video framerate (in frames
  /// per second) that the DASH player can select during playback. The player
  /// will not choose any framerate higher than the specified maximum.
  ///
  /// The value for this property should be a string representing the numerical framerate,
  /// for example, '30' to set the maximum acceptable framerate to 30fps. The player
  /// will then select from framerates up to and including 30fps.
  ///
  /// **Important**: The set maximum framerate cannot be lower than the minimum
  /// framerate available in the stream's manifest.
  unwantedFramerate,

  /// The audio track info of the DASH stream.
  audioStreamInfo,

  /// The susbtitle track info of the DASH stream.
  subtitleStreamInfo,

  /// The video track info of the DASH stream.
  videoStreamInfo,

  /// Only available for DASH stream.
  ///
  /// Update the language code in manifest like lang="'en'+'i'", where "i" will be an integer
  /// when there are more than one adaptation set with same language code.
  ///
  /// The value for this property is an integer string: '0','1' or others as defined in the manifest.
  updateSameLanguageCode,

  /// Sets the DASH authentication token to be used before the player is initialized.
  ///
  /// This property is used to provide an initial DASH authentication token for
  /// video streams that require token-based authentication. It should be set
  /// before the DASH player begins its preparation process (i.e., before the
  /// player is fully initialized).
  ///
  /// For detailed information about the token's purpose and format, please refer to the
  /// documentation for the [updateDashToken] API in 'video_player.dart'.
  ///
  /// The key difference between using this property and the [updateDashToken] method
  /// is the timing of their use:
  /// * `dashToken` (this property): Must be set *before* the player initialization
  ///   is complete. It is for providing the token at the outset.
  /// * `updateDashToken` (the method): Is called *after* the player has been
  ///   initialized to dynamically update or change the token during playback.
  dashToken,

  /// Only available for DASH stream.
  ///
  /// Whether to enable the function of obtaining http header. 'TRUE' or others.
  openHttpHeader,
}

/// The different types of buffer configurations that can be set on the player.
enum BufferConfigType {
  /// Total buffer size in byte.
  totalBufferSizeInByte,

  /// Total buffer size in time.
  totalBufferSizeInTime,

  /// Buffer size for play in byte.
  bufferSizeInByteForPlay,

  /// Buffer size for play in time.
  bufferSizeInSecForPlay,

  /// Buffer size for resume in byte.
  bufferSizeInByteForResume,

  /// Buffer size for resume in time.
  bufferSizeInSecForResume,

  /// Buffering timeout for play in seconds.
  bufferingTimeoutInSecForPlay,
}

/// The different types of display rotations that can be set on the player.
enum DisplayRotation {
  /// No rotation.
  rotation0,

  /// 90 degrees rotation.
  rotation90,

  /// 180 degrees rotation.
  rotation180,

  /// 270 degrees rotation.
  rotation270,
}

/// Sets the video screen mode in the specified display area.
enum DisplayMode {
  /// player display mode letter box
  letterBox,

  /// player display mode origin size
  originSize,

  /// player display mode full screen
  fullScreen,

  /// player display mode cropped full
  croppedFull,

  /// player display mode origin or letter
  originOrLetter,

  /// player display mode dst roi
  dstRoi,

  /// player display mode auto aspect ratio
  autoAspectRatio,

  /// player display mode dst roi auto aspect ratio
  dstRoiAutoAspectRatio,
}

/// The different types of dash player properties that can be set on the player.
enum DashPlayerProperty {
  /// Max band width of dash player, the value is int type.
  maxBandWidth,

  /// Dash player stream info, the value is string type.
  dashStreamInfo,

  /// Retrieves the last HTTP response header from the DASH player's network requests.
  ///
  /// This property is used to get the HTTP headers received in response to the
  /// requests made by the DASH player for fetching video manifest files or media segments.
  ///
  /// To use this property, you must first enable HTTP header retrieval by setting
  /// [StreamingPropertyType.openHttpHeader] to `'TRUE'`. Once enabled, you can
  /// retrieve the headers using the [getData] method with this `httpHeader` key.
  httpHeader,
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
    this.isPlaying,
    this.subtitleAttributes,
    this.adInfo,
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

  /// Whether the video is currently playing.
  ///
  /// Only used if [eventType] is [VideoEventType.isPlayingStateUpdate].
  final bool? isPlaying;

  /// Attributes of the video subtitle.
  final List<dynamic>? subtitleAttributes;

  /// The ad info from dash.
  ///
  /// Only used if [eventType] is [VideoEventType.adFromDash].
  final Map<Object?, Object?>? adInfo;

  @override
  bool operator ==(Object other) {
    return identical(this, other) ||
        other is VideoEvent &&
            runtimeType == other.runtimeType &&
            eventType == other.eventType &&
            duration == other.duration &&
            size == other.size &&
            buffered == other.buffered &&
            text == other.text &&
            isPlaying == other.isPlaying &&
            subtitleAttributes == other.subtitleAttributes &&
            adInfo == other.adInfo;
  }

  @override
  int get hashCode =>
      eventType.hashCode ^
      duration.hashCode ^
      size.hashCode ^
      buffered.hashCode ^
      text.hashCode ^
      isPlaying.hashCode ^
      subtitleAttributes.hashCode ^
      adInfo.hashCode;
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

  /// The playback state of the video has changed.
  ///
  /// This event is fired when the video starts or pauses due to user actions or
  /// phone calls, or other app media such as music players.
  isPlayingStateUpdate,

  /// The video need to restore player.
  restored,

  /// The ad event from dash.
  adFromDash,

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
