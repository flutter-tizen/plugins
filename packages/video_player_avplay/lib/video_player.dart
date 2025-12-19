// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:io';

import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_tizen/flutter_tizen.dart' as tizen;

import 'src/ad_info_from_dash.dart';
import 'src/closed_caption_file.dart';
import 'src/drm_configs.dart';
import 'src/hole.dart';
import 'src/tracks.dart';
import 'video_player_platform_interface.dart';

export 'src/closed_caption_file.dart';
export 'src/drm_configs.dart';
export 'src/tracks.dart';

/// This will be used to set the ResumeTime when player restore.
typedef RestoreTimeCallback = int Function();

/// This will be used to set the RecreateMessage when player restore.
typedef RestoreDataSourceCallback = DataSource Function();

VideoPlayerPlatform? _lastVideoPlayerPlatform;

VideoPlayerPlatform get _videoPlayerPlatform {
  final VideoPlayerPlatform currentInstance = VideoPlayerPlatform.instance;
  if (_lastVideoPlayerPlatform != currentInstance) {
    // This will clear all open videos on the platform when a full restart is
    // performed.
    currentInstance.init();
    _lastVideoPlayerPlatform = currentInstance;
  }
  return currentInstance;
}

/// The duration, current position, buffering state, error state and settings
/// of a [VideoPlayerController].
@immutable
class VideoPlayerValue {
  /// Constructs a video with the given values. Only [duration] is required. The
  /// rest will initialize with default values when unset.
  const VideoPlayerValue({
    required this.duration,
    this.size = Size.zero,
    this.position = Duration.zero,
    this.textCaption = TextCaption.none,
    this.pictureCaption = PictureCaption.none,
    this.captionOffset = Duration.zero,
    this.tracks = const <Track>[],
    this.buffered = 0,
    this.isInitialized = false,
    this.isPlaying = false,
    this.isLooping = false,
    this.isBuffering = false,
    this.volume = 1.0,
    this.playbackSpeed = 1.0,
    this.errorDescription,
    this.isCompleted = false,
    this.adInfo,
    this.manifestInfo,
  });

  /// Returns an instance for a video that hasn't been loaded.
  VideoPlayerValue.uninitialized()
      : this(
          duration: DurationRange(Duration.zero, Duration.zero),
          isInitialized: false,
        );

  /// Returns an instance with the given [errorDescription].
  VideoPlayerValue.erroneous(String errorDescription)
      : this(
          duration: DurationRange(Duration.zero, Duration.zero),
          isInitialized: false,
          errorDescription: errorDescription,
        );

  /// This constant is just to indicate that parameter is not passed to [copyWith]
  /// workaround for this issue https://github.com/dart-lang/language/issues/2009
  static const String _defaultErrorDescription = 'defaultErrorDescription';

  /// The total duration of the video.
  ///
  /// The duration is [Duration.zero] if the video hasn't been initialized.
  final DurationRange duration;

  /// The current playback position.
  final Duration position;

  /// The [TextCaption] that should be displayed based on the current [position].
  ///
  ///  If there is no text caption for the current
  /// [position], this will be a [TextCaption.none] object.
  final TextCaption textCaption;

  /// The [PictureCaption] that should be displayed based on the current [position].
  ///
  ///  If there is no text caption for the current
  /// [position], this will be a [PictureCaption.none] object.
  final PictureCaption pictureCaption;

  /// The [Duration] that should be used to offset the current [position] to get the correct [Caption].
  ///
  /// Defaults to Duration.zero.
  final Duration captionOffset;

  /// The currently buffered size.
  final int buffered;

  /// True if the video is playing. False if it's paused.
  final bool isPlaying;

  /// True if the video is looping.
  final bool isLooping;

  /// True if the video is currently buffering.
  final bool isBuffering;

  /// The current volume of the playback.
  final double volume;

  /// The current speed of the playback.
  final double playbackSpeed;

  /// The current playback tracks.
  final List<Track> tracks;

  /// A description of the error if present.
  ///
  /// If [hasError] is false this is `null`.
  final String? errorDescription;

  /// True if video has finished playing to end.
  ///
  /// Reverts to false if video position changes, or video begins playing.
  /// Does not update if video is looping.
  final bool isCompleted;

  /// The [size] of the currently loaded video.
  final Size size;

  /// Indicates whether or not the video has been loaded and is ready to play.
  final bool isInitialized;

  /// Provides information about advertisements embedded within a DASH video stream.
  ///
  /// This property holds an [AdInfoFromDash] object when an advertisement event
  /// is detected during video playback. It is particularly relevant for DASH
  /// (Dynamic Adaptive Streaming over HTTP) streams that contain ad breaks.
  /// When the video player transitions to an ad segment, this property will be
  /// populated with metadata about the current advertisement, allowing the
  /// application to react to ad events (e.g., display an ad overlay, skip button,
  /// or other ad-related UI).
  ///
  /// If no ad is currently playing or if the video stream does not support
  /// ad information, this property will be `null`. You can check [hasAdInfo]
  /// to determine if ad information is available.
  final AdInfoFromDash? adInfo;

  /// Provides information about manifest when the DASH streaming manifest is
  /// updated (e.g., when new video segments become available, bitrate changes
  /// occur, or the stream configuration is modified), this property will be
  /// populated with the updated manifest information.
  ///
  /// If no manifest update has occurred or if the video format does not
  /// support dynamic manifest updates, this property will be `null`. You can
  /// check [hasManifestUpdated] to determine if manifest information is available.
  final String? manifestInfo;

  /// Indicates whether or not the video is in an error state. If this is true
  /// [errorDescription] should have information about the problem.
  bool get hasError => errorDescription != null;

  /// Indicates whether or not the video has ADInfo.
  bool get hasAdInfo => adInfo != null;

  /// Indicates whether the video has updated its manifest.
  bool get hasManifestUpdated => manifestInfo != null;

  /// Returns [size.width] / [size.height].
  ///
  /// Will return `1.0` if:
  /// * [isInitialized] is `false`
  /// * [size.width], or [size.height] is equal to `0.0`
  /// * aspect ratio would be less than or equal to `0.0`
  double get aspectRatio {
    if (!isInitialized || size.width == 0 || size.height == 0) {
      return 1.0;
    }
    final double aspectRatio = size.width / size.height;
    if (aspectRatio <= 0) {
      return 1.0;
    }
    return aspectRatio;
  }

  /// Returns the [TextCaption] that should be displayed based on the current [position].
  /// If the value of [textCaption.end] has greater than the current [position], this will be a [TextCaption.none] object.
  /// Only used for [copyWith].
  TextCaption get _currentTextCaption {
    return position > textCaption.end ? TextCaption.none : textCaption;
  }

  /// Returns the [PictureCaption] that should be displayed based on the current [position].
  /// If the value of [PictureCaption.end] has greater than the current [position], this will be a [PictureCaption.none] object.
  /// Only used for [copyWith].
  PictureCaption get _currentPictureCaption {
    return position > pictureCaption.end ? PictureCaption.none : pictureCaption;
  }

