// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(PigeonOptions(
  dartOut: 'lib/src/messages.g.dart',
  cppHeaderOut: 'tizen/src/messages.h',
  cppSourceOut: 'tizen/src/messages.cc',
))
class PlayerMessage {
  PlayerMessage(this.playerId);
  int playerId;
}

class LoopingMessage {
  LoopingMessage(this.playerId, this.isLooping);
  int playerId;
  bool isLooping;
}

class VolumeMessage {
  VolumeMessage(this.playerId, this.volume);
  int playerId;
  double volume;
}

class PlaybackSpeedMessage {
  PlaybackSpeedMessage(this.playerId, this.speed);
  int playerId;
  double speed;
}

class VideoTrackMessage {
  VideoTrackMessage(this.playerId, this.videoTracks);
  int playerId;
  List<Map<Object?, Object?>?> videoTracks;
}

class AudioTrackMessage {
  AudioTrackMessage(this.playerId, this.audioTracks);
  int playerId;
  List<Map<Object?, Object?>?> audioTracks;
}

class TextTrackMessage {
  TextTrackMessage(this.playerId, this.textTracks);
  int playerId;
  List<Map<Object?, Object?>?> textTracks;
}

class SelectedTracksMessage {
  SelectedTracksMessage(this.playerId, this.trackId, this.trackType);
  int playerId;
  int trackId;
  int trackType;
}

class PositionMessage {
  PositionMessage(this.playerId, this.position);
  int playerId;
  int position;
}

class CreateMessage {
  CreateMessage();
  String? asset;
  String? uri;
  String? packageName;
  String? formatHint;
  Map<Object?, Object?>? httpHeaders;
  Map<Object?, Object?>? drmConfigs;
}

class MixWithOthersMessage {
  MixWithOthersMessage(this.mixWithOthers);
  bool mixWithOthers;
}

class GeometryMessage {
  GeometryMessage(this.playerId, this.x, this.y, this.width, this.height);
  int playerId;
  int x;
  int y;
  int width;
  int height;
}

@HostApi()
abstract class VideoPlayerVideoholeApi {
  void initialize();
  PlayerMessage create(CreateMessage msg);
  void dispose(PlayerMessage msg);
  void setLooping(LoopingMessage msg);
  void setVolume(VolumeMessage msg);
  void setPlaybackSpeed(PlaybackSpeedMessage msg);
  void play(PlayerMessage msg);
  PositionMessage position(PlayerMessage msg);
  @async
  void seekTo(PositionMessage msg);
  VideoTrackMessage videoTrack(PlayerMessage msg);
  AudioTrackMessage audioTrack(PlayerMessage msg);
  TextTrackMessage textTrack(PlayerMessage msg);
  void setTrackSelection(SelectedTracksMessage msg);
  void pause(PlayerMessage msg);
  void setMixWithOthers(MixWithOthersMessage msg);
  void setDisplayGeometry(GeometryMessage msg);
}
