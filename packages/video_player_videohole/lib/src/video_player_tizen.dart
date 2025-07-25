// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import '../video_player_platform_interface.dart';
import 'messages.g.dart';
import 'tracks.dart';

/// An implementation of [VideoPlayerPlatform] that uses the
/// Pigeon-generated [VideoPlayerVideoholeApi].
class VideoPlayerTizen extends VideoPlayerPlatform {
  final VideoPlayerVideoholeApi _api = VideoPlayerVideoholeApi();

  @override
  Future<void> init() {
    return _api.initialize();
  }

  @override
  Future<void> dispose(int playerId) {
    return _api.dispose(PlayerMessage(playerId: playerId));
  }

  @override
  Future<int?> create(DataSource dataSource) async {
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
        message.uri = dataSource.uri;
      case DataSourceType.contentUri:
        message.uri = dataSource.uri;
    }

    final PlayerMessage response = await _api.create(message);
    return response.playerId;
  }

  @override
  Future<void> setLooping(int playerId, bool looping) {
    return _api.setLooping(
      LoopingMessage(playerId: playerId, isLooping: looping),
    );
  }

  @override
  Future<void> play(int playerId) {
    return _api.play(PlayerMessage(playerId: playerId));
  }

  @override
  Future<bool> setActivate(int playerId) {
    return _api.setActivate(PlayerMessage(playerId: playerId));
  }

  @override
  Future<bool> setDeactivate(int playerId) {
    return _api.setDeactivate(PlayerMessage(playerId: playerId));
  }

  @override
  Future<void> pause(int playerId) {
    return _api.pause(PlayerMessage(playerId: playerId));
  }

  @override
  Future<void> setVolume(int playerId, double volume) {
    return _api.setVolume(VolumeMessage(playerId: playerId, volume: volume));
  }

  @override
  Future<void> setPlaybackSpeed(int playerId, double speed) {
    assert(speed > 0);

    return _api.setPlaybackSpeed(
      PlaybackSpeedMessage(playerId: playerId, speed: speed),
    );
  }

  @override
  Future<void> seekTo(int playerId, Duration position) {
    return _api.seekTo(
      PositionMessage(playerId: playerId, position: position.inMilliseconds),
    );
  }

  @override
  Future<List<VideoTrack>> getVideoTracks(int playerId) async {
    final TrackMessage response = await _api.track(
      TrackTypeMessage(playerId: playerId, trackType: TrackType.video.name),
    );

    final List<VideoTrack> videoTracks = <VideoTrack>[];
    for (final Map<Object?, Object?>? trackMap in response.tracks) {
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
    final TrackMessage response = await _api.track(
      TrackTypeMessage(playerId: playerId, trackType: TrackType.audio.name),
    );

    final List<AudioTrack> audioTracks = <AudioTrack>[];
    for (final Map<Object?, Object?>? trackMap in response.tracks) {
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
    final TrackMessage response = await _api.track(
      TrackTypeMessage(playerId: playerId, trackType: TrackType.text.name),
    );

    final List<TextTrack> textTracks = <TextTrack>[];
    for (final Map<Object?, Object?>? trackMap in response.tracks) {
      final int trackId = trackMap!['trackId']! as int;
      final String language = trackMap['language']! as String;

      textTracks.add(TextTrack(trackId: trackId, language: language));
    }

    return textTracks;
  }

  @override
  Future<bool> setTrackSelection(int playerId, Track track) {
    return _api.setTrackSelection(
      SelectedTracksMessage(
        playerId: playerId,
        trackId: track.trackId,
        trackType: track.trackType.name,
      ),
    );
  }

  @override
  Future<DurationRange> getDuration(int playerId) async {
    final DurationMessage message = await _api.duration(
      PlayerMessage(playerId: playerId),
    );
    return DurationRange(
      Duration(milliseconds: message.durationRange?[0] ?? 0),
      Duration(milliseconds: message.durationRange?[1] ?? 0),
    );
  }

  @override
  Future<Duration> getPosition(int playerId) async {
    final PositionMessage response = await _api.position(
      PlayerMessage(playerId: playerId),
    );
    return Duration(milliseconds: response.position);
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
  Future<void> setMixWithOthers(bool mixWithOthers) {
    return _api.setMixWithOthers(
      MixWithOthersMessage(mixWithOthers: mixWithOthers),
    );
  }

  @override
  Future<void> setDisplayGeometry(
    int playerId,
    int x,
    int y,
    int width,
    int height,
  ) {
    return _api.setDisplayGeometry(
      GeometryMessage(
        playerId: playerId,
        x: x,
        y: y,
        width: width,
        height: height,
      ),
    );
  }

  @override
  Future<void> suspend(int playerId) {
    return _api.suspend(playerId);
  }

  @override
  Future<void> restore(
    int playerId, {
    DataSource? dataSource,
    int resumeTime = -1,
  }) {
    final CreateMessage message = CreateMessage();

    if (dataSource != null) {
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
          message.uri = dataSource.uri;
        case DataSourceType.contentUri:
          message.uri = dataSource.uri;
      }
    }

    return _api.restore(playerId, message, resumeTime);
  }

  @override
  Future<bool> setDisplayRotate(int playerId, DisplayRotation rotation) {
    return _api.setDisplayRotate(
      RotationMessage(playerId: playerId, rotation: rotation.index),
    );
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
