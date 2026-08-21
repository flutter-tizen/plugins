// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:isolate' show RawReceivePort;

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import '../video_player_platform_interface.dart';
import 'ffi_messages.g.dart';
import 'tracks.dart';

/// An implementation of [VideoPlayerPlatform] that uses FFI for all methods.
class VideoPlayerTizen extends VideoPlayerPlatform {
  /// Create a new VideoPlayerTizen instance.
  VideoPlayerTizen() : super();

  final VideoPlayerVideoholeFFIApi _ffiApi = VideoPlayerVideoholeFFIApi();

  @override
  Future<void> init() async {
    // Use FFI for initialization (synchronous call)
    final int result = _ffiApi.initialize();
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_INITIALIZE_FAILED',
        message: 'FFI initialize failed with code: $result',
      );
    }
  }

  @override
  Future<void> dispose(int playerId) async {
    // Close the StreamController for this player
    final StreamController<VideoEvent>? controller = _eventControllers.remove(
      playerId,
    );
    if (controller != null && !controller.isClosed) {
      await controller.close();
    }

    // Use FFI for dispose (synchronous call)
    final int result = _ffiApi.dispose(playerId);
    // Don't throw on error - just return
    // The player may already be disposed or in an invalid state
    if (result != 0) {
      return;
    }
  }

  @override
  Future<int?> create(DataSource dataSource) async {
    // Two-phase initialization:
    // Phase 1: Create player without starting prepare
    // Phase 2: Register event port and start listening BEFORE calling prepare()

    // Ensure the global event port is registered
    _ensureEventPortRegistered();

    // Use CreateMessage class for FFI create (synchronous call)
    final CreateMessage message = CreateMessage();

    switch (dataSource.sourceType) {
      case DataSourceType.asset:
        message.asset = dataSource.asset;
        message.packageName = dataSource.package;
      case DataSourceType.network:
        message.uri = dataSource.uri;
        message.formatHint = _videoFormatStringMap[dataSource.formatHint];
        message.httpHeaders = dataSource.httpHeaders;
        message.drmConfigs = dataSource.drmConfigs?.toMap();
        message.playerOptions = dataSource.playerOptions;
      case DataSourceType.file:
      case DataSourceType.contentUri:
        message.uri = dataSource.uri;
    }

    // Phase 1: Create player (does NOT start prepare_async)
    final int playerId = _ffiApi.create(message);

    if (playerId < 0) {
      throw PlatformException(
        code: 'FFI_CREATE_FAILED',
        message: 'FFI create failed with code: $playerId',
      );
    }

    // Phase 2: Register global Dart port (only needs to be done once)
    // Note: registerDartPort is now a no-op since we use global port
    // The port is already registered in _ensureEventPortRegistered()

    return playerId;
  }

  @override
  Future<void> prepare(int playerId) async {
    final int result = _ffiApi.prepare(playerId);
    if (result < 0) {
      throw PlatformException(
        code: 'FFI_PREPARE_FAILED',
        message: 'FFI prepare failed with code: $result',
      );
    }
  }

  /// Ensure the event port is registered before any events are sent
  void _ensureEventPortRegistered() {
    // Always rebind the event port for this instance to ensure events
    // are routed to the correct _eventControllers map.
    // This fixes the issue where VideoPlayerPlatform.instance replacement
    // would cause events to be routed to the old instance's controllers.
    if (_eventPort == null) {
      // Initialize Dart API DL before using Dart_PostCObject_DL
      ffiInitializeApiDL();

      _eventPort = RawReceivePort();

      // Listen to FFI events and route them to this instance's StreamControllers
      _eventPort!.handler = (dynamic message) {
        try {
          // Message format from C++: [player_id, event_json_string]
          if (message is List && message.length == 2) {
            final int receivingPlayerId = message[0] as int;
            final String eventJson = message[1] as String;

            // Parse JSON to Map
            final Map<String, dynamic> eventMap =
                jsonDecode(eventJson) as Map<String, dynamic>;

            // Route to the correct StreamController for this instance
            final StreamController<VideoEvent>? controller =
                _eventControllers[receivingPlayerId];
            if (controller != null && !controller.isClosed) {
              // Handle error events by adding them as stream errors
              if (eventMap['event'] == 'error') {
                controller.addError(
                  PlatformException(
                    code: eventMap['code'] as String? ?? 'unknown',
                    message: eventMap['message'] as String?,
                  ),
                );
                return;
              }
              final VideoEvent videoEvent = _parseVideoEventFromMap(eventMap);
              controller.add(videoEvent);
            }
          }
        } catch (e, stackTrace) {
          // Log error but don't crash
          debugPrint('Error processing FFI event: $e\n$stackTrace');
        }
      };

      // Register the port with C++ side using FFI
      ffiRegisterEventPort(_eventPort!.nativePort);
      debugPrint('Event port registered: ${_eventPort!.nativePort}');
    }
  }

  @override
  Future<void> setLooping(int playerId, bool looping) async {
    // Use FFI for setLooping (synchronous call)
    final int result = _ffiApi.setLooping(playerId, looping);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_LOOPING_FAILED',
        message: 'FFI setLooping failed with code: $result',
      );
    }
  }

  @override
  Future<void> play(int playerId) async {
    // Use FFI for play (synchronous call)
    final int result = _ffiApi.play(playerId);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_PLAY_FAILED',
        message: 'FFI play failed with code: $result',
      );
    }
  }

  @override
  Future<bool> setActivate(int playerId) async {
    // Use FFI for setActivate (synchronous call)
    final int result = _ffiApi.setActivate(playerId);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_ACTIVATE_FAILED',
        message: 'FFI setActivate failed with code: $result',
      );
    }
    return true;
  }

  @override
  Future<bool> setDeactivate(int playerId) async {
    // Use FFI for setDeactivate (synchronous call)
    final int result = _ffiApi.setDeactivate(playerId);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_DEACTIVATE_FAILED',
        message: 'FFI setDeactivate failed with code: $result',
      );
    }
    return true;
  }

  @override
  Future<void> pause(int playerId) async {
    // Use FFI for pause (synchronous call)
    final int result = _ffiApi.pause(playerId);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_PAUSE_FAILED',
        message: 'FFI pause failed with code: $result',
      );
    }
  }

  @override
  Future<void> setVolume(int playerId, double volume) async {
    // Use FFI for setVolume (synchronous call)
    final int result = _ffiApi.setVolume(playerId, volume);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_VOLUME_FAILED',
        message: 'FFI setVolume failed with code: $result',
      );
    }
  }

  @override
  Future<void> setPlaybackSpeed(int playerId, double speed) async {
    // Use FFI for setPlaybackSpeed (synchronous call)
    assert(speed > 0);
    final int result = _ffiApi.setPlaybackSpeed(playerId, speed);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_PLAYBACK_SPEED_FAILED',
        message: 'FFI setPlaybackSpeed failed with code: $result',
      );
    }
  }

  @override
  Future<void> seekTo(int playerId, Duration position) async {
    // Use FFI for seekTo (synchronous call)
    final int result = _ffiApi.seekTo(playerId, position.inMilliseconds);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SEEK_TO_FAILED',
        message: 'FFI seekTo failed with code: $result',
      );
    }
  }

  @override
  Future<List<VideoTrack>> getVideoTracks(int playerId) async {
    final TrackMessage message = _ffiApi.getTrackInfo(playerId, 'video');

    final List<VideoTrack> videoTracks = <VideoTrack>[];
    for (final Map<Object?, Object?>? trackMap in message.tracks) {
      final int trackId = trackMap!['trackId']! as int;
      final int bitrate = trackMap['bitrate']! as int;
      final int width = trackMap['width']! as int;
      final int height = trackMap['height']! as int;

      videoTracks.add(
        VideoTrack(
          trackId: trackId,
          width: width,
          height: height,
          bitrate: bitrate,
        ),
      );
    }

    return videoTracks;
  }

  @override
  Future<List<AudioTrack>> getAudioTracks(int playerId) async {
    final TrackMessage message = _ffiApi.getTrackInfo(playerId, 'audio');

    final List<AudioTrack> audioTracks = <AudioTrack>[];
    for (final Map<Object?, Object?>? trackMap in message.tracks) {
      final int trackId = trackMap!['trackId']! as int;
      final String language = trackMap['language']! as String;
      final int channel = trackMap['channel']! as int;
      final int bitrate = trackMap['bitrate']! as int;

      audioTracks.add(
        AudioTrack(
          trackId: trackId,
          language: language,
          channel: channel,
          bitrate: bitrate,
        ),
      );
    }

    return audioTracks;
  }

  @override
  Future<List<TextTrack>> getTextTracks(int playerId) async {
    final TrackMessage message = _ffiApi.getTrackInfo(playerId, 'text');

    final List<TextTrack> textTracks = <TextTrack>[];
    for (final Map<Object?, Object?>? trackMap in message.tracks) {
      final int trackId = trackMap!['trackId']! as int;
      final String language = trackMap['language']! as String;

      textTracks.add(TextTrack(trackId: trackId, language: language));
    }

    return textTracks;
  }

  @override
  Future<bool> setTrackSelection(int playerId, Track track) async {
    // Use FFI for setTrackSelection (synchronous call)
    final int result = _ffiApi.setTrackSelection(
      playerId,
      track.trackId,
      track.trackType.name,
    );
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_TRACK_SELECTION_FAILED',
        message: 'FFI setTrackSelection failed with code: $result',
      );
    }
    return true;
  }

  @override
  Future<DurationRange> getDuration(int playerId) async {
    // Use FFI for getDuration (synchronous call)
    final DurationMessage message = _ffiApi.duration(playerId);
    return DurationRange(
      Duration(milliseconds: message.durationRange?[0] ?? 0),
      Duration(milliseconds: message.durationRange?[1] ?? 0),
    );
  }

  @override
  Future<Duration> getPosition(int playerId) async {
    // Use FFI for getPosition (synchronous call)
    final int positionMs = _ffiApi.getPosition(playerId);
    if (positionMs < 0) {
      throw PlatformException(
        code: 'FFI_GET_POSITION_FAILED',
        message: 'FFI getPosition failed with code: $positionMs',
      );
    }
    return Duration(milliseconds: positionMs);
  }

  // Instance RawReceivePort for FFI event notifications (not static!)
  // Using instance port ensures that when VideoPlayerPlatform.instance
  // is replaced, the new instance has its own port that routes events
  // to its own _eventControllers map.
  RawReceivePort? _eventPort;

  // Map of playerId to StreamController for broadcasting events
  final Map<int, StreamController<VideoEvent>> _eventControllers =
      <int, StreamController<VideoEvent>>{};

  @override
  Stream<VideoEvent> videoEventsFor(int playerId) {
    _ensureEventPortRegistered();

    // Return the stream for this specific player
    return _eventControllers
        .putIfAbsent(playerId, () => StreamController<VideoEvent>.broadcast())
        .stream;
  }

  VideoEvent _parseVideoEventFromMap(Map<String, dynamic> map) {
    switch (map['event']) {
      case 'initialized':
      case 'restored':
        final List<dynamic>? durationVal = map['duration'] as List<dynamic>?;
        VideoEventType videoEventType;
        if (map['event'] == 'initialized') {
          videoEventType = VideoEventType.initialized;
        } else {
          videoEventType = VideoEventType.restored;
        }
        return VideoEvent(
          eventType: videoEventType,
          duration: DurationRange(
            Duration(milliseconds: durationVal?[0] as int? ?? 0),
            Duration(milliseconds: durationVal?[1] as int? ?? 0),
          ),
          size: Size(
            (map['width'] as num?)?.toDouble() ?? 0.0,
            (map['height'] as num?)?.toDouble() ?? 0.0,
          ),
        );
      case 'completed':
        return VideoEvent(eventType: VideoEventType.completed);
      case 'bufferingUpdate':
        final int value = map['value']! as int;
        return VideoEvent(
          buffered: value,
          eventType: VideoEventType.bufferingUpdate,
        );
      case 'bufferingStart':
        return VideoEvent(eventType: VideoEventType.bufferingStart);
      case 'bufferingEnd':
        return VideoEvent(eventType: VideoEventType.bufferingEnd);
      case 'subtitleUpdate':
        return VideoEvent(
          eventType: VideoEventType.subtitleUpdate,
          text: map['text']! as String,
        );
      case 'isPlayingStateUpdate':
        return VideoEvent(
          eventType: VideoEventType.isPlayingStateUpdate,
          isPlaying: map['isPlaying']! as bool,
        );
      default:
        return VideoEvent(eventType: VideoEventType.unknown);
    }
  }

  @override
  Widget buildView(int playerId) {
    return Texture(textureId: playerId);
  }

  @override
  Future<void> setMixWithOthers(bool mixWithOthers) async {
    // Use FFI for setMixWithOthers (synchronous call)
    final int result = _ffiApi.setMixWithOthers(mixWithOthers);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_MIX_WITH_OTHERS_FAILED',
        message: 'FFI setMixWithOthers failed with code: $result',
      );
    }
  }

  @override
  Future<void> setDisplayGeometry(
    int playerId,
    int x,
    int y,
    int width,
    int height,
  ) async {
    // Use FFI for setDisplayGeometry (synchronous call)
    final int result = _ffiApi.setDisplayGeometry(
      playerId,
      x,
      y,
      width,
      height,
    );
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_DISPLAY_GEOMETRY_FAILED',
        message: 'FFI setDisplayGeometry failed with code: $result',
      );
    }
  }

  @override
  Future<void> suspend(int playerId) async {
    // Use FFI for suspend (synchronous call)
    final int result = _ffiApi.suspend(playerId);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SUSPEND_FAILED',
        message: 'FFI suspend failed with code: $result',
      );
    }
  }

  @override
  Future<void> restore(
    int playerId, {
    DataSource? dataSource,
    int resumeTime = -1,
  }) async {
    // Use FFI for restore (synchronous call)
    // Use CreateMessage class (JSON conversion handled internally)
    CreateMessage? message;
    if (dataSource != null) {
      message = CreateMessage();

      switch (dataSource.sourceType) {
        case DataSourceType.asset:
          message.asset = dataSource.asset;
          message.packageName = dataSource.package;
        case DataSourceType.network:
          message.uri = dataSource.uri;
          message.formatHint = _videoFormatStringMap[dataSource.formatHint];
          message.httpHeaders = dataSource.httpHeaders;
          message.drmConfigs = dataSource.drmConfigs?.toMap();
          message.playerOptions = dataSource.playerOptions;
        case DataSourceType.file:
        case DataSourceType.contentUri:
          message.uri = dataSource.uri;
      }
    }

    // P0-3 fix: restore returns void, player ID remains unchanged
    // FFI restore returns 0 on success, -1 on failure
    final int result = _ffiApi.restore(playerId, message, resumeTime);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_RESTORE_FAILED',
        message: 'FFI restore failed with code: $result',
      );
    }
    // Player ID remains unchanged, no StreamController update needed
  }

  @override
  Future<bool> setDisplayRotate(int playerId, DisplayRotation rotation) async {
    // Use FFI for setDisplayRotate (synchronous call)
    final int result = _ffiApi.setDisplayRotate(playerId, rotation.index);
    if (result != 0) {
      throw PlatformException(
        code: 'FFI_SET_DISPLAY_ROTATE_FAILED',
        message: 'FFI setDisplayRotate failed with code: $result',
      );
    }
    return true;
  }

  static const Map<VideoFormat, String> _videoFormatStringMap =
      <VideoFormat, String>{
        VideoFormat.ss: 'ss',
        VideoFormat.hls: 'hls',
        VideoFormat.dash: 'dash',
        VideoFormat.other: 'other',
      };
}
