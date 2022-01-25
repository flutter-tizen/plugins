// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:flutter/services.dart';

enum AudioVolumeType {
  system,
  notification,
  alarm,
  ringtone,
  media,
  call,
  voip,
  voice,
  none
}

AudioVolumeType _stringToAudioVolumeType(String s) {
  switch (s) {
    case 'system':
      return AudioVolumeType.system;
    case 'notification':
      return AudioVolumeType.notification;
    case 'alarm':
      return AudioVolumeType.alarm;
    case 'ringtone':
      return AudioVolumeType.ringtone;
    case 'media':
      return AudioVolumeType.media;
    case 'call':
      return AudioVolumeType.call;
    case 'voip':
      return AudioVolumeType.voip;
    case 'voice':
      return AudioVolumeType.voice;
    case 'none':
    default:
      return AudioVolumeType.none;
  }
}

class AudioManager {
  static const MethodChannel _channel = MethodChannel('audio_manager');

  static const EventChannel _eventChannel =
      EventChannel('audio_manager_events');

  static final volumeController = AudioVolume();
}

class AudioVolume {
  static final AudioVolume _audioVolume = AudioVolume._internal();

  factory AudioVolume() {
    return _audioVolume;
  }

  AudioVolume._internal();

  Future<AudioVolumeType> get currentPlaybackType async {
    try {
      final String? type = await AudioManager._channel
          .invokeMethod<String>('getCurrentPlaybackType');
      return _stringToAudioVolumeType(type!);
    } catch (err) {
      throw Exception('currentPlaybackType failed, $err');
    }
  }

  Future<int> getLevel(AudioVolumeType type) async {
    try {
      final int? level = await AudioManager._channel.invokeMethod<int>(
          'getLevel', {'type': type.toString().split('.').last});
      return level!;
    } catch (err) {
      throw Exception('getLevel failed, $err');
    }
  }

  void setLevel(AudioVolumeType type, int level) async {
    try {
      await AudioManager._channel.invokeMethod<void>('setLevel', {
        'type': type.toString().split('.').last,
        'volume': level.toString()
      });
    } catch (err) {
      throw Exception('setLevel failed, $err');
    }
  }

  Future<int> getMaxLevel(AudioVolumeType type) async {
    try {
      final int? level = await AudioManager._channel.invokeMethod<int>(
          'getMaxLevel', {'type': type.toString().split('.').last});
      return level!;
    } catch (err) {
      throw Exception('getMaxLevel failed, $err');
    }
  }

  Stream<VolumeChangedEvent> onChanged = AudioManager._eventChannel
      .receiveBroadcastStream()
      .map((dynamic msg) =>
          VolumeChangedEvent.fromMap(msg as Map<String, String>));
}

class VolumeChangedEvent {
  VolumeChangedEvent({required this.level, required this.type});

  final int level;

  final AudioVolumeType type;

  static VolumeChangedEvent fromMap(Map<String, String> map) =>
      VolumeChangedEvent(
          level: int.parse(map['level'] ?? '0'),
          type: _stringToAudioVolumeType(map['type'] ?? 'none'));
}