  /// Returns a new instance that has the same values as this current instance,
  /// except for any overrides passed in as arguments to [copyWidth].
  VideoPlayerValue copyWith({
    DurationRange? duration,
    Size? size,
    Duration? position,
    TextCaption? textCaption,
    PictureCaption? pictureCaption,
    Duration? captionOffset,
    List<Track>? tracks,
    int? buffered,
    bool? isInitialized,
    bool? isPlaying,
    bool? isLooping,
    bool? isBuffering,
    double? volume,
    double? playbackSpeed,
    String? errorDescription = _defaultErrorDescription,
    bool? isCompleted,
    AdInfoFromDash? adInfo,
    String? manifestInfo,
  }) {
    return VideoPlayerValue(
      duration: duration ?? this.duration,
      size: size ?? this.size,
      textCaption: textCaption ?? _currentTextCaption,
      pictureCaption: pictureCaption ?? _currentPictureCaption,
      position: position ?? this.position,
      captionOffset: captionOffset ?? this.captionOffset,
      tracks: tracks ?? this.tracks,
      buffered: buffered ?? this.buffered,
      isInitialized: isInitialized ?? this.isInitialized,
      isPlaying: isPlaying ?? this.isPlaying,
      isLooping: isLooping ?? this.isLooping,
      isBuffering: isBuffering ?? this.isBuffering,
      volume: volume ?? this.volume,
      playbackSpeed: playbackSpeed ?? this.playbackSpeed,
      errorDescription: errorDescription != _defaultErrorDescription
          ? errorDescription
          : this.errorDescription,
      isCompleted: isCompleted ?? this.isCompleted,
      adInfo: adInfo,
      manifestInfo: manifestInfo,
    );
  }

  @override
  String toString() {
    return '${objectRuntimeType(this, 'VideoPlayerValue')}('
        'duration: $duration, '
        'size: $size, '
        'position: $position, '
        'textCaption: $textCaption, '
        'pictureCaption: $pictureCaption, '
        'captionOffset: $captionOffset, '
        'tracks: $tracks, '
        'buffered: $buffered, '
        'isInitialized: $isInitialized, '
        'isPlaying: $isPlaying, '
        'isLooping: $isLooping, '
        'isBuffering: $isBuffering, '
        'volume: $volume, '
        'playbackSpeed: $playbackSpeed, '
        'errorDescription: $errorDescription, '
        'adInfo: $adInfo, '
        'manifestInfo: $manifestInfo, '
        'isCompleted: $isCompleted),';
  }

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is VideoPlayerValue &&
          runtimeType == other.runtimeType &&
          duration == other.duration &&
          size == other.size &&
          position == other.position &&
          textCaption == other.textCaption &&
          pictureCaption == other.pictureCaption &&
          captionOffset == other.captionOffset &&
          listEquals(tracks, other.tracks) &&
          buffered == other.buffered &&
          isInitialized == other.isInitialized &&
          isPlaying == other.isPlaying &&
          isLooping == other.isLooping &&
          isBuffering == other.isBuffering &&
          volume == other.volume &&
          playbackSpeed == other.playbackSpeed &&
          errorDescription == other.errorDescription &&
          adInfo == other.adInfo &&
          manifestInfo == other.manifestInfo &&
          isCompleted == other.isCompleted;

  @override
  int get hashCode => Object.hash(
        duration,
        size,
        position,
        textCaption,
        pictureCaption,
        captionOffset,
        tracks,
        buffered,
        isInitialized,
        isPlaying,
        isLooping,
        isBuffering,
        volume,
        playbackSpeed,
        errorDescription,
        adInfo,
        manifestInfo,
        isCompleted,
      );
}

/// Controls a platform video player, and provides updates when the state is
/// changing.
///
/// Instances must be initialized with initialize.
///
/// The video is displayed in a Flutter app by creating a [VideoPlayer] widget.
///
/// To reclaim the resources used by the player call [dispose].
///
/// After [dispose] all further calls are ignored.
class VideoPlayerController extends ValueNotifier<VideoPlayerValue> {
  /// Constructs a [VideoPlayerController] playing a video from an asset.
  ///
  /// The name of the asset is given by the [dataSource] argument and must not be
  /// null. The [package] argument must be non-null when the asset comes from a
  /// package and null otherwise.
  VideoPlayerController.asset(
    this.dataSource, {
    this.package,
    this.closedCaptionFile,
    this.videoPlayerOptions,
  })  : dataSourceType = DataSourceType.asset,
        formatHint = null,
        httpHeaders = const <String, String>{},
        drmConfigs = null,
        playerOptions = const <String, dynamic>{},
        streamingProperty = null,
        super(
          VideoPlayerValue(
            duration: DurationRange(Duration.zero, Duration.zero),
          ),
        );

  /// Constructs a [VideoPlayerController] playing a video from obtained from
  /// the network.
  ///
  /// The URI for the video is given by the [dataSource] argument and must not be
  /// null.
  /// **Android only**: The [formatHint] option allows the caller to override
  /// the video format detection code.
  /// [httpHeaders] option allows to specify HTTP headers
  /// for the request to the [dataSource].
  VideoPlayerController.network(
    this.dataSource, {
    this.formatHint,
    this.closedCaptionFile,
    this.videoPlayerOptions,
    this.httpHeaders = const <String, String>{},
    this.drmConfigs,
    this.playerOptions,
    this.streamingProperty,
  })  : dataSourceType = DataSourceType.network,
        package = null,
        super(
          VideoPlayerValue(
            duration: DurationRange(Duration.zero, Duration.zero),
          ),
        );

  /// Constructs a [VideoPlayerController] playing a video from a file.
  ///
  /// This will load the file from the file-URI given by:
  /// `'file://${file.path}'`.
  VideoPlayerController.file(
    File file, {
    this.closedCaptionFile,
    this.videoPlayerOptions,
  })  : dataSource = 'file://${file.path}',
        dataSourceType = DataSourceType.file,
        package = null,
        formatHint = null,
        httpHeaders = const <String, String>{},
        drmConfigs = null,
        playerOptions = const <String, dynamic>{},
        streamingProperty = null,
        super(
          VideoPlayerValue(
            duration: DurationRange(Duration.zero, Duration.zero),
          ),
        );

  /// Constructs a [VideoPlayerController] playing a video from a contentUri.
  ///
  /// This will load the video from the input content-URI.
  /// This is supported on Android only.
  VideoPlayerController.contentUri(
    Uri contentUri, {
    this.closedCaptionFile,
    this.videoPlayerOptions,
  })  : assert(
          defaultTargetPlatform == TargetPlatform.android,
          'VideoPlayerController.contentUri is only supported on Android.',
        ),
        dataSource = contentUri.toString(),
        dataSourceType = DataSourceType.contentUri,
        package = null,
        formatHint = null,
        httpHeaders = const <String, String>{},
        drmConfigs = null,
        playerOptions = const <String, dynamic>{},
        streamingProperty = null,
        super(
          VideoPlayerValue(
            duration: DurationRange(Duration.zero, Duration.zero),
          ),
        );

  /// The URI to the video file. This will be in different formats depending on
  /// the [DataSourceType] of the original video.
  final String dataSource;

  /// HTTP headers used for the request to the [dataSource].
  /// Only for [VideoPlayerController.network].
  /// Always empty for other video types.
  final Map<String, String> httpHeaders;

  /// Configurations for playing DRM content (optional).
  /// Only for [VideoPlayerController.network].
  final DrmConfigs? drmConfigs;

  /// Player Options used for add additional parameters.
  /// Only for [VideoPlayerController.network].
  final Map<String, dynamic>? playerOptions;

