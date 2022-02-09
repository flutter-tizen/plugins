// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

/// Enumeration for the audio volume types.
enum AudioVolumeType {
  /// System.
  system,

  /// Notification.
  notification,

  /// Alarm.
  alarm,

  /// Ringtone.
  ringtone,

  /// Media.
  media,

  /// Call.
  call,

  /// VoIP.
  voip,

  /// Voice.
  voice,

  /// No volume.
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

/// Provides the ability to control volume levels.
class AudioManager {
  AudioManager._();

  static const MethodChannel _channel = MethodChannel('tizen/audio_manager');

  static const EventChannel _eventChannel =
      EventChannel('tizen/audio_manager_events');

  /// Gets the volume controller.
  static final AudioVolume volumeController = AudioVolume();
}

/// Provides the ability to control and monitor the volume levels.
class AudioVolume {
  /// Provides the ability to control and monitor the volume levels.
  factory AudioVolume() {
    return _audioVolume;
  }

  AudioVolume._internal();

  static final AudioVolume _audioVolume = AudioVolume._internal();

  /// Gets the volume type of the sound being currently played.
  Future<AudioVolumeType> get currentPlaybackType async {
    final String? type = await AudioManager._channel
        .invokeMethod<String>('getCurrentPlaybackType');
    return _stringToAudioVolumeType(type!);
  }

  /// Gets the current volume level for [type].
  Future<int> getLevel(AudioVolumeType type) async {
    final int? level = await AudioManager._channel.invokeMethod<int>(
      'getLevel',
      <String, String>{'type': type.name},
    );
    return level!;
  }

  /// Sets the volume level for [type].
  Future<void> setLevel(AudioVolumeType type, int level) async {
    await AudioManager._channel.invokeMethod<void>(
      'setLevel',
      <String, dynamic>{'type': type.name, 'volume': level},
    );
  }

  /// Gets the maximum volume level for [type].
  Future<int> getMaxLevel(AudioVolumeType type) async {
    final int? level = await AudioManager._channel.invokeMethod<int>(
      'getMaxLevel',
      <String, String>{'type': type.name},
    );
    return level!;
  }

  /// A stream of events occurring when the volume level is changed.
  Stream<VolumeChangedEvent> onChanged = AudioManager._eventChannel
      .receiveBroadcastStream()
      .map((dynamic msg) => VolumeChangedEvent.fromMap(
          (msg as Map<Object?, Object?>).cast<String, dynamic>()));
}

/// Represents an event emitted on volume change.
class VolumeChangedEvent {
  /// Represents an event emitted on volume change.
  VolumeChangedEvent({required this.level, required this.type});

  /// New volume level.
  final int level;

  /// Specifies an audio type for which the volume has changed.
  final AudioVolumeType type;

  /// Creates an event from a map.
  static VolumeChangedEvent fromMap(Map<String, dynamic> map) =>
      VolumeChangedEvent(
          level: map['level'] as int? ?? 0,
          type: _stringToAudioVolumeType(map['type'] as String? ?? 'none'));
}
