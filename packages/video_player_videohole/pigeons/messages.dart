// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(
  PigeonOptions(
    dartOut: 'lib/src/messages.g.dart',
    cppHeaderOut: 'tizen/src/messages.h',
    cppSourceOut: 'tizen/src/messages.cc',
  ),
)
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

class TrackMessage {
  TrackMessage(this.playerId, this.tracks);
  int playerId;
  List<Map<Object?, Object?>?> tracks;
}

class TrackTypeMessage {
  TrackTypeMessage(this.playerId, this.trackType);
  int playerId;
  String trackType;
}

class SelectedTracksMessage {
  SelectedTracksMessage(this.playerId, this.trackId, this.trackType);
  int playerId;
  int trackId;
  String trackType;
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
  Map<Object?, Object?>? playerOptions;
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

class DurationMessage {
  DurationMessage(this.playerId);
  int playerId;
  List<int?>? durationRange;
}

class RotationMessage {
  RotationMessage(this.playerId, this.rotation);
  int playerId;
  int rotation;
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
  bool setDeactivate(PlayerMessage msg);
  bool setActivate(PlayerMessage msg);
  TrackMessage track(TrackTypeMessage msg);
  bool setTrackSelection(SelectedTracksMessage msg);
  PositionMessage position(PlayerMessage msg);
  @async
  void seekTo(PositionMessage msg);
  void pause(PlayerMessage msg);
  void setMixWithOthers(MixWithOthersMessage msg);
  void setDisplayGeometry(GeometryMessage msg);
  DurationMessage duration(PlayerMessage msg);
  void suspend(int playerId);
  void restore(int playerId, CreateMessage? msg, int resumeTime);
  bool setDisplayRotate(RotationMessage msg);
}