  /// Sets specific feature values for HTTP, MMS, or specific streaming engine (Smooth Streaming, HLS, DASH, DivX Plus Streaming, or Widevine).
  /// The available streaming properties depend on the streaming protocol or engine.
  /// Only for [VideoPlayerController.network].
  final Map<StreamingPropertyType, String>? streamingProperty;

  /// **Android only**. Will override the platform's generic file format
  /// detection with whatever is set here.
  final VideoFormat? formatHint;

  /// Describes the type of data source this [VideoPlayerController]
  /// is constructed with.
  final DataSourceType dataSourceType;

  /// Provide additional configuration options (optional). Like setting the audio mode to mix
  final VideoPlayerOptions? videoPlayerOptions;

  /// Only set for [asset] videos. The package that the asset was loaded from.
  final String? package;

  /// Optional field to specify a file containing the closed
  /// captioning.
  ///
  /// This future will be awaited and the file will be loaded when
  /// [initialize()] is called.
  final Future<ClosedCaptionFile>? closedCaptionFile;

  ClosedCaptionFile? _closedCaptionFile;
  Timer? _timer;
  Timer? _durationTimer;
  bool _isDisposed = false;
  Completer<void>? _creatingCompleter;
  StreamSubscription<dynamic>? _eventSubscription;
  _VideoAppLifeCycleObserver? _lifeCycleObserver;
  RestoreDataSourceCallback? _onRestoreDataSource;
  RestoreTimeCallback? _onRestoreTime;

  /// The id of a player that hasn't been initialized.
  @visibleForTesting
  static const int kUninitializedPlayerId = -1;
  int _playerId = kUninitializedPlayerId;

  /// This is just exposed for testing. It shouldn't be used by anyone depending
  /// on the plugin.
  @visibleForTesting
  int get playerId => _playerId;

  final MethodChannel _channel = const MethodChannel(
    'dev.flutter.videoplayer.drm',
  );

  Future<void> _checkPlatformAndApiVersion() async {
    final DeviceInfoPluginTizen deviceInfoPlugin = DeviceInfoPluginTizen();
    final TizenDeviceInfo deviceInfo = await deviceInfoPlugin.tizenInfo;

    if ((deviceInfo.platformVersion != null &&
            deviceInfo.platformVersion!.isNotEmpty) &&
        tizen.apiVersion != 'none') {
      if (deviceInfo.platformVersion != tizen.apiVersion) {
        final double? platformVersion = double.tryParse(
          deviceInfo.platformVersion!,
        );
        final double? apiVersion = double.tryParse(tizen.apiVersion);
        if (platformVersion != null && apiVersion != null) {
          if (platformVersion == 6.0 || platformVersion == 10.0) {
            throw Exception(
              'The current TizenOS version is $platformVersion and the app API version($apiVersion). The app API version must also be $platformVersion. '
              'The avplay plugin, with an apiVersion of $apiVersion does not guarantee compatibility with other TizenOS versions. Therefore '
              'Please set the "api-version" in tizen-manifest.xml to $platformVersion and rebuild.',
            );
          } else if ((platformVersion >= 6.5 && platformVersion <= 9.0) &&
              (apiVersion == 6.0 || apiVersion == 10.0)) {
            throw Exception(
              'The current TizenOS version is $platformVersion and the app API version($apiVersion). The app API version must be at least 6.5. '
              'The avplay plugin, with an apiVersion of $apiVersion does not guarantee compatibility with other TizenOS versions. Therefore '
              'Please set the "api-version" in tizen-manifest.xml to a minimum of 6.5 and rebuild.',
            );
          }
        }
      }
    }
  }

