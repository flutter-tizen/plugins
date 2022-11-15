// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(PigeonOptions(
    dartOut: 'lib/src/messages.dart',
    dartTestOut: 'test/test_api.dart',
    //cppHeaderOut: 'tizen/src/messages.h',
    //cppSourceOut: 'tizen/src/messages.cc',
    copyrightHeader: 'pigeons/copyright.txt'))
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
  GeometryMessage(this.playerId, this.x, this.y, this.w, this.h);
  int playerId;
  int x;
  int y;
  int w;
  int h;
}

@HostApi(dartHostTestHandler: 'TestHostVideoPlayerApi')
abstract class VideoPlayerApi {
  void initialize();
  PlayerMessage create(CreateMessage msg);
  void dispose(PlayerMessage msg);
  void setLooping(LoopingMessage msg);
  void setVolume(VolumeMessage msg);
  void setPlaybackSpeed(PlaybackSpeedMessage msg);
  void play(PlayerMessage msg);
  PositionMessage position(PlayerMessage msg);
  void seekTo(PositionMessage msg);
  void pause(PlayerMessage msg);
  void setMixWithOthers(MixWithOthersMessage msg);
  void setDisplayRoi(GeometryMessage arg);
}
