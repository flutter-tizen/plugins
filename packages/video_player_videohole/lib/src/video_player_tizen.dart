// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import '../video_player_platform_interface.dart';
import 'messages.g.dart';
import 'tracks.dart';

/// An implementation of [VideoPlayerPlatform] that uses FFI for all methods.
class VideoPlayerTizen extends VideoPlayerPlatform {
  final VideoPlayerFFIApi _ffiApi = VideoPlayerFFIApi();

  @override
  Future<void> init() async {
    // Use FFI for initialization (synchronous call)
    final int result = _ffiApi.initialize();
    if (result != 0) {
      throw Exception('FFI initialize failed with code: $result');
    }
  }

  @override
  Future<void> dispose(int playerId) async {
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
    // Use FFI for create (synchronous call)
    int playerId;

    switch (dataSource.sourceType) {
      case DataSourceType.asset:
        playerId = _ffiApi.create(
          asset: dataSource.asset,
          packageName: dataSource.package,
        );
      case DataSourceType.network:
        playerId = _ffiApi.create(
          uri: dataSource.uri,
          formatHint: _videoFormatStringMap[dataSource.formatHint],
          httpHeaders: dataSource.httpHeaders,
          drmConfigs: dataSource.drmConfigs?.toMap(),
          playerOptions: dataSource.playerOptions,
        );
      case DataSourceType.file:
      case DataSourceType.contentUri:
        playerId = _ffiApi.create(uri: dataSource.uri);
    }

    if (playerId < 0) {
      throw Exception('FFI create failed with code: $playerId');
    }
    return playerId;
  }

  @override
  Future<void> setLooping(int playerId, bool looping) async {
    // Use FFI for setLooping (synchronous call)
    final int result = _ffiApi.setLooping(playerId, looping);
    if (result != 0) {
      throw Exception('FFI setLooping failed with code: $result');
    }
  }

  @override
  Future<void> play(int playerId) async {
    // Use FFI for play (synchronous call)
    final int result = _ffiApi.play(playerId);
    if (result != 0) {
      throw Exception('FFI play failed with code: $result');
    }
  }

  @override
  Future<bool> setActivate(int playerId) async {
    // Use FFI for setActivate (synchronous call)
    final int result = _ffiApi.setActivate(playerId);
    if (result != 0) {
      throw Exception('FFI setActivate failed with code: $result');
    }
    return true;
  }

  @override
  Future<bool> setDeactivate(int playerId) async {
    // Use FFI for setDeactivate (synchronous call)
    final int result = _ffiApi.setDeactivate(playerId);
    if (result != 0) {
      throw Exception('FFI setDeactivate failed with code: $result');
    }
    return true;
  }

  @override
  Future<void> pause(int playerId) async {
    // Use FFI for pause (synchronous call)
    final int result = _ffiApi.pause(playerId);
    if (result != 0) {
      throw Exception('FFI pause failed with code: $result');
    }
  }

  @override
  Future<void> setVolume(int playerId, double volume) async {
    // Use FFI for setVolume (synchronous call)
    final int result = _ffiApi.setVolume(playerId, volume);
    if (result != 0) {
      throw Exception('FFI setVolume failed with code: $result');
    }
  }

  @override
  Future<void> setPlaybackSpeed(int playerId, double speed) async {
    // Use FFI for setPlaybackSpeed (synchronous call)
    assert(speed > 0);
    final int result = _ffiApi.setPlaybackSpeed(playerId, speed);
    if (result != 0) {
      throw Exception('FFI setPlaybackSpeed failed with code: $result');
    }
  }

  @override
  Future<void> seekTo(int playerId, Duration position) async {
    // Use FFI for seekTo (synchronous call)
    final int result = _ffiApi.seekTo(playerId, position.inMilliseconds);
    if (result != 0) {
      throw Exception('FFI seekTo failed with code: $result');
    }
  }