  /// Attempts to open the given [dataSource] and load metadata about the video.
  Future<void> initialize() async {
    await _checkPlatformAndApiVersion();

    final bool allowBackgroundPlayback =
        videoPlayerOptions?.allowBackgroundPlayback ?? false;
    if (!allowBackgroundPlayback) {
      _lifeCycleObserver = _VideoAppLifeCycleObserver(this);
    }
    _lifeCycleObserver?.initialize();
    _creatingCompleter = Completer<void>();

    late DataSource dataSourceDescription;
    switch (dataSourceType) {
      case DataSourceType.asset:
        dataSourceDescription = DataSource(
          sourceType: DataSourceType.asset,
          asset: dataSource,
          package: package,
        );
      case DataSourceType.network:
        dataSourceDescription = DataSource(
          sourceType: DataSourceType.network,
          uri: dataSource,
          formatHint: formatHint,
          httpHeaders: httpHeaders,
          drmConfigs: drmConfigs,
          playerOptions: playerOptions,
          streamingProperty: streamingProperty,
        );
      case DataSourceType.file:
        dataSourceDescription = DataSource(
          sourceType: DataSourceType.file,
          uri: dataSource,
        );
      case DataSourceType.contentUri:
        dataSourceDescription = DataSource(
          sourceType: DataSourceType.contentUri,
          uri: dataSource,
        );
    }

    if (videoPlayerOptions?.mixWithOthers != null) {
      await _videoPlayerPlatform.setMixWithOthers(
        videoPlayerOptions!.mixWithOthers,
      );
    }

    _playerId = (await _videoPlayerPlatform.create(dataSourceDescription)) ??
        kUninitializedPlayerId;
    _creatingCompleter!.complete(null);
    final Completer<void> initializingCompleter = Completer<void>();

    void eventListener(VideoEvent event) {
      if (_isDisposed) {
        return;
      }

      switch (event.eventType) {
        case VideoEventType.initialized:
        case VideoEventType.restored:
          value = value.copyWith(
            duration: event.duration,
            size: event.size,
            isInitialized: event.duration != null,
            errorDescription: null,
            isCompleted: false,
          );
          if (VideoEventType.initialized == event.eventType) {
            assert(
              !initializingCompleter.isCompleted,
              'VideoPlayerController already initialized. This is typically a '
              'sign that an implementation of the VideoPlayerPlatform '
              '(${_videoPlayerPlatform.runtimeType}) has a bug and is sending '
              'more than one initialized event per instance.',
            );
            if (initializingCompleter.isCompleted) {
              throw StateError('VideoPlayerController already initialized');
            }
            initializingCompleter.complete(null);
          }
          _applyLooping();
          // NOTE(jsuya): The plusplayer's SetVolume() work when player is
          // paused or played, so it changes the order of _applyPlayPause()
          // and _applyVolume().
          if (event.eventType == VideoEventType.restored &&
              _onRestoreDataSource != null) {
            play();
          } else {
            _applyPlayPause();
          }
          _applyVolume();
          _durationTimer?.cancel();
          _durationTimer = _createDurationTimer();
        case VideoEventType.completed:
          // In this case we need to stop _timer, set isPlaying=false, and
          // position=value.duration. Instead of setting the values directly,
          // we use pause() and seekTo() to ensure the platform stops playing
          // and seeks to the last frame of the video.
          pause().then((void pauseResult) => seekTo(value.duration.end));
          value = value.copyWith(
              isCompleted: true,
              textCaption: TextCaption.none,
              pictureCaption: PictureCaption.none);
          _durationTimer?.cancel();
        case VideoEventType.bufferingUpdate:
          value = value.copyWith(buffered: event.buffered);
        case VideoEventType.bufferingStart:
          value = value.copyWith(isBuffering: true);
        case VideoEventType.bufferingEnd:
          value = value.copyWith(isBuffering: false);
        case VideoEventType.subtitleUpdate:
          final Duration textDuration = event.textDuration == 0
              ? Duration.zero
              : Duration(milliseconds: event.textDuration!);
          if (event.picture?.isNotEmpty ?? false) {
            final PictureCaption pictureCaption = PictureCaption(
              number: 0,
              start: value.position,
              end: value.position + textDuration,
              picture: event.picture,
              pictureWidth: event.pictureWidth,
              pictureHeight: event.pictureHeight,
            );
            value = value.copyWith(pictureCaption: pictureCaption);
          } else {
            final List<SubtitleAttribute> subtitleAttributes =
                SubtitleAttribute.fromEventSubtitleAttrList(
              event.subtitleAttributes,
            );

            final (
              TextOriginAndExtent?,
              TextStyle?,
              AlignmentGeometry,
              Color,
              double
            ) subtitleAttr =
                TextCaption.processSubtitleAttributes(subtitleAttributes);
            final TextOriginAndExtent? actualTextOriginAndExtent =
                subtitleAttr.$1;
            final TextStyle? actualTextStyle = subtitleAttr.$2;
            final AlignmentGeometry actualTextAlign = subtitleAttr.$3;
            final Color actualWindowBgColor = subtitleAttr.$4;
            final double actualFontSize = subtitleAttr.$5;

            final TextCaption textCaption = TextCaption(
                number: 0,
                start: value.position,
                end: value.position + textDuration,
                text: event.text ?? '',
                subtitleAttributes: subtitleAttributes,
                textStyle: actualTextStyle,
                textOriginAndExtent: actualTextOriginAndExtent,
                textAlign: actualTextAlign,
                windowBgColor: actualWindowBgColor,
                fontSize: actualFontSize);
            value = value.copyWith(textCaption: textCaption);
          }
        case VideoEventType.isPlayingStateUpdate:
          if (event.isPlaying ?? false) {
            value = value.copyWith(
              isPlaying: event.isPlaying,
              isCompleted: false,
            );
            _timer ??= _createTimer();
          } else {
            value = value.copyWith(isPlaying: event.isPlaying);
          }
        case VideoEventType.adFromDash:
          final AdInfoFromDash? adInfo =
              AdInfoFromDash.fromAdInfoMap(event.adInfo);
          value = value.copyWith(adInfo: adInfo);
        case VideoEventType.manifestInfoUpdated:
          value = value.copyWith(manifestInfo: event.manifestInfo);
        case VideoEventType.unknown:
          break;
      }
    }

    if (closedCaptionFile != null) {
      _closedCaptionFile ??= await closedCaptionFile;
      value = value.copyWith(textCaption: _getCaptionAt(value.position));
    }

    if (drmConfigs?.licenseCallback != null) {
      _channel.setMethodCallHandler((MethodCall call) async {
        if (call.method == 'requestLicense') {
          final Map<dynamic, dynamic> argumentsMap =
              call.arguments as Map<dynamic, dynamic>;
          final Uint8List message = argumentsMap['message']! as Uint8List;
          return drmConfigs!.licenseCallback!(message);
        } else {
          throw Exception('not implemented ${call.method}');
        }
      });
    }

    void errorListener(Object obj) {
      final PlatformException e = obj as PlatformException;
      value = VideoPlayerValue.erroneous(e.message!);
      _timer?.cancel();
      _durationTimer?.cancel();
      if (!initializingCompleter.isCompleted) {
        initializingCompleter.completeError(obj);
      }
    }

    _eventSubscription = _videoPlayerPlatform
        .videoEventsFor(_playerId)
        .listen(eventListener, onError: errorListener);
    return initializingCompleter.future;
  }

  @override
  Future<void> dispose() async {
    if (_creatingCompleter != null) {
      await _creatingCompleter!.future;
      if (!_isDisposed) {
        _isDisposed = true;
        _timer?.cancel();
        _durationTimer?.cancel();
        await _eventSubscription?.cancel();
        await _videoPlayerPlatform.dispose(_playerId);
      }
      _lifeCycleObserver?.dispose();
    }
    _isDisposed = true;
    super.dispose();
  }

  /// Starts playing the video.
  ///
  /// If the video is at the end, this method starts playing from the beginning.
  ///
  /// This method returns a future that completes as soon as the "play" command
  /// has been sent to the platform, not when playback itself is totally
  /// finished.
  Future<void> play() async {
    if (value.position == value.duration.end) {
      await seekTo(Duration.zero);
    }
    value = value.copyWith(isPlaying: true);
    await _applyPlayPause();
  }

  /// Sets the video activated. Use it if create two native players.
  Future<bool> activate() async {
    return _applyActivate();
  }

  /// Sets the video deactivated. Use it if create two native players.
  Future<bool> deactivate() async {
    return _applyDeactivate();
  }

  /// Sets whether or not the video should loop after playing once. See also
  /// [VideoPlayerValue.isLooping].
  Future<void> setLooping(bool looping) async {
    value = value.copyWith(isLooping: looping);
    await _applyLooping();
  }

  /// Pauses the video.
  Future<void> pause() async {
    value = value.copyWith(isPlaying: false);
    await _applyPlayPause();
  }

  /// The duration in the current video.
  Future<DurationRange?> get duration async {
    if (_isDisposed || _durationTimer == null) {
      return null;
    }
    return _videoPlayerPlatform.getDuration(_playerId);
  }

  Timer _createDurationTimer() {
    return Timer.periodic(const Duration(milliseconds: 1000), (
      Timer timer,
    ) async {
      if (_isDisposed) {
        return;
      }
      final DurationRange? newDuration = await duration;
      if (newDuration == null) {
        return;
      }
      _updateDuration(newDuration);
    });
  }

  Future<void> _applyLooping() async {
    if (_isDisposedOrNotInitialized) {
      return;
    }
    await _videoPlayerPlatform.setLooping(_playerId, value.isLooping);
  }

  Future<bool> _applyActivate() async {
    if (_isDisposedOrNotInitialized) {
      return false;
    }
    return _videoPlayerPlatform.setActivate(_playerId);
  }

  Future<bool> _applyDeactivate() async {
    if (_isDisposedOrNotInitialized) {
      return false;
    }
    return _videoPlayerPlatform.setDeactivate(_playerId);
  }

  Future<void> _applyPlayPause() async {
    if (_isDisposedOrNotInitialized) {
      return;
    }
    if (value.isPlaying) {
      await _videoPlayerPlatform.play(_playerId);

      // Cancel previous timer.
      _timer?.cancel();
      _timer = _createTimer();
    } else {
      _timer?.cancel();
      await _videoPlayerPlatform.pause(_playerId);
    }
  }

  Future<void> _applyVolume() async {
    if (_isDisposedOrNotInitialized) {
      return;
    }
    await _videoPlayerPlatform.setVolume(_playerId, value.volume);
  }

  Future<void> _applyPlaybackSpeed() async {
    if (_isDisposedOrNotInitialized) {
      return;
    }

    await _videoPlayerPlatform.setPlaybackSpeed(_playerId, value.playbackSpeed);
  }

  /// The position in the current video.
  Future<Duration?> get position async {
    if (_isDisposed || _timer == null) {
      return null;
    }
    return _videoPlayerPlatform.getPosition(_playerId);
  }

