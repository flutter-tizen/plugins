// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';

import '../video_player_platform_interface.dart';
import 'messages.g.dart';

/// An implementation of [VideoPlayerPlatform] that uses the
/// Pigeon-generated [TizenVideoPlayerApi].
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
        break;
      case DataSourceType.network:
        message.uri = dataSource.uri;
        message.formatHint = _videoFormatStringMap[dataSource.formatHint];
        message.httpHeaders = dataSource.httpHeaders;
        message.drmConfigs = dataSource.drmConfigs?.toMap();
        break;
      case DataSourceType.file:
        message.uri = dataSource.uri;
        break;
      case DataSourceType.contentUri:
        message.uri = dataSource.uri;
        break;
    }

    final PlayerMessage response = await _api.create(message);
    return response.playerId;
  }

  @override
  Future<void> setLooping(int playerId, bool looping) {
    return _api
        .setLooping(LoopingMessage(playerId: playerId, isLooping: looping));
  }

  @override
  Future<void> play(int playerId) {
    return _api.play(PlayerMessage(playerId: playerId));
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
        PlaybackSpeedMessage(playerId: playerId, speed: speed));
  }

  @override
  Future<void> seekTo(int playerId, Duration position) {
    return _api.seekTo(
        PositionMessage(playerId: playerId, position: position.inMilliseconds));
  }

  @override
  Future<Duration> getPosition(int playerId) async {
    final PositionMessage response =
        await _api.position(PlayerMessage(playerId: playerId));
    return Duration(milliseconds: response.position);
  }

  @override
  Stream<VideoEvent> videoEventsFor(int playerId) {
    return _eventChannelFor(playerId)
        .receiveBroadcastStream()
        .map((dynamic event) {
      final Map<dynamic, dynamic> map = event as Map<dynamic, dynamic>;
      switch (map['event']) {
        case 'initialized':
          return VideoEvent(
            eventType: VideoEventType.initialized,
            duration: Duration(milliseconds: map['duration']! as int),
            size: Size((map['width'] as num?)?.toDouble() ?? 0.0,
                (map['height'] as num?)?.toDouble() ?? 0.0),
          );
        case 'completed':
          return VideoEvent(
            eventType: VideoEventType.completed,
          );
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
    return _api
        .setMixWithOthers(MixWithOthersMessage(mixWithOthers: mixWithOthers));
  }

  @override
  Future<void> setDisplayGeometry(
    int playerId,
    int x,
    int y,
    int width,
    int height,
  ) {
    return _api.setDisplayGeometry(GeometryMessage(
      playerId: playerId,
      x: x,
      y: y,
      width: width,
      height: height,
    ));
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
