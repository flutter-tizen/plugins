// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:video_player_platform_interface/video_player_platform_interface.dart'
    as platform_interface;

import 'src/messages.g.dart';

// TODO(JSUYA): Remove the ignore and rename parameters when adding support for platform views.
// ignore_for_file: avoid_renaming_method_parameters

/// A Tizen implementation of [VideoPlayerPlatform] that uses the
/// Pigeon-generated [TizenVideoPlayerApi].
class VideoPlayerTizen extends platform_interface.VideoPlayerPlatform {
  final TizenVideoPlayerApi _api = TizenVideoPlayerApi();

  /// Registers this class as the default platform instance.
  static void register() {
    platform_interface.VideoPlayerPlatform.instance = VideoPlayerTizen();
  }

  @override
  Future<void> init() {
    return _api.initialize();
  }

  @override
  Future<void> dispose(int textureId) {
    return _api.dispose(TextureMessage(textureId: textureId));
  }

  @override
  Future<int?> create(platform_interface.DataSource dataSource) async {
    String? asset;
    String? packageName;
    String? uri;
    String? formatHint;
    var httpHeaders = <String, String>{};
    switch (dataSource.sourceType) {
      case platform_interface.DataSourceType.asset:
        asset = dataSource.asset;
        packageName = dataSource.package;
      case platform_interface.DataSourceType.network:
        uri = dataSource.uri;
        formatHint = _videoFormatStringMap[dataSource.formatHint];
        httpHeaders = dataSource.httpHeaders;
      case platform_interface.DataSourceType.file:
        uri = dataSource.uri;
      case platform_interface.DataSourceType.contentUri:
        uri = dataSource.uri;
    }
    final message = CreateMessage(
      asset: asset,
      packageName: packageName,
      uri: uri,
      httpHeaders: httpHeaders,
      formatHint: formatHint,
    );

    final TextureMessage response = await _api.create(message);
    return response.textureId;
  }

  @override
  Future<void> setLooping(int textureId, bool looping) {
    return _api.setLooping(LoopingMessage(textureId: textureId, isLooping: looping));
  }

  @override
  Future<void> play(int textureId) {
    return _api.play(TextureMessage(textureId: textureId));
  }

  @override
  Future<void> pause(int textureId) {
    return _api.pause(TextureMessage(textureId: textureId));
  }

  @override
  Future<void> setVolume(int textureId, double volume) {
    return _api.setVolume(VolumeMessage(textureId: textureId, volume: volume));
  }

  @override
  Future<void> setPlaybackSpeed(int textureId, double speed) {
    assert(speed > 0);

    return _api.setPlaybackSpeed(PlaybackSpeedMessage(textureId: textureId, speed: speed));
  }

  @override
  Future<void> seekTo(int textureId, Duration position) {
    return _api.seekTo(PositionMessage(textureId: textureId, position: position.inMilliseconds));
  }

  @override
  Future<Duration> getPosition(int textureId) async {
    final PositionMessage response = await _api.position(TextureMessage(textureId: textureId));
    return Duration(milliseconds: response.position);
  }

  @override
  Stream<platform_interface.VideoEvent> videoEventsFor(int textureId) {
    return _eventChannelFor(textureId).receiveBroadcastStream().map((dynamic event) {
      final map = event as Map<dynamic, dynamic>;
      switch (map['event']) {
        case 'initialized':
          return platform_interface.VideoEvent(
            eventType: platform_interface.VideoEventType.initialized,
            duration: Duration(milliseconds: map['duration'] as int),
            size: Size(
              (map['width'] as num?)?.toDouble() ?? 0.0,
              (map['height'] as num?)?.toDouble() ?? 0.0,
            ),
            rotationCorrection: map['rotationCorrection'] as int? ?? 0,
          );
        case 'completed':
          return platform_interface.VideoEvent(
            eventType: platform_interface.VideoEventType.completed,
          );
        case 'bufferingUpdate':
          final values = map['values'] as List<dynamic>;

          return platform_interface.VideoEvent(
            buffered: values.map<platform_interface.DurationRange>(_toDurationRange).toList(),
            eventType: platform_interface.VideoEventType.bufferingUpdate,
          );
        case 'bufferingStart':
          return platform_interface.VideoEvent(
            eventType: platform_interface.VideoEventType.bufferingStart,
          );
        case 'bufferingEnd':
          return platform_interface.VideoEvent(
            eventType: platform_interface.VideoEventType.bufferingEnd,
          );
        case 'isPlayingStateUpdate':
          return platform_interface.VideoEvent(
            eventType: platform_interface.VideoEventType.isPlayingStateUpdate,
            isPlaying: map['isPlaying'] as bool,
          );
        default:
          return platform_interface.VideoEvent(
            eventType: platform_interface.VideoEventType.unknown,
          );
      }
    });
  }