  Timer _createTimer() {
    return Timer.periodic(const Duration(milliseconds: 500), (
      Timer timer,
    ) async {
      if (_isDisposed) {
        return;
      }
      final Duration? newPosition = await position;
      if (newPosition != null) {
        _updatePosition(newPosition);
      }
    });
  }

  /// Sets the video's current timestamp to be at [moment]. The next
  /// time the video is played it will resume from the given [moment].
  ///
  /// If [moment] is outside of the video's full range it will be automatically
  /// and silently clamped.
  Future<void> seekTo(Duration position) async {
    if (_isDisposedOrNotInitialized) {
      return;
    }
    if (position > value.duration.end) {
      position = value.duration.end;
    } else if (position < Duration.zero) {
      position = Duration.zero;
    }
    await _videoPlayerPlatform.seekTo(_playerId, position);
    _updatePosition(position);
  }

  /// The video tracks in the current video.
  Future<List<VideoTrack>?> get videoTracks async {
    if (!value.isInitialized || _isDisposed) {
      return null;
    }
    return _videoPlayerPlatform.getVideoTracks(_playerId);
  }

  /// The audio tracks in the current video.
  Future<List<AudioTrack>?> get audioTracks async {
    if (!value.isInitialized || _isDisposed) {
      return null;
    }
    return _videoPlayerPlatform.getAudioTracks(_playerId);
  }

  /// The text tracks in the current video.
  Future<List<TextTrack>?> get textTracks async {
    if (!value.isInitialized || _isDisposed) {
      return null;
    }
    return _videoPlayerPlatform.getTextTracks(_playerId);
  }

  /// Sets the selected tracks.
  Future<bool> setTrackSelection(Track track) async {
    if (!value.isInitialized || _isDisposed) {
      return false;
    }
    return _videoPlayerPlatform.setTrackSelection(_playerId, track);
  }

  /// Sets the audio volume of [this].
  ///
  /// [volume] indicates a value between 0.0 (silent) and 1.0 (full volume) on a
  /// linear scale.
  Future<void> setVolume(double volume) async {
    value = value.copyWith(volume: volume.clamp(0.0, 1.0));
    await _applyVolume();
  }

  /// Retrieves a specific property value obtained by the streaming engine (Smooth Streaming, HLS, DASH, or Widevine).
  Future<String> getStreamingProperty(StreamingPropertyType type) async {
    if (_isDisposedOrNotInitialized) {
      return '';
    }

    final Future<String> streamingProperty =
        _videoPlayerPlatform.getStreamingProperty(_playerId, type);
    await streamingProperty.then((String result) {
      // ignore: avoid_print
      print('[getStreamingProperty()] type: $type, result: $result');
    });

    return streamingProperty;
  }

  /// Sets specific feature values for HTTP, MMS, or specific streaming engine (Smooth Streaming, HLS, DASH, DivX Plus Streaming, or Widevine).
  /// The available streaming properties depend on the streaming protocol or engine.
  /// Use the CUSTOM_MESSAGE property for streaming engine or CP-specific settings.
  Future<void> setStreamingProperty(
    StreamingPropertyType type,
    String value,
  ) async {
    if (_isDisposedOrNotInitialized) {
      return;
    }
    if (formatHint != VideoFormat.dash &&
        (type == StreamingPropertyType.dashToken ||
            type == StreamingPropertyType.openHttpHeader)) {
      throw Exception(
          'setStreamingProperty().$type only support for dash format!');
    }
    return _videoPlayerPlatform.setStreamingProperty(_playerId, type, value);
  }

  /// Sets the buffer size for the play and resume scenarios. The time buffer size must be at least 4 seconds.
  /// For example, if a 10 second buffer size is set, playback can only start or resume after 10 seconds of media has accumulated in the buffer.
  /// Play scenarios include user-initiated streaming playback and whenever media playback is starting for the first time.
  /// Resume scenarios include resuming playback after pause or seek operations, or when lack of data causes playback rebuffering.
  Future<bool> setBufferConfig(BufferConfigType type, int value) async {
    if (_isDisposedOrNotInitialized) {
      return false;
    }
    return _videoPlayerPlatform.setBufferConfig(_playerId, type, value);
  }

  /// Set the rotate angle of display
  Future<bool> setDisplayRotate(DisplayRotation rotation) async {
    if (_isDisposedOrNotInitialized) {
      return false;
    }

    return _videoPlayerPlatform.setDisplayRotate(_playerId, rotation);
  }

  /// Set the video display mode.
  Future<bool> setDisplayMode(DisplayMode displayMode) async {
    if (_isDisposedOrNotInitialized) {
      return false;
    }

    return _videoPlayerPlatform.setDisplayMode(_playerId, displayMode);
  }

  /// [optional]Set the restoreDataSource and resumeTime of video.
  /// For live streaming or DRM-encrypted content playback, you must check whether the
  /// streaming URL has changed or the DRM session or license has expired, and specify
  /// the new URL and DRM information as needed.
  ///
  /// * restoreDataSource[optional][nullable]: Optional updated restoreDataSource after
  ///   suspend, includes elements such as the new URL and DRM information.
  ///   If null, the dataSource stored at 'initailize()' is used.
  ///   For live streaming or DRM-encrypted content playback, in case the URL has changed
  ///   or the DRM license or session has expired, checking for and passing the newest URL
  ///   is recommended.
  /// * resumeTime[optional][default=-1]: (milliseconds) Optional position from which to
  ///   resume playback in three scenarios: the streaming type is live, power off/on
  ///   within 5 seconds, or changing the URL(that is, restoreDataSource is non-null).
  ///   If less than 0, the position stored at '_suspend()' is used.
  void setRestoreData({
    RestoreDataSourceCallback? restoreDataSource,
    RestoreTimeCallback? resumeTime,
  }) {
    _onRestoreDataSource = restoreDataSource;
    _onRestoreTime = resumeTime;
  }

  /// Pauses the player when the application is sent to the background.
  /// Saves the current statistics for the ongoing playback session.
  Future<void> _suspend() async {
    if (_isDisposedOrNotInitialized) {
      return;
    }

    await _videoPlayerPlatform.suspend(_playerId);
    _durationTimer?.cancel();
    _timer?.cancel();
    _timer = null;
  }

  /// Restores the player state when the application is resumed.
  Future<void> _restore() async {
    if (_isDisposedOrNotInitialized) {
      return;
    }

    final DataSource? dataSource =
        (_onRestoreDataSource != null) ? _onRestoreDataSource!() : null;
    final int resumeTime = (_onRestoreTime != null) ? _onRestoreTime!() : -1;

    await _videoPlayerPlatform.restore(
      _playerId,
      dataSource: dataSource,
      resumeTime: resumeTime,
    );
  }

  /// Set dashplusplayer properties,can be called after initialized.
  Future<bool> setData(Map<DashPlayerProperty, Object> data) async {
    if (_isDisposedOrNotInitialized) {
      return false;
    }

    return _videoPlayerPlatform.setData(playerId, data);
  }