  @override
  Future<List<VideoTrack>> getVideoTracks(int playerId) async {
    // Use FFI for getTrackInfo (synchronous call)
    // C++ returns: {"playerId": <id>, "tracks": [{...}, ...]}
    final String jsonResponse = _ffiApi.getTrackInfo(playerId, 'video');
    final Map<String, dynamic> responseJson =
        jsonDecode(jsonResponse) as Map<String, dynamic>;
    final List<dynamic> tracksJson = responseJson['tracks'] as List<dynamic>;

    final List<VideoTrack> videoTracks = <VideoTrack>[];
    for (final dynamic trackMap in tracksJson) {
      final int trackId = trackMap['trackId'] as int;
      final int bitrate = trackMap['bitrate'] as int;
      final int width = trackMap['width'] as int;
      final int height = trackMap['height'] as int;

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
    // Use FFI for getTrackInfo (synchronous call)
    // C++ returns: {"playerId": <id>, "tracks": [{...}, ...]}
    final String jsonResponse = _ffiApi.getTrackInfo(playerId, 'audio');
    final Map<String, dynamic> responseJson =
        jsonDecode(jsonResponse) as Map<String, dynamic>;
    final List<dynamic> tracksJson = responseJson['tracks'] as List<dynamic>;

    final List<AudioTrack> audioTracks = <AudioTrack>[];
    for (final dynamic trackMap in tracksJson) {
      final int trackId = trackMap['trackId'] as int;
      final String language = trackMap['language'] as String;
      final int channel = trackMap['channel'] as int;
      final int bitrate = trackMap['bitrate'] as int;

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
    // Use FFI for getTrackInfo (synchronous call)
    // C++ returns: {"playerId": <id>, "tracks": [{...}, ...]}
    final String jsonResponse = _ffiApi.getTrackInfo(playerId, 'text');
    final Map<String, dynamic> responseJson =
        jsonDecode(jsonResponse) as Map<String, dynamic>;
    final List<dynamic> tracksJson = responseJson['tracks'] as List<dynamic>;

    final List<TextTrack> textTracks = <TextTrack>[];
    for (final dynamic trackMap in tracksJson) {
      final int trackId = trackMap['trackId'] as int;
      final String language = trackMap['language'] as String;

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
      throw Exception('FFI setTrackSelection failed with code: $result');
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
      throw Exception('FFI getPosition failed with code: $positionMs');
    }
    return Duration(milliseconds: positionMs);
  }

  @override
  Stream<VideoEvent> videoEventsFor(int playerId) {
    return _eventChannelFor(playerId).receiveBroadcastStream().map((
      dynamic event,
    ) {
      final Map<dynamic, dynamic> map = event as Map<dynamic, dynamic>;
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
              Duration(milliseconds: durationVal?[0] as int),
              Duration(milliseconds: durationVal?[1] as int),
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
    });
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
      throw Exception('FFI setMixWithOthers failed with code: $result');
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
    final int result =
        _ffiApi.setDisplayGeometry(playerId, x, y, width, height);
    if (result != 0) {
      throw Exception('FFI setDisplayGeometry failed with code: $result');
    }
  }

  @override
  Future<void> suspend(int playerId) async {
    // Use FFI for suspend (synchronous call)
    final int result = _ffiApi.suspend(playerId);
    if (result != 0) {
      throw Exception('FFI suspend failed with code: $result');
    }
  }

  @override
  Future<void> restore(
    int playerId, {
    DataSource? dataSource,
    int resumeTime = -1,
  }) async {
    // Use FFI for restore (synchronous call)
    // Build JSON string from dataSource (same pattern as create)
    String? createMessageJson;
    if (dataSource != null) {
      final Map<String, dynamic> jsonMap = <String, dynamic>{};

      switch (dataSource.sourceType) {
        case DataSourceType.asset:
          if (dataSource.asset != null && dataSource.asset!.isNotEmpty) {
            jsonMap['asset'] = dataSource.asset;
          }
          if (dataSource.package != null && dataSource.package!.isNotEmpty) {
            jsonMap['packageName'] = dataSource.package;
          }
        case DataSourceType.network:
          if (dataSource.uri != null && dataSource.uri!.isNotEmpty) {
            jsonMap['uri'] = dataSource.uri;
          }
          if (dataSource.formatHint != null) {
            jsonMap['formatHint'] =
                _videoFormatStringMap[dataSource.formatHint];
          }
          if (dataSource.httpHeaders.isNotEmpty) {
            jsonMap['httpHeaders'] = dataSource.httpHeaders;
          }
          if (dataSource.drmConfigs != null) {
            jsonMap['drmConfigs'] = dataSource.drmConfigs!.toMap();
          }
          if (dataSource.playerOptions != null &&
              dataSource.playerOptions!.isNotEmpty) {
            jsonMap['playerOptions'] = dataSource.playerOptions;
          }
        case DataSourceType.file:
        case DataSourceType.contentUri:
          if (dataSource.uri != null && dataSource.uri!.isNotEmpty) {
            jsonMap['uri'] = dataSource.uri;
          }
      }

      createMessageJson = jsonEncode(jsonMap);
    }

    final int result = _ffiApi.restore(playerId, createMessageJson, resumeTime);
    if (result != 0) {
      // FFI restore failed, but don't throw - just return
      // The player may still be in a valid state
      return;
    }
  }

  @override
  Future<bool> setDisplayRotate(int playerId, DisplayRotation rotation) async {
    // Use FFI for setDisplayRotate (synchronous call)
    final int result = _ffiApi.setDisplayRotate(playerId, rotation.index);
    if (result != 0) {
      throw Exception('FFI setDisplayRotate failed with code: $result');
    }
    return true;
  }

  EventChannel _eventChannelFor(int playerId) {
    return EventChannel('tizen/video_player/video_events_$playerId');
  }

  static const Map<VideoFormat, String> _videoFormatStringMap =
      <VideoFormat, String>{
    VideoFormat.ss: 'ss',
    VideoFormat.hls: 'hls',
    VideoFormat.dash: 'dash',
    VideoFormat.other: 'other',
  };
}
