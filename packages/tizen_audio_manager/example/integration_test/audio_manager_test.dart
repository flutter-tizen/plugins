// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_audio_manager/tizen_audio_manager.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  test('test alarm max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.alarm,
    );
    expect(result, isNonNegative);
  });

  test('test call max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.call,
    );
    expect(result, isNonNegative);
  });

  test('test media max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.media,
    );
    expect(result, isNonNegative);
  });

  test('test notification max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.notification,
    );
    expect(result, isNonNegative);
  });

  test('test ringtone max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.ringtone,
    );
    expect(result, isNonNegative);
  });

  test('test system max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.system,
    );
    expect(result, isNonNegative);
  });

  test('test voice max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.voice,
    );
    expect(result, isNonNegative);
  });

  test('test voip max level', () async {
    final int result = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.voip,
    );
    expect(result, isNonNegative);
  });

  test('test alarm set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.alarm,
    );
    await AudioManager.volumeController.setLevel(AudioVolumeType.alarm, max);
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.alarm,
    );
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.alarm, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.alarm);
    expect(level, equals(0));
  });

  test('test call set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.call,
    );
    await AudioManager.volumeController.setLevel(AudioVolumeType.call, max);
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.call,
    );
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.call, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.call);
    expect(level, equals(0));
  });

  test('test media set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.media,
    );
    await AudioManager.volumeController.setLevel(AudioVolumeType.media, max);
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.media,
    );
    expect(level, equals(max));

    // TODO(seungsoo47): When setting the maximum volume (Volume: 15) for a
    // media type in AudioFW, it should be changed to 15. However, the Volume
    // app on the Tizen Platform resets the volume to 13 for hearing protection.
    // Therefore, before setting it to 0, we will wait about 3 seconds for the
    // Volume app to reset the volume to (13) before proceeding to the next
    // step.
    await Future<void>.delayed(const Duration(seconds: 3));

    await AudioManager.volumeController.setLevel(AudioVolumeType.media, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.media);
    expect(level, equals(0));
  });

  test('test notification set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.notification,
    );
    await AudioManager.volumeController.setLevel(
      AudioVolumeType.notification,
      max,
    );
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.notification,
    );
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(
      AudioVolumeType.notification,
      0,
    );
    level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.notification,
    );
    expect(level, equals(0));
  });

  test('test ringtone set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.ringtone,
    );
    await AudioManager.volumeController.setLevel(AudioVolumeType.ringtone, max);
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.ringtone,
    );
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.ringtone, 0);
    level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.ringtone,
    );
    expect(level, equals(0));
  });

  test('test system set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.system,
    );
    await AudioManager.volumeController.setLevel(AudioVolumeType.system, max);
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.system,
    );
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.system, 0);
    level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.system,
    );
    expect(level, equals(0));
  });

  test('test voice set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.voice,
    );
    await AudioManager.volumeController.setLevel(AudioVolumeType.voice, max);
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.voice,
    );
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.voice, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.voice);
    expect(level, equals(0));
  });

  test('test voip set level', () async {
    final int max = await AudioManager.volumeController.getMaxLevel(
      AudioVolumeType.voip,
    );
    await AudioManager.volumeController.setLevel(AudioVolumeType.voip, max);
    int level = await AudioManager.volumeController.getLevel(
      AudioVolumeType.voip,
    );
    expect(level, equals(max));

    await AudioManager.volumeController.setLevel(AudioVolumeType.voip, 0);
    level = await AudioManager.volumeController.getLevel(AudioVolumeType.voip);
    expect(level, equals(0));
  });

  test('currentPlaybackType returns a valid AudioVolumeType', () async {
    final AudioVolumeType type =
        await AudioManager.volumeController.currentPlaybackType;
    expect(AudioVolumeType.values, contains(type));
  });

  test('onChanged emits VolumeChangedEvent when volume is changed', () async {
    final int originalLevel =
        await AudioManager.volumeController.getLevel(AudioVolumeType.system);
    final int maxLevel =
        await AudioManager.volumeController.getMaxLevel(AudioVolumeType.system);
    final int targetLevel = originalLevel == maxLevel ? 0 : maxLevel;

    final Completer<VolumeChangedEvent> completer =
        Completer<VolumeChangedEvent>();
    final StreamSubscription<VolumeChangedEvent> subscription = AudioManager
        .volumeController.onChanged
        .listen((VolumeChangedEvent event) {
      if (event.type == AudioVolumeType.system && !completer.isCompleted) {
        completer.complete(event);
      }
    });

    try {
      await AudioManager.volumeController
          .setLevel(AudioVolumeType.system, targetLevel);
      final VolumeChangedEvent event =
          await completer.future.timeout(const Duration(seconds: 5));

      expect(event.type, equals(AudioVolumeType.system));
      expect(event.level, equals(targetLevel));
    } finally {
      await AudioManager.volumeController
          .setLevel(AudioVolumeType.system, originalLevel);
      await subscription.cancel();
    }
  });
}