  /// Get dashplusplayer properties,can be called after initialized.
  Future<Map<DashPlayerProperty, Object>> getData(
    Set<DashPlayerProperty> keys,
  ) async {
    if (_isDisposedOrNotInitialized) {
      return <DashPlayerProperty, Object>{};
    }

    final Future<Map<DashPlayerProperty, Object>> data =
        _videoPlayerPlatform.getData(playerId, keys);
    await data.then((Object result) {
      // ignore: avoid_print
      print('[getData()] result: \n$result');
    });

    return data;
  }

  /// Updates the authentication token for the current DASH video stream.
  ///
  /// This method is used for DASH stream that requires token-based authentication.
  /// The server hosting the DASH stream may have token validation enabled, requiring
  /// a valid token to be included in the request URL to access the video segments.
  ///
  /// The [dashToken] parameter should contain the new authentication token string.
  /// When this API is called, it will dynamically update the token used for
  /// subsequent network requests. In the current implementation, the URL associated
  /// with the video source must contain a placeholder segment in the format
  /// "token=XXX". Calling this API will replace the "XXX" part of the placeholder
  /// with the new [dashToken] value.
  ///
  /// This method can only be called after the video player has been initialized
  /// and is currently playing a DASH format video. An exception will be thrown if
  /// called under other video formats.
  ///
  /// Returns `true` if the token was successfully updated, `false` otherwise.
  Future<bool> updateDashToken(String dashToken) async {
    if (_isDisposedOrNotInitialized) {
      return false;
    }

    if (formatHint != VideoFormat.dash) {
      throw Exception('updateDashToken() only support for dash format!');
    }

    return _videoPlayerPlatform.updateDashToken(playerId, dashToken);
  }

  /// Get activated(selected) track infomation of the associated media.
  Future<List<Track>> getActiveTrackInfo() async {
    if (_isDisposedOrNotInitialized) {
      return <Track>[];
    }

    return _videoPlayerPlatform.getActiveTrackInfo(playerId);
  }

  /// Sets the playback speed of [this].
  ///
  /// [speed] indicates a speed value with different platforms accepting
  /// different ranges for speed values. The [speed] must be greater than 0.
  ///
  /// The values will be handled as follows:
  /// * On web, the audio will be muted at some speed when the browser
  ///   determines that the sound would not be useful anymore. For example,
  ///   "Gecko mutes the sound outside the range `0.25` to `5.0`" (see https://developer.mozilla.org/en-US/docs/Web/API/HTMLMediaElement/playbackRate).
  /// * On Android, some very extreme speeds will not be played back accurately.
  ///   Instead, your video will still be played back, but the speed will be
  ///   clamped by ExoPlayer (but the values are allowed by the player, like on
  ///   web).
  /// * On iOS, you can sometimes not go above `2.0` playback speed on a video.
  ///   An error will be thrown for if the option is unsupported. It is also
  ///   possible that your specific video cannot be slowed down, in which case
  ///   the plugin also reports errors.
  Future<void> setPlaybackSpeed(double speed) async {
    if (speed < 0) {
      throw ArgumentError.value(
        speed,
        'Negative playback speeds are generally unsupported.',
      );
    } else if (speed == 0) {
      throw ArgumentError.value(
        speed,
        'Zero playback speed is generally unsupported. Consider using [pause].',
      );
    }

    value = value.copyWith(playbackSpeed: speed);
    await _applyPlaybackSpeed();
  }

  /// Sets the caption offset.
  ///
  /// The [offset] will be used when getting the correct caption for a specific position.
  /// The [offset] can be positive or negative.
  ///
  /// The values will be handled as follows:
  /// *  0: This is the default behaviour. No offset will be applied.
  /// * >0: The caption will have a negative offset. So you will get caption text from the past.
  /// * <0: The caption will have a positive offset. So you will get caption text from the future.
  void setCaptionOffset(Duration offset) {
    value = value.copyWith(
      captionOffset: offset,
      textCaption: _getCaptionAt(value.position),
    );
  }

  /// The closed caption based on the current [position] in the video.
  ///
  /// If there are no closed captions at the current [position], this will
  /// return an empty [Caption].
  ///
  /// If no [closedCaptionFile] was specified, this will always return an empty
  /// [Caption].
  TextCaption _getCaptionAt(Duration position) {
    if (_closedCaptionFile == null) {
      return position > value.textCaption.end
          ? TextCaption.none
          : value.textCaption;
    }

    final Duration delayedPosition = position + value.captionOffset;
    // TODO(johnsonmh): This would be more efficient as a binary search.
    for (final TextCaption textCaption in _closedCaptionFile!.textCaptions) {
      if (textCaption.start <= delayedPosition &&
          textCaption.end >= delayedPosition) {
        return textCaption;
      }
    }

    return TextCaption.none;
  }

  PictureCaption _getPictureCaptionAt(Duration position) {
    return position > value.pictureCaption.end
        ? PictureCaption.none
        : value.pictureCaption;
  }

  void _updatePosition(Duration position) {
    value = value.copyWith(
      position: position,
      textCaption: _getCaptionAt(position),
      pictureCaption: _getPictureCaptionAt(position),
      isCompleted: position == value.duration.end,
    );
  }

  void _updateDuration(DurationRange duration) {
    value = value.copyWith(duration: duration);
  }

  @override
  void removeListener(VoidCallback listener) {
    // Prevent VideoPlayer from causing an exception to be thrown when attempting to
    // remove its own listener after the controller has already been disposed.
    if (!_isDisposed) {
      super.removeListener(listener);
    }
  }

  bool get _isDisposedOrNotInitialized => _isDisposed || !value.isInitialized;
}

class _VideoAppLifeCycleObserver extends Object with WidgetsBindingObserver {
  _VideoAppLifeCycleObserver(this._controller);

  final VideoPlayerController _controller;

  void initialize() {
    _ambiguate(WidgetsBinding.instance)!.addObserver(this);
  }

  @override
  void didChangeAppLifecycleState(AppLifecycleState state) {
    if (state == AppLifecycleState.paused) {
      _controller._suspend();
    } else if (state == AppLifecycleState.resumed) {
      _controller._restore();
    }
  }

  void dispose() {
    _ambiguate(WidgetsBinding.instance)!.removeObserver(this);
  }
}

/// Widget that displays the video controlled by [controller].
class VideoPlayer extends StatefulWidget {
  /// Uses the given [controller] for all video rendered in this widget.
  const VideoPlayer(this.controller, {super.key, this.scale = 1.0});

  /// The [VideoPlayerController] responsible for the video being rendered in
  /// this widget.
  final VideoPlayerController controller;

  /// Scales the VideoPlayer widget size.
  final double scale;

  @override
  State<VideoPlayer> createState() => _VideoPlayerState();
}

class _VideoPlayerState extends State<VideoPlayer> {
  _VideoPlayerState() {
    _listener = () {
      final int newPlayerId = widget.controller.playerId;
      if (newPlayerId != _playerId) {
        setState(() {
          _playerId = newPlayerId;
        });
      }
    };
  }

  MethodChannel windowChannel = const MethodChannel('tizen/internal/window');

  late VoidCallback _listener;

  late int _playerId;

  final GlobalKey _videoBoxKey = GlobalKey();
  Rect _playerRect = Rect.zero;

  Orientation? _playerOrientation;

  @override
  void initState() {
    super.initState();
    _playerId = widget.controller.playerId;
    // Need to listen for initialization events since the actual player ID
    // becomes available after asynchronous initialization finishes.
    widget.controller.addListener(_listener);

    WidgetsBinding.instance.addPostFrameCallback(_afterFrameLayout);
  }

