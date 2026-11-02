// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(
  PigeonOptions(
    dartOut: 'lib/src/messages.g.dart',
    dartFfiOut: 'lib/src/messages.g.ffi.dart',
    configDirectory: 'tool/pigeon',
    dartOptions: DartOptions(
      ffiOptions: DartFfiOptions(
        bindingClassName: 'VideoPlayerTizenFfiBindings',
        nativeLibraryExpression: r'''
(() {
  final String libraryPath = File('/proc/self/maps')
      .readAsLinesSync()
      .map((String line) {
        final int index = line.indexOf('/');
        return index == -1 ? '' : line.substring(index);
      })
      .firstWhere(
        (String path) => path.endsWith('/libflutter_plugins.so'),
      );
  return ffi.DynamicLibrary.open(libraryPath);
})()''',
      ),
    ),
    cppOptions: CppOptions(namespace: 'video_player_tizen', useStdVisit: false),
    cppHeaderOut: 'tizen/src/messages.h',
    cppSourceOut: 'tizen/src/messages.cc',
    cppFfiHeaderOut: 'tizen/src/messages_ffi.h',
    cppFfiSourceOut: 'tizen/src/messages_ffi.cc',
  ),
)
class TextureMessage {
  TextureMessage(this.textureId);
  int textureId;
}

class LoopingMessage {
  LoopingMessage(this.textureId, this.isLooping);
  int textureId;
  bool isLooping;
}

class VolumeMessage {
  VolumeMessage(this.textureId, this.volume);
  int textureId;
  double volume;
}

class PlaybackSpeedMessage {
  PlaybackSpeedMessage(this.textureId, this.speed);
  int textureId;
  double speed;
}

class PositionMessage {
  PositionMessage(this.textureId, this.position);
  int textureId;
  int position;
}

class CreateMessage {
  CreateMessage({required this.httpHeaders});
  String? asset;
  String? uri;
  String? packageName;
  String? formatHint;
  Map<String?, String?> httpHeaders;
}

class MixWithOthersMessage {
  MixWithOthersMessage(this.mixWithOthers);
  bool mixWithOthers;
}

class AudioTrackMessage {
  AudioTrackMessage(this.trackIndex, this.isSelected);
  int trackIndex;
  String? language;
  bool isSelected;
}

class SelectAudioTrackMessage {
  SelectAudioTrackMessage(this.textureId, this.trackIndex);
  int textureId;
  int trackIndex;
}

class PreventsDisplaySleepMessage {
  PreventsDisplaySleepMessage(this.textureId, this.preventsDisplaySleep);
  int textureId;
  bool preventsDisplaySleep;
}

@HostApi()
abstract class TizenVideoPlayerApi {
  void initialize();
  TextureMessage create(CreateMessage msg);
  void dispose(TextureMessage msg);
  void setLooping(LoopingMessage msg);
  void setVolume(VolumeMessage msg);
  void setPlaybackSpeed(PlaybackSpeedMessage msg);
  void play(TextureMessage msg);
  PositionMessage position(TextureMessage msg);
  @async
  void seekTo(PositionMessage msg);
  void pause(TextureMessage msg);
  void setMixWithOthers(MixWithOthersMessage msg);
  void setPreventsDisplaySleep(PreventsDisplaySleepMessage msg);
  List<AudioTrackMessage> getAudioTracks(TextureMessage msg);
  void selectAudioTrack(SelectAudioTrackMessage msg);
}

@EventChannelApi()
abstract class VideoPlayerEvents {
  Map<Object?, Object?> videoEvents();
}