  @override
  Widget buildView(int textureId) {
    return Texture(textureId: textureId);
  }

  @override
  Future<void> setMixWithOthers(bool mixWithOthers) {
    return _api.setMixWithOthers(MixWithOthersMessage(mixWithOthers: mixWithOthers));
  }

  @override
  Future<void> setPreventsDisplaySleepDuringVideoPlayback(
    int textureId,
    bool preventsDisplaySleepDuringVideoPlayback,
  ) {
    return _api.setPreventsDisplaySleep(
      PreventsDisplaySleepMessage(
        textureId: textureId,
        preventsDisplaySleep: preventsDisplaySleepDuringVideoPlayback,
      ),
    );
  }

  @override
  Future<List<platform_interface.VideoAudioTrack>> getAudioTracks(int textureId) async {
    final List<AudioTrackMessage> tracks = await _api.getAudioTracks(
      TextureMessage(textureId: textureId),
    );
    return tracks
        .map(
          (AudioTrackMessage track) => platform_interface.VideoAudioTrack(
            id: track.trackIndex.toString(),
            label: null,
            language: track.language,
            isSelected: track.isSelected,
          ),
        )
        .toList();
  }

  @override
  Future<void> selectAudioTrack(int textureId, String trackId) {
    return _api.selectAudioTrack(
      SelectAudioTrackMessage(textureId: textureId, trackIndex: int.parse(trackId)),
    );
  }

  @override
  bool isAudioTrackSupportAvailable() {
    return true;
  }

  @override
  Future<List<platform_interface.VideoTrack>> getVideoTracks(int textureId) async {
    final List<VideoTrackMessage> tracks = await _api.getVideoTracks(
      TextureMessage(textureId: textureId),
    );
    return tracks
        .map(
          (VideoTrackMessage track) => platform_interface.VideoTrack(
            id: '${track.bandwidth}_${track.width}x${track.height}',
            isSelected: track.isSelected,
            bitrate: track.bandwidth > 0 ? track.bandwidth : null,
            width: track.width > 0 ? track.width : null,
            height: track.height > 0 ? track.height : null,
          ),
        )
        .toList();
  }

  @override
  Future<void> selectVideoTrack(int textureId, platform_interface.VideoTrack? track) async {
    if (track != null && track.bitrate == null && track.width == null && track.height == null) {
      throw ArgumentError.value(track, 'track', 'must have a bitrate, width or height');
    }
    return _api.selectVideoTrack(
      SelectVideoTrackMessage(
        textureId: textureId,
        bandwidth: track?.bitrate ?? -1,
        width: track?.width ?? -1,
        height: track?.height ?? -1,
      ),
    );
  }

  @override
  bool isVideoTrackSupportAvailable() {
    return true;
  }

  EventChannel _eventChannelFor(int textureId) {
    return EventChannel('flutter.io/videoPlayer/videoEvents$textureId');
  }

  static const Map<platform_interface.VideoFormat, String> _videoFormatStringMap =
      <platform_interface.VideoFormat, String>{
        platform_interface.VideoFormat.ss: 'ss',
        platform_interface.VideoFormat.hls: 'hls',
        platform_interface.VideoFormat.dash: 'dash',
        platform_interface.VideoFormat.other: 'other',
      };

  platform_interface.DurationRange _toDurationRange(dynamic value) {
    final pair = value as List<dynamic>;
    return platform_interface.DurationRange(
      Duration(milliseconds: pair[0] as int),
      Duration(milliseconds: pair[1] as int),
    );
  }
}