  bool _isInvalid(double value) {
    return value.isInfinite || value.isNaN;
  }

  void _afterFrameLayout(_) {
    if (widget.controller.value.isInitialized) {
      final Rect rect = _getCurrentRect();
      if (rect != Rect.zero && _playerRect != rect) {
        final double offsetLeft = rect.left - rect.left.floor();
        final double offsetTop = rect.top - rect.top.floor();
        final double offsetWidth = rect.width.ceil() - rect.width;
        final double offsetHeight = rect.height.ceil() - rect.height;
        final int left = _isInvalid(rect.left) ? 0 : rect.left.floor();
        final int top = _isInvalid(rect.top) ? 0 : rect.top.floor();
        final int width = _isInvalid(rect.width)
            ? 1
            : rect.width.ceil() + ((offsetLeft > offsetWidth) ? 1 : 0);
        final int height = _isInvalid(rect.height)
            ? 1
            : rect.height.ceil() + ((offsetTop > offsetHeight) ? 1 : 0);

        _videoPlayerPlatform.setDisplayGeometry(
            _playerId, left, top, width, height);
        _playerRect = rect;
      }
    }
    WidgetsBinding.instance.addPostFrameCallback(_afterFrameLayout);
  }

  Rect _getCurrentRect() {
    final RenderObject? renderObject =
        _videoBoxKey.currentContext?.findRenderObject();
    if (renderObject == null) {
      return Rect.zero;
    }
    // ignore: deprecated_member_use
    final double pixelRatio = WidgetsBinding.instance.window.devicePixelRatio;
    final RenderBox renderBox = renderObject as RenderBox;
    final Offset offset = renderBox.localToGlobal(Offset.zero) * pixelRatio;
    final Size size = renderBox.size * pixelRatio * widget.scale;
    return offset & size;
  }

  @override
  void didUpdateWidget(VideoPlayer oldWidget) {
    super.didUpdateWidget(oldWidget);
    oldWidget.controller.removeListener(_listener);
    _playerId = widget.controller.playerId;
    widget.controller.addListener(_listener);
  }

  @override
  void deactivate() {
    super.deactivate();
    widget.controller.removeListener(_listener);
  }

  Future<void> _updatePlayerOrientation() async {
    final Orientation orientation = MediaQuery.orientationOf(context);
    int? rotate;
    if (widget.controller.value.isInitialized &&
        (_playerOrientation == null || _playerOrientation != orientation)) {
      rotate = await windowChannel.invokeMethod('getRotation');
      _playerOrientation = orientation;
    }

    if (rotate == null) {
      return;
    }

    if (rotate == 90) {
      await widget.controller.setDisplayRotate(DisplayRotation.rotation270);
    } else if (rotate == 180) {
      await widget.controller.setDisplayRotate(DisplayRotation.rotation180);
    } else if (rotate == 270) {
      await widget.controller.setDisplayRotate(DisplayRotation.rotation90);
    } else {
      await widget.controller.setDisplayRotate(DisplayRotation.rotation0);
    }
  }

  @override
  Widget build(BuildContext context) {
    _updatePlayerOrientation();
    return Container(key: _videoBoxKey, child: const Hole());
  }
}

/// Used to configure the [VideoProgressIndicator] widget's colors for how it
/// describes the video's status.
///
/// The widget uses default colors that are customizeable through this class.
class VideoProgressColors {
  /// Any property can be set to any color. They each have defaults.
  ///
  /// [playedColor] defaults to red at 70% opacity. This fills up a portion of
  /// the [VideoProgressIndicator] to represent how much of the video has played
  /// so far.
  ///
  /// [bufferedColor] defaults to blue at 20% opacity. This fills up a portion
  /// of [VideoProgressIndicator] to represent how much of the video has
  /// buffered so far.
  ///
  /// [backgroundColor] defaults to gray at 50% opacity. This is the background
  /// color behind both [playedColor] and [bufferedColor] to denote the total
  /// size of the video compared to either of those values.
  const VideoProgressColors({
    this.playedColor = const Color.fromRGBO(255, 0, 0, 0.7),
    this.bufferedColor = const Color.fromRGBO(50, 50, 200, 0.2),
    this.backgroundColor = const Color.fromRGBO(200, 200, 200, 0.5),
  });

  /// [playedColor] defaults to red at 70% opacity. This fills up a portion of
  /// the [VideoProgressIndicator] to represent how much of the video has played
  /// so far.
  final Color playedColor;

  /// [bufferedColor] defaults to blue at 20% opacity. This fills up a portion
  /// of [VideoProgressIndicator] to represent how much of the video has
  /// buffered so far.
  final Color bufferedColor;

  /// [backgroundColor] defaults to gray at 50% opacity. This is the background
  /// color behind both [playedColor] and [bufferedColor] to denote the total
  /// size of the video compared to either of those values.
  final Color backgroundColor;
}

class _VideoScrubber extends StatefulWidget {
  const _VideoScrubber({required this.child, required this.controller});

  final Widget child;
  final VideoPlayerController controller;

  @override
  _VideoScrubberState createState() => _VideoScrubberState();
}

class _VideoScrubberState extends State<_VideoScrubber> {
  bool _controllerWasPlaying = false;

  VideoPlayerController get controller => widget.controller;

  @override
  Widget build(BuildContext context) {
    void seekToRelativePosition(Offset globalPosition) {
      final RenderBox box = context.findRenderObject()! as RenderBox;
      final Offset tapPos = box.globalToLocal(globalPosition);
      final double relative = tapPos.dx / box.size.width;
      final Duration position = controller.value.duration.end * relative;
      controller.seekTo(position);
    }

    return GestureDetector(
      behavior: HitTestBehavior.opaque,
      child: widget.child,
      onHorizontalDragStart: (DragStartDetails details) {
        if (!controller.value.isInitialized) {
          return;
        }
        _controllerWasPlaying = controller.value.isPlaying;
        if (_controllerWasPlaying) {
          controller.pause();
        }
      },
      onHorizontalDragUpdate: (DragUpdateDetails details) {
        if (!controller.value.isInitialized) {
          return;
        }
        seekToRelativePosition(details.globalPosition);
      },
      onHorizontalDragEnd: (DragEndDetails details) {
        if (_controllerWasPlaying &&
            controller.value.position != controller.value.duration.end) {
          controller.play();
        }
      },
      onTapDown: (TapDownDetails details) {
        if (!controller.value.isInitialized) {
          return;
        }
        seekToRelativePosition(details.globalPosition);
      },
    );
  }
}

/// Displays the play/buffering status of the video controlled by [controller].
///
/// If [allowScrubbing] is true, this widget will detect taps and drags and
/// seek the video accordingly.
///
/// [padding] allows to specify some extra padding around the progress indicator
/// that will also detect the gestures.
class VideoProgressIndicator extends StatefulWidget {
  /// Construct an instance that displays the play/buffering status of the video
  /// controlled by [controller].
  ///
  /// Defaults will be used for everything except [controller] if they're not
  /// provided. [allowScrubbing] defaults to false, and [padding] will default
  /// to `top: 5.0`.
  const VideoProgressIndicator(
    this.controller, {
    super.key,
    this.colors = const VideoProgressColors(),
    required this.allowScrubbing,
    this.padding = const EdgeInsets.only(top: 5.0),
  });

  /// The [VideoPlayerController] that actually associates a video with this
  /// widget.
  final VideoPlayerController controller;

  /// The default colors used throughout the indicator.
  ///
  /// See [VideoProgressColors] for default values.
  final VideoProgressColors colors;

  /// When true, the widget will detect touch input and try to seek the video
  /// accordingly. The widget ignores such input when false.
  ///
  /// Defaults to false.
  final bool allowScrubbing;

  /// This allows for visual padding around the progress indicator that can
  /// still detect gestures via [allowScrubbing].
  ///
  /// Defaults to `top: 5.0`.
  final EdgeInsets padding;

  @override
  State<VideoProgressIndicator> createState() => _VideoProgressIndicatorState();
}

class _VideoProgressIndicatorState extends State<VideoProgressIndicator> {
  _VideoProgressIndicatorState() {
    listener = () {
      if (!mounted) {
        return;
      }
      setState(() {});
    };
  }

  late VoidCallback listener;

  VideoPlayerController get controller => widget.controller;

  VideoProgressColors get colors => widget.colors;

  @override
  void initState() {
    super.initState();
    controller.addListener(listener);
  }

  @override
  void deactivate() {
    controller.removeListener(listener);
    super.deactivate();
  }

  @override
  Widget build(BuildContext context) {
    Widget progressIndicator;
    if (controller.value.isInitialized) {
      final int duration = controller.value.duration.end.inMilliseconds;
      final int position = controller.value.position.inMilliseconds;

      progressIndicator = Stack(
        fit: StackFit.passthrough,
        children: <Widget>[
          LinearProgressIndicator(
            value: duration != 0 ? position / duration : 0,
            valueColor: AlwaysStoppedAnimation<Color>(colors.playedColor),
            backgroundColor: Colors.transparent,
          ),
        ],
      );
    } else {
      progressIndicator = LinearProgressIndicator(
        valueColor: AlwaysStoppedAnimation<Color>(colors.playedColor),
        backgroundColor: colors.backgroundColor,
      );
    }
    final Widget paddedProgressIndicator = Padding(
      padding: widget.padding,
      child: progressIndicator,
    );
    if (widget.allowScrubbing) {
      return _VideoScrubber(
        controller: controller,
        child: paddedProgressIndicator,
      );
    } else {
      return paddedProgressIndicator;
    }
  }
}

/// Widget for displaying closed captions on top of a video.
///
/// If [text] is null, this widget will not display anything.
///
/// If [textStyle] is supplied, it will be used to style the text in the closed
/// caption.
///
/// Note: in order to have closed captions, you need to specify a
/// [VideoPlayerController.closedCaptionFile].
///
/// Usage:
///
/// ```dart
/// Stack(children: <Widget>[
///   VideoPlayer(_controller),
///   ClosedCaption(text: _controller.value.caption.text),
/// ]),
/// ```
class ClosedCaption extends StatelessWidget {
  /// Creates a a new closed caption, designed to be used with
  /// [VideoPlayerValue.textCaption] and [VideoPlayerValue.pictureCaption].
  ///
  /// If [text] is null or empty, nothing will be displayed.
  const ClosedCaption(
      {super.key, this.textCaption, this.pictureCaption, this.customTextStyle});

  /// The text that will be shown in the closed caption, or null if no caption
  /// should be shown.
  /// If the text is empty the caption will not be shown.
  final TextCaption? textCaption;

  /// The picture that will be shown in the closed caption, or null.
  final PictureCaption? pictureCaption;

  /// Users can use it to customize the subtitle style.
  final TextStyle? customTextStyle;

  Widget _buildTextSubtitle({
    required String text,
    required TextStyle textStyle,
    Color? backgroundColor,
  }) {
    return Align(
      alignment: Alignment.bottomCenter,
      child: Padding(
        padding: const EdgeInsets.only(bottom: 24.0),
        child: DecoratedBox(
          decoration: BoxDecoration(
            color: backgroundColor ?? const Color(0xB8000000),
            borderRadius: BorderRadius.circular(2.0),
          ),
          child: Padding(
            padding: const EdgeInsets.symmetric(horizontal: 2.0),
            child: Text(text, style: textStyle),
          ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    if (pictureCaption?.picture?.isNotEmpty ?? false) {
      final Image subtitleImage = Image.memory(pictureCaption!.picture!,
          width: pictureCaption!.pictureWidth,
          height: pictureCaption!.pictureHeight, errorBuilder:
              (BuildContext context, Object error, StackTrace? stackTrace) {
        // ignore: avoid_print
        print('[ClosedCaption] Image.memory error: $error');
        // ignore: avoid_print
        print('[ClosedCaption] StackTrace: $stackTrace');

        return const Text('');
      });
      return Align(
          alignment: Alignment.bottomCenter,
          child: Padding(
            padding: const EdgeInsets.only(bottom: 24.0),
            child: subtitleImage,
          ));
    } else {
      final String? text = textCaption?.text;
      if (text == null || text.isEmpty) {
        return const SizedBox.shrink();
      }

      if (customTextStyle != null) {
        return _buildTextSubtitle(text: text, textStyle: customTextStyle!);
      }

      TextStyle effectiveTextStyle = textCaption?.textStyle ??
          DefaultTextStyle.of(
            context,
          ).style.copyWith(fontSize: 36.0, color: Colors.white);

      if (textCaption?.textOriginAndExtent != null) {
        final RegExp newLineRegex = RegExp(r'\r\n|\r|\n');
        final int textLine = newLineRegex.allMatches(text).length + 1;
        // ignore: avoid_print
        print('****************textLine is $textLine************************');

        return Positioned.fill(
          child: Align(
            alignment: Alignment(
              textCaption!.textOriginAndExtent!.originX,
              textCaption!.textOriginAndExtent!.originY,
            ),
            child: LayoutBuilder(
              builder: (BuildContext context, BoxConstraints constraints) {
                final double actualHeight = constraints.maxHeight *
                    (textCaption?.textOriginAndExtent?.extentHeight ?? 1.0);

                final double dynamicFontSize =
                    actualHeight / textLine * (textCaption?.fontSize ?? 1.0);

                effectiveTextStyle =
                    effectiveTextStyle.copyWith(fontSize: dynamicFontSize);

                return FractionallySizedBox(
                  widthFactor: textCaption?.textOriginAndExtent?.extentWidth,
                  heightFactor: textCaption?.textOriginAndExtent?.extentHeight,
                  child: ColoredBox(
                    color:
                        textCaption?.windowBgColor ?? const Color(0xB8000000),
                    child: Align(
                        alignment: textCaption?.textAlign ?? Alignment.center,
                        child: Text(text, style: effectiveTextStyle)),
                  ),
                );
              },
            ),
          ),
        );
      } else {
        effectiveTextStyle = effectiveTextStyle.copyWith(fontSize: 36.0);
        return _buildTextSubtitle(
            text: text,
            textStyle: effectiveTextStyle,
            backgroundColor: textCaption?.windowBgColor);
      }
    }
  }
}

/// This allows a value of type T or T? to be treated as a value of type T?.
///
/// We use this so that APIs that have become non-nullable can still be used
/// with `!` and `?` on the stable branch.
// TODO(ianh): Remove this once we roll stable in late 2021.
T? _ambiguate<T>(T? value) => value;
